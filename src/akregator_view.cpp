/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny, Sashmit Bhaduri            *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *   smt@vfemail.net (Sashmit Bhaduri)                                     *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "akregator_part.h"
#include "akregator_view.h"
#include "addfeeddialog.h"
#include "propertiesdialog.h"
#include "frame.h"
#include "fetchtransaction.h"
#include "intervalmanager.h"
#include "feediconmanager.h"
#include "feedstree.h"
#include "articlelist.h"
#include "articleviewer.h"
#include "viewer.h"
#include "archive.h"
#include "feed.h"
#include "akregatorconfig.h"
#include "pageviewer.h"
#include "articlefilter.h"
#include "tabwidget.h"

#include <kaction.h>
#include <kapplication.h>
#include <kcharsets.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <kfileitem.h>
#include <khtml_part.h>
#include <khtmlview.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <klineedit.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpassdlg.h>
#include <kprocess.h>
#include <krun.h>
#include <kshell.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kxmlguifactory.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qfile.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmultilineedit.h>
#include <qpopupmenu.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qvaluevector.h>
#include <qwhatsthis.h>

using namespace Akregator;

aKregatorView::aKregatorView( aKregatorPart *part, QWidget *parent, const char *wName)
   : QWidget(parent, wName), m_feeds(), m_viewMode(NormalView)
{
    m_part=part;
    m_stopLoading=false;

    setFocusPolicy(QWidget::StrongFocus);

    m_feedTreePixmap=KGlobal::iconLoader()->loadIcon("txt", KIcon::Small);
    m_folderTreePixmap=KGlobal::iconLoader()->loadIcon("folder", KIcon::Small);
    m_errorTreePixmap=KGlobal::iconLoader()->loadIcon("error", KIcon::Small);

    QVBoxLayout *lt = new QVBoxLayout( this );

    m_feedSplitter = new QSplitter(QSplitter::Horizontal, this, "panner1");
    m_feedSplitter->setOpaqueResize( true );
    lt->addWidget(m_feedSplitter);

    m_transaction= new FetchTransaction(this);
    connect (m_transaction, SIGNAL(fetched(Feed*)), this, SLOT(slotFeedFetched(Feed*)));
    connect (m_transaction, SIGNAL(fetchError(Feed*)), this, SLOT(slotFeedFetchError(Feed*)));
    connect (m_transaction, SIGNAL(completed()), this, SLOT(slotFetchesCompleted()));

    m_tree = new FeedsTree( m_feedSplitter, "FeedsTree" );

    connect(m_tree, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
              this, SLOT(slotContextMenu(KListView*, QListViewItem*, const QPoint&)));
    connect(m_tree, SIGNAL(selectionChanged(QListViewItem*)),
              this, SLOT(slotItemChanged(QListViewItem*)));
    connect(m_tree, SIGNAL(itemRenamed(QListViewItem *)),
              this, SLOT(slotItemRenamed(QListViewItem *)));
    connect(m_tree, SIGNAL(itemRenamed(QListViewItem *,int)),
              this, SLOT(slotItemRenamed(QListViewItem *)));
    connect(m_tree, SIGNAL(dropped (KURL::List &, QListViewItem *, QListViewItem *)),
              this, SLOT(slotFeedURLDropped (KURL::List &, QListViewItem *, QListViewItem *)));
    connect(m_tree, SIGNAL(moved()),
              this, SLOT(slotItemMoved()));

    m_feedSplitter->setResizeMode( m_tree, QSplitter::KeepSize );

    m_tabs = new TabWidget(m_feedSplitter);

    m_tabsClose = new QToolButton( m_tabs );
    m_tabsClose->setAccel(QKeySequence("Ctrl+W"));
    connect( m_tabsClose, SIGNAL( clicked() ), this,
            SLOT( slotRemoveFrame() ) );

    m_tabsClose->setIconSet( SmallIcon( "tab_remove" ) );
    m_tabsClose->adjustSize();
    QToolTip::add(m_tabsClose, i18n("Close the current tab"));
    m_tabs->setCornerWidget( m_tabsClose, TopRight );

    connect( m_tabs, SIGNAL( currentFrameChanged(Frame *) ), this,
            SLOT( slotFrameChanged(Frame *) ) );

    QWhatsThis::add(m_tabs, i18n("You can view multiple articles in several open tabs."));

    m_mainTab = new QWidget(this, "Article Tab");
    QVBoxLayout *mainTabLayout = new QVBoxLayout( m_mainTab, 0, 2, "mainTabLayout");

    QWhatsThis::add(m_mainTab, i18n("Articles list."));

    QHBoxLayout *searchLayout = new QHBoxLayout( 0, 0, KDialog::spacingHint(), "searchLayout" );
    QToolButton *clearButton = new QToolButton( m_mainTab );
    clearButton->setIconSet( SmallIconSet( QApplication::reverseLayout() ? "clear_left" : "locationbar_erase" ) );
    clearButton->setAutoRaise(true);
    searchLayout->addWidget(clearButton);
    QLabel* searchLabel = new QLabel(m_mainTab);
    searchLabel->setText( i18n("S&earch:") );
    searchLayout->addWidget(searchLabel);
    m_searchLine = new KLineEdit(m_mainTab, "searchline");
    searchLabel->setBuddy(m_searchLine);
    searchLayout->addWidget(m_searchLine);
    m_searchCombo = new KComboBox(m_mainTab, "searchcombo");
    searchLayout->addWidget(m_searchCombo);
    mainTabLayout->addLayout(searchLayout);

    m_searchCombo->insertItem(i18n("All Articles"));
    m_searchCombo->insertItem(i18n("Unread"));
    m_searchCombo->insertItem(i18n("New"));
    m_searchCombo->insertItem(i18n("New & Unread"));

    QToolTip::add( clearButton, i18n( "Clear filter" ) );
    QToolTip::add( m_searchLine, i18n( "Enter space-separated terms to filter article list" ) );
    QToolTip::add( m_searchCombo, i18n( "Choose what kind of articles to show in article list" ) );

    connect(clearButton, SIGNAL( clicked() ),
                      m_searchLine, SLOT(clear()) );
    connect(m_searchCombo, SIGNAL(activated(int)),
                          this, SLOT(slotSearchComboChanged(int)));
    connect(m_searchLine, SIGNAL(textChanged(const QString &)),
                        this, SLOT(slotSearchTextChanged(const QString &)));

    m_currentTextFilter=0;
    m_currentStatusFilter=0;
    m_queuedSearches=0;
    m_fetchTimer=0;
    
    m_articleSplitter = new QSplitter(QSplitter::Vertical, m_mainTab, "panner2");

    m_articles = new ArticleList( m_articleSplitter, "articles" );
    connect( m_articles, SIGNAL(mouseButtonPressed(int, QListViewItem *, const QPoint &, int)), this, SLOT(slotMouseButtonPressed(int, QListViewItem *, const QPoint &, int)));

    // use selectionChanged instead of clicked
    connect( m_articles, SIGNAL(selectionChanged(QListViewItem *)),
                   this, SLOT( slotArticleSelected(QListViewItem *)) );
    connect( m_articles, SIGNAL(doubleClicked(QListViewItem *, const QPoint &, int)),
                   this, SLOT( slotArticleDoubleClicked(QListViewItem *, const QPoint &, int)) );

    m_articleViewer = new ArticleViewer(m_articleSplitter, "article_viewer");

    connect( m_articleViewer, SIGNAL(urlClicked(const KURL&, bool)),
                        this, SLOT(slotOpenTab(const KURL&, bool)) );

    connect( m_articleViewer->browserExtension(), SIGNAL(mouseOverInfo(const KFileItem *)),
                                            this, SLOT(slotMouseOverInfo(const KFileItem *)) );

    QWhatsThis::add(m_articleViewer->widget(), i18n("Browsing area."));
    mainTabLayout->addWidget( m_articleSplitter );

    m_mainFrame=new Frame(this, m_part, m_mainTab, i18n("Articles"), false);
    connectFrame(m_mainFrame);
    m_tabs->addFrame(m_mainFrame);
    
    // -- DEFAULT INIT
    reset();
    m_articleViewer->openDefault();
    // -- /DEFAULT INIT

    m_feedSplitter->setSizes( Settings::splitter1Sizes() );
    m_articleSplitter->setSizes( Settings::splitter2Sizes() );

    m_searchCombo->setCurrentItem(Settings::quickFilter());
    slotSearchComboChanged(Settings::quickFilter());

    m_globalFetchTimer = new QTimer;
    connect( m_globalFetchTimer, SIGNAL(timeout()), this, SLOT(slotFetchAllFeeds()));
    if(Settings::useIntervalFetch())
       m_globalFetchTimer->start( Settings::autoFetchInterval()*60*1000 );

    m_fetchTimer=new QTimer(this);
    connect(m_fetchTimer, SIGNAL(timeout()), this, SLOT(slotDoIntervalFetches()));
    m_fetchTimer->start(1000*60, false);

    // delete expired articles once per hour
    m_expiryTimer = new QTimer(this);
    connect(m_expiryTimer, SIGNAL(timeout()), this, 
    SLOT(slotDeleteExpiredArticles()) );
    m_expiryTimer->start(3600000);
    
    // Change default view mode
    int viewMode = Settings::viewMode();
    
    if (viewMode==CombinedView)        slotCombinedView();
    else if (viewMode==WidescreenView) slotWidescreenView();
    else                               slotNormalView();
}

void aKregatorView::saveSettings(bool /*quit*/)
{
   Settings::setSplitter1Sizes( m_feedSplitter->sizes() );
   Settings::setSplitter2Sizes( m_articleSplitter->sizes() );
   Settings::setViewMode( m_viewMode );
   Settings::writeConfig();
   if(Settings::useIntervalFetch())
      m_globalFetchTimer->changeInterval( Settings::autoFetchInterval()*60*1000 );
   else
      m_globalFetchTimer->stop();
}

void aKregatorView::slotOpenTab(const KURL& url, bool background=false)
{
    PageViewer *page = new PageViewer(this, "page");
    connect( page, SIGNAL(setWindowCaption (const QString &)),
            this, SLOT(slotTabCaption (const QString &)) );
    connect( page, SIGNAL(urlClicked(const KURL &,bool)),
             this, SLOT(slotOpenTab(const KURL &,bool)) );

    Frame *frame=new Frame(this, page, page->widget(), i18n("Untitled"));
    connectFrame(frame);
    m_tabs->addFrame(frame);

    if(!background)
        m_tabs->showPage(page->widget());
    if (m_tabs->count() > 1 && m_tabs->currentPageIndex() != 0)
        m_tabsClose->setEnabled(true);
    page->openURL(url);
}

void aKregatorView::connectFrame(Frame *f)
{
    connect(f, SIGNAL(statusText(const QString &)), this, SLOT(slotStatusText(const QString&)));
    connect(f, SIGNAL(captionChanged (const QString &)), this, SLOT(slotCaptionChanged (const QString &)));
    connect(f, SIGNAL(loadingProgress(int)), this, SLOT(slotLoadingProgress(int)) );
    connect(f, SIGNAL(started()), this, SLOT(slotStarted()));
    connect(f, SIGNAL(completed()), this, SLOT(slotCompleted()));
    connect(f, SIGNAL(canceled(const QString &)), this, SLOT(slotCanceled(const QString&)));
}

void aKregatorView::slotStatusText(const QString &c)
{
    const Frame *f=static_cast<const Frame *>(sender());
    if (!f) return;
    if (m_currentFrame != f) return;

    m_part->setStatusBar(c);
}

void aKregatorView::slotCaptionChanged(const QString &c)
{
    const Frame *f=static_cast<const Frame *>(sender());
    if (!f) return;
    if (m_currentFrame != f) return;

    m_part->setCaption(c);
}

void aKregatorView::slotStarted()
{
    const Frame *f=static_cast<const Frame *>(sender());
    if (!f) return;
    if (m_currentFrame != f) return;

    m_part->setStarted();
}

void aKregatorView::slotCanceled(const QString &s)
{
    const Frame *f=static_cast<const Frame *>(sender());
    if (!f) return;
    if (m_currentFrame != f) return;

    m_part->setCanceled(s);
}

void aKregatorView::slotCompleted()
{
    const Frame *f=static_cast<const Frame *>(sender());
    if (!f) return;
    if (m_currentFrame != f) return;

    m_part->setCompleted();
}

void aKregatorView::slotLoadingProgress(int percent)
{
    const Frame *f=static_cast<const Frame *>(sender());
    if (!f) return;
    if (m_currentFrame != f) return;

    m_part->setProgress(percent);
}


// clears everything out
void aKregatorView::reset()
{
    m_feeds.clearFeeds();
    m_tree->clear();

    // Root item
    FeedsTreeItem *elt = new FeedsTreeItem( true, m_tree, QString::null );
    elt->setPixmap(0, m_folderTreePixmap );
    m_feeds.addFeedGroup(elt)->setTitle( i18n("All Feeds") );
    elt->setExpandable(true);
    elt->setOpen(true);
}

QString aKregatorView::getTitleNodeText(const QDomDocument &doc)
{
    if (doc.documentElement().tagName().lower() != "opml")
        return QString::null;

    QDomNode headNode = doc.documentElement().firstChild();
    while (!headNode.isNull() &&
           headNode.toElement().tagName().lower() != "head") {
        headNode = headNode.nextSibling();
    }

    if (headNode.isNull()) {
        return QString::null;
    }

    QDomNode textNode=headNode.namedItem("text");
    if (textNode.isNull())
        textNode=headNode.namedItem("title");
    if (textNode.isNull())
        return QString::null;

    QString result = textNode.toElement().text().simplifyWhiteSpace();
    if (result.isEmpty())
        return QString::null;

    return result;
}


bool aKregatorView::importFeeds(const QDomDocument& doc)
{
    QString text=getTitleNodeText(doc);
    if (text.isNull())
        text=i18n("Imported Folder");
    bool Ok;
    text = KInputDialog::getText(i18n("Add Imported Folder"), i18n("Imported folder name:"), text, &Ok);
    if (!Ok) return false;

    FeedsTreeItem *elt = new FeedsTreeItem( true, m_tree->firstChild(), QString::null );
    elt->setPixmap(0, m_folderTreePixmap);
    m_feeds.addFeedGroup(elt)->setTitle(text);
    elt->setExpandable(true);
    elt->setOpen(true);

    startOperation();
    if (!loadFeeds(doc, elt))
    {
        operationError(i18n("Invalid Feed List"));
        return false;
    }

    endOperation();
    return true;
}

bool aKregatorView::loadFeeds(const QDomDocument& doc, QListViewItem *parent)
{
    // this should be OPML document
    QDomElement root = doc.documentElement();

    m_stopLoading=false;
    kdDebug() << "loading OPML feed "<<root.tagName().lower()<<endl;
    if (root.tagName().lower() != "opml")
        return false;

    QDomNode bodyNode = root.firstChild();
    while (!bodyNode.isNull() &&
           bodyNode.toElement().tagName().lower() != "body") {
        bodyNode = bodyNode.nextSibling();
    }

    if (bodyNode.isNull()) {
        kdDebug() << "Failed to acquire body node, markup broken?" << endl;
        return false;
    }

    QDomElement body = bodyNode.toElement();

    if (!parent)
    {
        reset();
        parent = m_tree->firstChild();
    }

    m_tree->setUpdatesEnabled(false);
    int numNodes=body.childNodes().count();
    int curNodes=0;

    QDomNode n = body.firstChild();
    while( !n.isNull() )
    {
        if (m_stopLoading)
           break;
        parseChildNodes(n, parent);
        curNodes++;
        m_mainFrame->setProgress(int(100*((double)curNodes/(double)numNodes)));
        n = n.nextSibling();
    }
 
    // delete expired articles 
 
    slotDeleteExpiredArticles();
    
    setTotalUnread();
    m_tree->setUpdatesEnabled(true);
    m_tree->triggerUpdate();

    return true;
}

void aKregatorView::slotDeleteExpiredArticles()
{
    for (QListViewItemIterator it(m_tree->firstChild()); it.current(); ++it)
    {
        Feed *f = static_cast<Feed *>(m_feeds.find(*it));
        if (f && !f->isGroup())
            f->deleteExpiredArticles();
    }
}

void aKregatorView::parseChildNodes(QDomNode &node, QListViewItem *parent)
{
    if (m_stopLoading)
        return;
    QDomElement e = node.toElement(); // try to convert the node to an element.

    if( !e.isNull() )
    {
        FeedsTreeItem *elt;
        QString title=e.hasAttribute("text") ? e.attribute("text") : e.attribute
            ("title");
        if (parent)
        {
            QListViewItem *lastChild = parent->firstChild();
            while (lastChild && lastChild->nextSibling()) lastChild = lastChild->nextSibling();
            elt = new FeedsTreeItem( true, parent, lastChild, KCharsets::resolveEntities(title) );

        }
        else
            elt = new FeedsTreeItem( true, m_tree, m_tree->lastItem(), KCharsets::resolveEntities(title) );

        if (e.hasAttribute("xmlUrl") || e.hasAttribute("xmlurl"))
        {
            elt->setFolder(false);
            QString xmlurl=e.hasAttribute("xmlUrl") ? e.attribute("xmlUrl") : e.attribute("xmlurl");

            elt->setPixmap(0, m_feedTreePixmap);
            addFeed_Internal( 0, elt,
                              title,
                              xmlurl,
                              e.attribute("htmlUrl"),
                              e.attribute("description"),
                              e.attribute("autoFetch") == "true" ? true : false,
                              e.attribute("fetchInterval").toUInt(),
                              Feed::stringToArchiveMode(e.attribute("archiveMode")),
                              e.attribute("maxArticleAge").toUInt(),
                              e.attribute("maxArticleNumber").toUInt()
                            );
        }
        else
        {
            m_feeds.addFeedGroup(elt);
            FeedGroup *g = m_feeds.find(elt);

            elt->setPixmap(0, m_folderTreePixmap);

            if (g)
                g->setTitle(title);

            elt->setExpandable(true);
            elt->setOpen( e.attribute("isOpen", "true") == "true" ? true : false );
        }

        kapp->processEvents();

        if (e.hasChildNodes())
        {
            QDomNode child = e.firstChild();
            while(!child.isNull())
            {
                parseChildNodes(child, elt);
                child = child.nextSibling();
            }
        }
    }
}

// oh ugly as hell (pass Feed parameters in a FeedData?)
Feed *aKregatorView::addFeed_Internal(Feed *ef, QListViewItem *elt,
                                      QString title, QString xmlUrl, QString htmlUrl,
                                      QString description, bool autoFetch, int fetchInterval,
				      Feed::ArchiveMode archiveMode,
                                      int maxArticleAge,
                                      int maxArticleNumber)
{
    Feed *feed;
    if (ef)
    {
        m_feeds.addFeed(ef);
        feed=ef;
    }
    else
    {
        m_feeds.addFeed(elt);
        feed = static_cast<Feed *>(m_feeds.find(elt));
    }

    feed->setTitle( title );
    feed->setXmlUrl(xmlUrl);
    feed->setHtmlUrl(htmlUrl);
    feed->setDescription(description);
    feed->setAutoFetch(autoFetch);
    feed->setFetchInterval(fetchInterval);
    feed->setArchiveMode(archiveMode);
    feed->setMaxArticleAge(maxArticleAge);
    feed->setMaxArticleNumber(maxArticleNumber);
    
    Archive::load(feed);

    FeedsTreeItem *fti = static_cast<FeedsTreeItem *>(elt);
    if (fti)
        fti->setUnread(feed->unread());

    return feed;
}

void aKregatorView::storeTree( QDomElement &node, QDomDocument &document )
{
   writeChildNodes(0, node, document);
}

// writes children of given node
// node NULL has special meaning - it saves whole tree
void aKregatorView::writeChildNodes( QListViewItem *item, QDomElement &node, QDomDocument &document)
{
    if (!item) // omit "All Feeds" from saving (BR #43)
    {
        item = m_tree->firstChild(); // All Feeds
        if (!item) return;
        writeChildNodes(item, node, document);
        return;
    }

    for (QListViewItem *it = item->firstChild(); it; it = it->nextSibling())
    {
        FeedGroup *g = m_feeds.find(it);
        if (g)
        {
            if (g->isGroup())
            {
                QDomElement base = g->toXml( node, document );
                base.setAttribute("isOpen", it->isOpen() ? "true" : "false");

                if (it->firstChild()) // BR#40
                   writeChildNodes( it, base, document );
            } else {
                g->toXml( node, document );
            }
        }
    }
}

bool aKregatorView::event(QEvent *e)
{
    if (e->type() == QEvent::ApplicationPaletteChange)
    {
        m_articleViewer->reload();
        return true;
    }
    return QWidget::event(e);
}

void aKregatorView::addFeedToGroup(const QString& url, const QString& group)
{
    QListViewItem *lastChild;
    // Locate the group.
    QListViewItem *item = m_tree->findItem(group, 0, 0);
    // If group does not exist, create as last in tree.
    if (!item)
    {
        // Get "All Feeds" folder.
        QListViewItem *allFeedsFolder = m_tree->firstChild();
        // Get last child of "All Feeds".
        lastChild = allFeedsFolder->firstChild();
        while (lastChild && lastChild->nextSibling())
            lastChild = lastChild->nextSibling();

        FeedsTreeItem *elt;
        if (lastChild)
            elt = new FeedsTreeItem(true, allFeedsFolder, lastChild, group);
        else
            elt = new FeedsTreeItem(true, allFeedsFolder, group);

        m_feeds.addFeedGroup(elt);
        FeedGroup *g = m_feeds.find(elt);
        if (g)
            g->setTitle( group );

        item = g->item();
    }
    // Locate last feed (or folder) in the group.
    lastChild = item->firstChild();
    while (lastChild && lastChild->nextSibling()) lastChild = lastChild->nextSibling();
    if (lastChild)
        m_tree->ensureItemVisible(lastChild);
    else
        m_tree->ensureItemVisible(item);
    // Invoke the Add Feed dialog with url filled in.
    addFeed(url, lastChild, item, true);
}

void aKregatorView::slotNormalView()
{
    if (m_viewMode==NormalView)
       return;

    if (m_viewMode==CombinedView)
    {
        m_articles->show();
        // tell articleview to redisplay+reformat
        ArticleListItem *item = static_cast<ArticleListItem *>(m_articles->currentItem());
        if (item)
             //     if (!feed->isGroup()) // What's that if good for? -fo
            m_articleViewer->slotShowArticle(item->article());
        else 
            m_articleViewer->slotClear();
    }

    m_articleSplitter->setOrientation(QSplitter::Vertical);
    m_viewMode=NormalView;

    Settings::setViewMode( m_viewMode );
}

void aKregatorView::slotWidescreenView()
{
    if (m_viewMode==WidescreenView)
       return;
    else if (m_viewMode==CombinedView)
    {
        m_articles->show();
        // tell articleview to redisplay+reformat
        ArticleListItem *item = static_cast<ArticleListItem *>(m_articles->currentItem());
        if (item)
        
           // if (!feed->isGroup()) // what's that if good for? -fo
                m_articleViewer->slotShowArticle(item->article());
        else
            m_articleViewer->slotClear();
    }

    m_articleSplitter->setOrientation(QSplitter::Horizontal);
    m_viewMode=WidescreenView;

    Settings::setViewMode( m_viewMode );
}

void aKregatorView::slotCombinedView()
{
    if (m_viewMode == CombinedView)
       return;

    m_articles->hide();
    m_viewMode = CombinedView;
    
    slotItemChanged(m_tree->currentItem());
    
    Settings::setViewMode( m_viewMode );
}


void aKregatorView::startOperation()
{
    m_mainFrame->setState(Frame::Started);
    m_part->actionCollection()->action("feed_fetch")->setEnabled(false);
    m_part->actionCollection()->action("feed_fetch_all")->setEnabled(false);
    m_mainFrame->setProgress(0);
}

void aKregatorView::endOperation()
{
    m_mainFrame->setState(Frame::Completed);
    m_part->actionCollection()->action("feed_fetch")->setEnabled(true);
    m_part->actionCollection()->action("feed_fetch_all")->setEnabled(true);
    m_mainFrame->setProgress(100);
}

void aKregatorView::operationError(const QString & /*msg*/)
{
    m_mainFrame->setState(Frame::Canceled);
    m_part->actionCollection()->action("feed_fetch")->setEnabled(true);
    m_part->actionCollection()->action("feed_fetch_all")->setEnabled(true);
    m_mainFrame->setProgress(-1);
}

void aKregatorView::slotRemoveFrame()
{
    Frame *f = m_tabs->currentFrame();
    if (f==m_mainFrame)
        return;
    m_tabs->removeFrame(f);
    delete f;
    if (m_tabs->count() <= 1)
        m_tabsClose->setEnabled(false);
}

void aKregatorView::slotFrameChanged(Frame *f)
{
    m_currentFrame=f;

    if (f==m_mainFrame)
        m_tabsClose->setEnabled(false);
    else
        m_tabsClose->setEnabled(true);

    KParts::ReadOnlyPart *p=f->part();
    m_part->changePart(p);
    m_part->setCaption(f->caption());
    m_part->setProgress(f->progress());
    m_part->setStatusBar(f->statusText());

    switch (f->state())
    {

        case Frame::Started:
            m_part->setStarted();
            break;
        case Frame::Canceled:
            m_part->setCanceled(QString::null);
            break;
        case Frame::Idle:
        case Frame::Completed:
        default:
            m_part->setCompleted();
    }

}

void aKregatorView::slotTabCaption(const QString &capt)
{
    if (!capt.isEmpty())
    {
        PageViewer *pv=(PageViewer *)sender();
        m_tabs->setTitle(capt, pv->widget());
    }
}

void aKregatorView::slotContextMenu(KListView*, QListViewItem* item, const QPoint& p)
{
   FeedGroup *feed = static_cast<FeedGroup *>(m_feeds.find(item));

   if (!feed)
       return;

   m_tabs->showPage(m_mainTab);

   QWidget *w;
   if (feed->isGroup())
      w = m_part->factory()->container("feedgroup_popup", m_part);
   else
      w = m_part->factory()->container("feeds_popup", m_part);
   if (w)
      static_cast<QPopupMenu *>(w)->exec(p);
}

void aKregatorView::slotItemChanged(QListViewItem *item)
{
    FeedGroup* node = m_feeds.find(item);
    
    m_tabs->showPage(m_mainTab);
    
    if (m_viewMode == CombinedView)
        m_articleViewer->slotShowNode(node);
    else   
        m_articles->slotShowNode(node);
    
    if (item && m_part->actionCollection()->action("feed_remove") )
    {
        if (item->parent())
            m_part->actionCollection()->action("feed_remove")->setEnabled(true);
        else
            m_part->actionCollection()->action("feed_remove")->setEnabled(false);
    }    
}


void aKregatorView::slotFeedAdd()
{
    FeedsTreeItem *i=static_cast<FeedsTreeItem*>(m_tree->currentItem());
    if (!i)
        i=static_cast<FeedsTreeItem*>(m_tree->firstChild()); // all feeds

    QListViewItem *lastChild; 
    if (i->isFolder())        
    {
        lastChild= i->firstChild();
        while (lastChild && lastChild->nextSibling())
            lastChild = lastChild->nextSibling();
    }
    else
    {
        // if it's not a folder, add the feed AFTER the selected feed
        lastChild=i;
        i=static_cast<FeedsTreeItem*>(i->parent());
    }

    addFeed(QString::null, lastChild, i);

}

void aKregatorView::addFeed(QString url, QListViewItem *after, QListViewItem* parent, bool autoExec /*= false*/)
{
    FeedsTreeItem *elt;
    Feed *feed;
    AddFeedDialog *afd = new AddFeedDialog( 0, "add_feed" );

    afd->setURL(url);

    QString text;
    if (autoExec)
    {
        afd->slotOk();
        feed=afd->feed;
        text=feed->title();
    }
    else
    {
        if (afd->exec() != QDialog::Accepted) return;
        text=afd->feedTitle;
        feed=afd->feed;
    }

    FeedPropertiesDialog *dlg = new FeedPropertiesDialog( 0, "edit_feed" );

    dlg->setFeedName(text);
    dlg->setUrl(afd->feedURL);
    dlg->selectFeedName();

    if (!autoExec)
        if (dlg->exec() != QDialog::Accepted) return;

    if (!parent)
        parent=m_tree->firstChild();

    if (after)
        elt = new FeedsTreeItem(false, parent, after, text);
    else
        elt = new FeedsTreeItem(false, parent, text);


    elt->setPixmap(0, m_feedTreePixmap);
    feed->setItem(elt);
 
    addFeed_Internal( feed, elt,
                      dlg->feedName(),
                      dlg->url(),
                      feed->htmlUrl(),
                      feed->description(),
                      dlg->autoFetch(),
                      dlg->fetchInterval(),
		      dlg->archiveMode(),
                      dlg->maxArticleAge(),
                      dlg->maxArticleNumber()
                    );

    m_tree->ensureItemVisible(elt);
    setTotalUnread();

    m_part->setModified(true);
    delete afd;
    delete dlg;
}

void aKregatorView::slotFeedAddGroup()
{
    if (!m_tree->currentItem() || m_feeds.find(m_tree->currentItem())->isGroup() == false)
    {
        KMessageBox::error(this, i18n("You have to choose a folder before adding a subfolder."));
        return;
    }

    bool Ok;
    FeedsTreeItem *elt;

    QString text = KInputDialog::getText(i18n("Add Folder"), i18n("Folder name:"), "", &Ok);
    if (!Ok) return;

    QListViewItem *lastChild = m_tree->currentItem()->firstChild();
    while (lastChild && lastChild->nextSibling())
        lastChild = lastChild->nextSibling();

    if (lastChild)
        elt = new FeedsTreeItem(true, m_tree->currentItem(), lastChild, text);
    else
        elt = new FeedsTreeItem(true, m_tree->currentItem(), text);

    // expandable, so we can use KListView's implementation to drop items into empty folders
    elt->setExpandable(true);
    elt->setOpen(true);

    m_feeds.addFeedGroup(elt);
    FeedGroup *g = m_feeds.find(elt);
    if (g)
        g->setTitle( text );

    m_tree->ensureItemVisible(elt);

    m_part->setModified(true);
}

void aKregatorView::slotFeedRemove()
{
    QListViewItem *elt = m_tree->currentItem();
    if (!elt) return;
    QListViewItem *parent = elt->parent();
    if (!parent) return; // don't delete root element! (safety valve)

    QString msg = elt->childCount() ?
        i18n("<qt>Are you sure you want to delete folder<br><b>%1</b><br> and its feeds and subfolders?</qt>") :
        i18n("<qt>Are you sure you want to delete feed<br><b>%1</b>?</qt>");
    if (KMessageBox::warningContinueCancel(0, msg.arg(elt->text(0)),i18n("Delete Feed"),KGuiItem(i18n("&Delete"),"editdelete")) == KMessageBox::Continue)
    {
        m_articles->clear();
        m_feeds.removeFeed(elt);
        // FIXME: kill children? (otoh - auto kill)
/*        if (!Notes.count())
            slotActionUpdate();
        if (!parent)
            parent = Items->firstChild();
        Items->prevItem = 0;
        slotNoteChanged(parent);*/

        m_part->setModified(true);
        setTotalUnread();
    }
}

void aKregatorView::slotFeedModify()
{
    kdDebug() << k_funcinfo << "BEGIN" << endl;

    FeedGroup *g = m_feeds.find(m_tree->currentItem());
    if (g->isGroup())
    {
        m_tree->currentItem()->setRenameEnabled(0, true);
        m_tree->currentItem()->startRename(0);
        return;
    }

    Feed *feed = static_cast<Feed *>(g);
    if (!feed) return;

    FeedPropertiesDialog *dlg = new FeedPropertiesDialog( 0, "edit_feed" );

    dlg->setFeedName( feed->title() );
    dlg->setUrl( feed->xmlUrl() );
    dlg->setAutoFetch(feed->autoFetch());
    dlg->setFetchInterval(feed->fetchInterval());
    dlg->setArchiveMode(feed->archiveMode());
    dlg->setMaxArticleAge(feed->maxArticleAge());
    dlg->setMaxArticleNumber(feed->maxArticleNumber());
    
    if (dlg->exec() != QDialog::Accepted) return;

    feed->setTitle( dlg->feedName() );
    feed->setXmlUrl( dlg->url() );
    feed->setAutoFetch(dlg->autoFetch());
    feed->setFetchInterval(dlg->fetchInterval());
    feed->setArchiveMode(dlg->archiveMode());
    feed->setMaxArticleAge(dlg->maxArticleAge());
    feed->setMaxArticleNumber(dlg->maxArticleNumber());
    
    m_part->setModified(true);

    delete dlg;
    kdDebug() << k_funcinfo << "END" << endl;
}

void aKregatorView::slotNextUnread()
{
    ArticleListItem *it= static_cast<ArticleListItem*>(m_articles->selectedItem());
    if (!it)
        it=static_cast<ArticleListItem*>(m_articles->firstChild());
    for ( ; it; it = static_cast<ArticleListItem*>(it->nextSibling()))
    {
        if ((it->article().status()==MyArticle::Unread) ||
                (it->article().status()==MyArticle::New))
        {
            m_articles->setSelected(it, true);
            m_articles->ensureItemVisible(it);
            slotArticleSelected(it);
            return;
        }
    }
}

void aKregatorView::slotMarkAllFeedsRead()
{
    markAllRead(m_tree->firstChild());
}

void aKregatorView::slotMarkAllRead()
{
    markAllRead(m_tree->currentItem());
}


void aKregatorView::slotOpenHomepage()
{
   QListViewItem *item=m_tree->currentItem();
   Feed *f = static_cast<Feed *>(m_feeds.find(item));
   if(Settings::mMBBehaviour() == Settings::EnumMMBBehaviour::OpenInExternalBrowser)
       displayInExternalBrowser(f->htmlUrl());
   else
       slotOpenTab(f->htmlUrl());
}

void aKregatorView::markAllRead(QListViewItem *item)
{
    if (!item)
        return;
    else
    {
        Feed *f = static_cast<Feed *>(m_feeds.find(item));
        if (!f) return;
        if (!f->isGroup())
        {
            //kdDebug() << k_funcinfo << "item " << f->title() << endl;
            f->markAllRead();
            FeedsTreeItem *fti = static_cast<FeedsTreeItem *>(item);
            if (fti)
                fti->setUnread(0);
            m_articles->triggerUpdate();
            // TODO: schedule this save
            Archive::save(f);
        }
        else
        {
            FeedGroup *g = m_feeds.find(item);
            if (!g)
                return;
            //kdDebug() << k_funcinfo << "group " << g->title() << endl;
            for (QListViewItem *it = item->firstChild(); it; it = it->nextSibling())
            {
               markAllRead(it);
            }
        }
    }

    setTotalUnread();
}

void aKregatorView::setTotalUnread()
{
    FeedsTreeItem *allFeedsItem = static_cast<FeedsTreeItem *>(m_tree->firstChild());
    int totalUnread=totalUnread=allFeedsItem->countUnreadRecursive();
    m_part->setTotalUnread(totalUnread);
}

void aKregatorView::fetchItem(QListViewItem *item)
{
    kdDebug() << "enter fetchItem" << endl;
    if (item)
    {
        FeedGroup *fg = m_feeds.find(item);
        if (fg && fg->isGroup())
        {
            for (QListViewItem *it = item->firstChild(); it; it = it->nextSibling())
                fetchItem(it);
        }
        else
        {
            Feed *f = static_cast<Feed *>(fg);
            if (f)
                m_transaction->fetch(f);
        }
    }
    kdDebug() << "leave fetchItem" << endl;
}

void aKregatorView::showFetchStatus()
{
    if (m_transaction->totalFetches())
    {
        m_mainFrame->setStatusText(i18n("Fetching Feeds..."));
        m_mainFrame->setProgress(0);
    }
}

/**
 * Display article in external browser.
 */
void aKregatorView::displayInExternalBrowser(const KURL &url)
{
    if (!url.isValid()) return;
    if (Settings::externalBrowserUseKdeDefault())
        KRun::runURL(url, "text/html", false, false);
    else
    {
        QString cmd = Settings::externalBrowserCustomCommand();
        QString urlStr = url.url();
        cmd.replace(QRegExp("%u"), urlStr);
        KProcess *proc = new KProcess;
        QStringList cmdAndArgs = KShell::splitArgs(cmd);
        *proc << cmdAndArgs;
        proc->start(KProcess::DontCare);
        delete proc;
    }
}

void aKregatorView::slotDoIntervalFetches()
{
    if (m_transaction->isRunning() || m_part->loading())
        return;
    kdDebug() << "doIntervalFetches"<<endl;
    for (QListViewItemIterator it(m_tree->firstChild()); it.current(); ++it)
    {
        Feed *f = static_cast<Feed *>(m_feeds.find(*it));
        if (f && !f->isGroup() && f->autoFetch())
        {
            uint lastFetch=IntervalManager::self()->lastFetchTime(f->xmlUrl());
            QDateTime dt=QDateTime::currentDateTime();
            uint curTime=dt.toTime_t();
            if (curTime-lastFetch >= uint(f->fetchInterval()*60))
            {
            kdDebug() << "interval fetching---"<< f->xmlUrl() <<endl;
                m_transaction->fetch(f);
            }
        }
    }
    m_transaction->start();
}

void aKregatorView::slotFetchCurrentFeed()
{
    showFetchStatus();
    fetchItem(m_tree->currentItem());
    startOperation();
    m_transaction->start();
}

void aKregatorView::slotFetchAllFeeds()
{
    // this iterator iterates through ALL child items
    showFetchStatus();

    for (QListViewItemIterator it(m_tree->firstChild()); it.current(); ++it)
    {
        //kdDebug() << "Fetching subitem " << (*it)->text(0) << endl;
        Feed *f = static_cast<Feed *>(m_feeds.find(*it));
        if (f && !f->isGroup())
            m_transaction->fetch(f);
    }
    startOperation();
    m_transaction->start();
}

void aKregatorView::slotFetchesCompleted()
{
    endOperation();
    setTotalUnread();
    m_mainFrame->setStatusText(QString::null);
}

void aKregatorView::slotFeedFetched(Feed *feed)
{
    // If its a currenly selected feed, update view
//    if (feed->item() == m_tree->currentItem())
  //      slotUpdateArticleList(feed, false);

    // iterate through the articles (once again) to do notifications properly
    if (feed->articles().count() > 0)
    {
        ArticleSequence articles = feed->articles(); 
        ArticleSequence::ConstIterator it;
        ArticleSequence::ConstIterator end = articles.end();
        for (it = articles.begin(); it != end; ++it)
        {
            if ((*it).status()==MyArticle::New)
            {
                m_part->newArticle(feed, *it);     // will do systray notification
            }
        }
    }

    // TODO: move to slotFetchesCompleted
    Archive::save(feed);

    IntervalManager::self()->feedFetched(feed->xmlUrl());

    // Also, update unread counts

    FeedsTreeItem *fti = static_cast<FeedsTreeItem *>(feed->item());
    if (fti)
        fti->setUnread(feed->unread());

    int p=int(100*((double)m_transaction->fetchesDone()/(double)m_transaction->totalFetches()));
    m_mainFrame->setProgress(p);
}

void aKregatorView::slotFeedFetchError(Feed *feed)
{
    int p=int(100*((double)m_transaction->fetchesDone()/(double)m_transaction->totalFetches()));
    m_mainFrame->setProgress(p);
    if (feed && feed->item())
        feed->item()->setPixmap(0, m_errorTreePixmap);
}

void aKregatorView::slotMouseButtonPressed(int button, QListViewItem * item, const QPoint &, int)
{
    if (item && button==Qt::MidButton)
    {
        ArticleListItem *i = static_cast<ArticleListItem *>(item);
        if (!i) return;
        if(Settings::mMBBehaviour() == Settings::EnumMMBBehaviour::OpenInExternalBrowser)
            displayInExternalBrowser(i->article().link());
        else
        {
            if(Settings::mMBBehaviour() == Settings::EnumMMBBehaviour::OpenInBackground)
                slotOpenTab(i->article().link(),true);
            else
                slotOpenTab(i->article().link());
        }
    }
}

void aKregatorView::slotArticleSelected(QListViewItem *i)
{
    if (m_viewMode==CombinedView) return; // shouldn't ever happen

    ArticleListItem *item = static_cast<ArticleListItem *>(i);
    if (!item) return;
    Feed *feed = item->feed();
    if (!feed) return;

    if (item->article().status() != MyArticle::Read)
    {
        int unread=feed->unread();
        unread--;
        feed->setUnread(unread);

        FeedsTreeItem *fti = static_cast<FeedsTreeItem *>(feed->item());
        if (fti)
            fti->setUnread(unread);

        setTotalUnread();

        item->article().setStatus(MyArticle::Read);

        // TODO: schedule this save.. don't want to save a huge file for one change
        Archive::save(feed);
    }
    m_articleViewer->slotShowArticle( item->article() );
}

void aKregatorView::slotArticleDoubleClicked(QListViewItem *i, const QPoint &, int)
{
    ArticleListItem *item = static_cast<ArticleListItem *>(i);
    if (!item) return;
    // TODO : make this configurable....
    displayInExternalBrowser(item->article().link());
}

void aKregatorView::slotFeedURLDropped(KURL::List &urls, QListViewItem *after, QListViewItem *parent)
{
    KURL::List::iterator it;
    for ( it = urls.begin(); it != urls.end(); ++it )
    {
        addFeed((*it).prettyURL(), after, parent);
    }
}

void aKregatorView::slotItemRenamed( QListViewItem *item )
{
    QString text = item->text(0);
    kdDebug() << "Item renamed to " << text << endl;

    Feed *feed = static_cast<Feed *> (m_feeds.find(item));
    if (feed)
    {
        if (feed->title()!=text)
            m_part->setModified(true);

        feed->setTitle( text );
    }
}

void aKregatorView::slotItemMoved()
{
    m_part->setModified(true);
}

void aKregatorView::slotSearchComboChanged(int index)
{
    Settings::setQuickFilter( index );
    updateSearch();
}

// from klistviewsearchline
void aKregatorView::slotSearchTextChanged(const QString &search)
{
    m_queuedSearches++;
    m_queuedSearch = search;
    QTimer::singleShot(200, this, SLOT(activateSearch()));
}

void aKregatorView::activateSearch()
{
    m_queuedSearches--;

    if(m_queuedSearches == 0)
        updateSearch(m_queuedSearch);
}

void aKregatorView::updateSearch(const QString &s)
{
    delete m_currentTextFilter;
    delete m_currentStatusFilter;

    QValueList<Criterion> textCriteria;
    QValueList<Criterion> statusCriteria;

    QString textSearch=s.isNull() ? m_searchLine->text() : s;

    if (!textSearch.isEmpty())
    {
        Criterion subjCrit( Criterion::Title, Criterion::Contains, textSearch );
        textCriteria << subjCrit;
        Criterion crit1( Criterion::Description, Criterion::Contains, textSearch );
        textCriteria << crit1;
    }

    if (m_searchCombo->currentItem())
    {
        switch (m_searchCombo->currentItem())
        {
            case 1:
            {
                Criterion crit( Criterion::Status, Criterion::Equals, MyArticle::Unread);
                statusCriteria << crit;
                break;
            }
            case 2:
            {
                Criterion crit( Criterion::Status, Criterion::Equals, MyArticle::New);
                statusCriteria << crit;
                break;
            }
            case 3:
            {
                Criterion crit1( Criterion::Status, Criterion::Equals, MyArticle::New);
                Criterion crit2( Criterion::Status, Criterion::Equals, MyArticle::Unread);
                statusCriteria << crit1;
                statusCriteria << crit2;
                break;
            }
            default:
                break;
        }
    }

    m_currentTextFilter = new ArticleFilter(textCriteria, ArticleFilter::LogicalOr, ArticleFilter::Notify);
    m_currentStatusFilter = new ArticleFilter(statusCriteria, ArticleFilter::LogicalOr, ArticleFilter::Notify);

    
    m_articleViewer->slotSetFilter(*m_currentTextFilter, *m_currentStatusFilter)
    ;m_articles->slotSetFilter(*m_currentTextFilter, *m_currentStatusFilter);
}

void aKregatorView::slotMouseOverInfo(const KFileItem *kifi)
{
    if (kifi)
    {
        KFileItem *k=(KFileItem*)kifi;
        m_mainFrame->setStatusText(k->url().prettyURL());//getStatusBarInfo());
    }
    else
    {
        m_mainFrame->setStatusText(QString::null);
    }
}

void aKregatorView::stopLoading()
{
    m_stopLoading=true;
}

void aKregatorView::readProperties(KConfig* config)
{
    // delete expired articles
    for (QListViewItemIterator it(m_tree->firstChild()); it.current(); ++it)
    {
        Feed *f = static_cast<Feed *>(m_feeds.find(*it));
        if (f && !f->isGroup())
            f->deleteExpiredArticles();
    }
    
    // read filter settings 
    
    m_searchLine->setText(config->readEntry("searchLine"));
    m_searchCombo->setCurrentItem(config->readEntry("searchCombo").toInt()); 
    slotSearchComboChanged(config->readEntry("searchCombo").toInt());  

    // read the position of the selected feed
        
    QString selectedFeed = config->readEntry("selectedFeed");
    if ( selectedFeed != QString::null )
    {
        QStringList pos = QStringList::split(' ', selectedFeed);
        QListViewItem* current = m_tree->firstChild();
        for ( unsigned int i = 0; current && i < pos.count(); i++ )
        {
            int childPos = pos[i].toUInt();
            current = current->firstChild();
            if (current)
                for (int j = 0; j < childPos; j++)
                    if ( current->nextSibling() )
                        current = current->nextSibling();
        }    
        m_tree->setSelected(current, true);
       
        // read the selected article title (not in Combined View)
        
        if ( m_viewMode != CombinedView )
        {
            QString selectedArticleEntry = config->readEntry("selectedArticle");
            if ( selectedArticleEntry != QString::null )
            {        
                QListViewItem* selectedArticle = m_articles->findItem(selectedArticleEntry, 0);
                if ( selectedArticle )
                    m_articles->setSelected(selectedArticle, true);
            } 
        } // if viewMode != combinedView
   } // if selectedFeed is set
}

void aKregatorView::saveProperties(KConfig* config)
{   
    // save filter settings
    config->writeEntry("searchLine", m_searchLine->text());
    config->writeEntry("searchCombo", m_searchCombo->currentItem());
    
    // write the position of the currently selected feed
    // format is a string, e.g. "3 2 1" means
    // 2nd child of the 3rd child of the 4th child of the root node (All Feeds) 
    if ( m_tree->selectedItem() ) 
    {
        QListViewItem* item = m_tree->selectedItem();
        QListViewItem* parent = item->parent();
        QString pos;
        
        while (parent)
        {
            int n = 0;
            QListViewItem* i = parent->firstChild();
            while (i && i != item)
            {
                i = i->nextSibling();
                n++;
            }
            pos = QString::number(n) + " " + pos; 
            item = item->parent();
            parent = item->parent(); 
        }
        pos = pos.stripWhiteSpace();
        config->writeEntry("selectedFeed", pos); 
    }
    
    // if not in CombinedView, save currently selected article 
    // atm the item's text() is saved, which is ambigous. 
    
    if ( m_viewMode != CombinedView )         
    {
        if ( m_articles->selectedItem() )
            config->writeEntry("selectedArticle", m_articles->selectedItem()->text(0));
    }
}

#include "akregator_view.moc"

// vim: set et ts=4 sts=4 sw=4:
