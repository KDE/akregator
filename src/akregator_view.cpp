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

#include <kapplication.h>
#include <kfiledialog.h>
#include <kfileitem.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kxmlguifactory.h>
#include <kaction.h>
#include <kstandarddirs.h>
#include <klineedit.h>
#include <kpassdlg.h>
#include <kcharsets.h>
#include <kstandarddirs.h>
#include <klistview.h>
#include <khtml_part.h>
#include <khtmlview.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <krun.h>
#include <kdialog.h>
#include <kurl.h>
#include <kprocess.h>

#ifndef KDE_MAKE_VERSION
#define KDE_MAKE_VERSION( a,b,c ) (((a) << 16) | ((b) << 8) | (c))
#endif

#ifndef KDE_IS_VERSION
#define KDE_IS_VERSION(a,b,c) ( KDE_VERSION >= KDE_MAKE_VERSION(a,b,c) )
#endif

#if KDE_IS_VERSION(3,1,94)
#include <kshell.h>
#endif

#include <qfile.h>
#include <qtextstream.h>
#include <qmultilineedit.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <qpopupmenu.h>
#include <qtoolbutton.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qvaluevector.h>
#include <qtooltip.h>
#include <qlayout.h>

using namespace Akregator;


aKregatorView::aKregatorView( aKregatorPart *part, QWidget *parent, const char *wName)
    : QWidget(parent, wName), m_feeds()
{
    m_part=part;

    QVBoxLayout *lt = new QVBoxLayout( this );

    m_panner1 = new QSplitter(QSplitter::Horizontal, this, "panner1");
    m_panner1->setOpaqueResize( true );
    lt->addWidget(m_panner1);

    m_tree = new FeedsTree( m_panner1, "FeedsTree" );

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

    m_panner1->setResizeMode( m_tree, QSplitter::KeepSize );

    m_tabs = new TabWidget(m_panner1);

    m_tabsClose = new QToolButton( m_tabs );
    m_tabsClose->setAccel(QKeySequence("Ctrl+W"));
    connect( m_tabsClose, SIGNAL( clicked() ), this,
            SLOT( slotRemoveTab() ) );

    m_tabsClose->setIconSet( SmallIcon( "tab_remove" ) );
    m_tabsClose->adjustSize();
    QToolTip::add(m_tabsClose, i18n("Close the current tab"));
    m_tabs->setCornerWidget( m_tabsClose, TopRight );

    connect( m_tabs, SIGNAL( currentChanged(QWidget *) ), this,
            SLOT( slotTabChanged(QWidget *) ) );

    QWhatsThis::add(m_tabs, i18n("You can view multiple articles in several open tabs."));

    m_mainTab = new QWidget(this, "Article Tab");
    QVBoxLayout *mainTabLayout = new QVBoxLayout( m_mainTab, 0, 2, "mainTabLayout");

    QWhatsThis::add(m_mainTab, i18n("Articles list."));

    QHBoxLayout *searchLayout = new QHBoxLayout( 0, 0, KDialog::spacingHint(), "searchLayout");
    m_searchLine = new KLineEdit(m_mainTab, "searchline");
    searchLayout->addWidget(m_searchLine);
    m_searchCombo = new KComboBox(m_mainTab, "searchcombo");
    searchLayout->addWidget(m_searchCombo);
    mainTabLayout->addLayout(searchLayout);

    m_searchCombo->insertItem(i18n("All Articles"));
    m_searchCombo->insertItem(i18n("Unread"));
    m_searchCombo->insertItem(i18n("New"));
    m_searchCombo->insertItem(i18n("New & Unread"));

    connect(m_searchCombo, SIGNAL(activated(int)),
                          this, SLOT(slotSearchComboChanged(int)));
    connect(m_searchLine, SIGNAL(textChanged(const QString &)),
                        this, SLOT(slotSearchTextChanged(const QString &)));

    m_currentTextFilter=0;
    m_currentStatusFilter=0;
    m_queuedSearches=0;

    m_panner2 = new QSplitter(QSplitter::Vertical, m_mainTab, "panner2");

    m_articles = new ArticleList( m_panner2, "articles" );
    connect( m_articles, SIGNAL(mouseButtonPressed(int, QListViewItem *, const QPoint &, int)), this, SLOT(slotMouseButtonPressed(int, QListViewItem *, const QPoint &, int)));
    connect( m_articles, SIGNAL(clicked(QListViewItem *)),
                   this, SLOT( slotArticleSelected(QListViewItem *)) );
    connect( m_articles, SIGNAL(doubleClicked(QListViewItem *, const QPoint &, int)),
                   this, SLOT( slotArticleDoubleClicked(QListViewItem *, const QPoint &, int)) );

    m_articleViewer = new ArticleViewer(m_panner2, "article_viewer");

    connect( m_articleViewer, SIGNAL(urlClicked(const KURL&)),
                        this, SLOT(slotOpenTab(const KURL&)) );

    connect( m_articleViewer->browserExtension(), SIGNAL(mouseOverInfo(const KFileItem *)),
                                            this, SLOT(slotMouseOverInfo(const KFileItem *)) );

    QWhatsThis::add(m_articleViewer->widget(), i18n("Browsing area."));
    mainTabLayout->addWidget( m_panner2 );

    m_tabs->addTab(m_mainTab, i18n( "Articles" ));
    m_tabs->setTitle(i18n( "Articles" ), m_mainTab);

    // -- DEFAULT INIT
    reset();
    m_articleViewer->openDefault();
    // -- /DEFAULT INIT

    // Change default view mode
    int viewMode = Settings::viewMode();

    if (viewMode==CombinedView)        slotCombinedView();
    else if (viewMode==WidescreenView) slotWidescreenView();
    else                               slotNormalView();

    m_panner1->setSizes( Settings::splitter1Sizes() );
    m_panner2->setSizes( Settings::splitter2Sizes() );

    m_searchCombo->setCurrentItem(Settings::quickFilter());
    slotSearchComboChanged(Settings::quickFilter());
}

void aKregatorView::saveSettings(bool /*quit*/)
{
   Settings::setSplitter1Sizes( m_panner1->sizes() );
   Settings::setSplitter2Sizes( m_panner2->sizes() );
   Settings::setViewMode( m_viewMode );
   Settings::writeConfig();
}

void aKregatorView::slotOpenTab(const KURL& url)
{
    PageViewer *page = new PageViewer(this, "page");
    connect( page, SIGNAL(setWindowCaption (const QString &)),
            this, SLOT(slotTabCaption (const QString &)) );
    page->openURL(url);

    m_tabs->addTab(page->widget(), i18n("Untitled"));
    m_tabs->showPage(page->widget());
    if (m_tabs->count() > 1)
        m_tabsClose->setEnabled(true);
}

// clears everything out
void aKregatorView::reset()
{
    m_feeds.clearFeeds();
    m_tree->clear();
    m_part->setModified(false);

    // Root item
    FeedsTreeItem *elt = new FeedsTreeItem( true, m_tree, QString::null );
    m_feeds.addFeedGroup(elt)->setTitle( i18n("All Feeds") );
    elt->setOpen(true);
}

bool aKregatorView::importFeeds(const QDomDocument& doc)
{
    bool Ok;

    QString text = KInputDialog::getText(i18n("Add Imported Folder"), i18n("Imported folder name:"), i18n("Imported Folder"), &Ok);
    if (!Ok) return false;

    FeedsTreeItem *elt = new FeedsTreeItem( true, m_tree->firstChild(), QString::null );
    m_feeds.addFeedGroup(elt)->setTitle(text);
    elt->setOpen(true);
    return loadFeeds(doc, elt);
}

bool aKregatorView::loadFeeds(const QDomDocument& doc, QListViewItem *parent)
{
    // this should be OPML document
    QDomElement root = doc.documentElement();
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
        parseChildNodes(n, parent);
        curNodes++;
        m_part->setProgress(int(100*((double)curNodes/(double)numNodes)));
        n = n.nextSibling();
    }
    setTotalUnread();
    m_tree->setUpdatesEnabled(true);
    m_tree->triggerUpdate();

    return true;
}

void aKregatorView::parseChildNodes(QDomNode &node, QListViewItem *parent)
{
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

            addFeed_Internal( 0, elt,
                              title,
                              xmlurl,
                              e.attribute("htmlUrl"),
                              e.attribute("description"),
                              e.attribute("updateTitle") == "true" ? true : false, 
                              e.attribute("autoFetch") == "true" ? true : false,
                              e.attribute("fetchInterval").toUInt()
                            );

        }
        else
        {
            m_feeds.addFeedGroup(elt);
            FeedGroup *g = m_feeds.find(elt);
            if (g)
                g->setTitle( e.attribute("text") );

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
                                      QString description, bool updateTitle, bool autoFetch, int fetchInterval)
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
    feed->xmlUrl         = xmlUrl;
    feed->htmlUrl        = htmlUrl;
    feed->description    = description;
    feed->updateTitle    = updateTitle;
    feed->setAutoFetch(autoFetch);
    feed->setFetchInterval(fetchInterval);
    
    connect( feed, SIGNAL(fetched(Feed* )),
             this, SLOT(slotFeedFetched(Feed *)) );

    connect( feed, SIGNAL(fetchError(Feed* )),
             this, SLOT(slotFeedFetchError(Feed *)) );

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

void aKregatorView::slotNormalView()
{
    if (m_viewMode==NormalView)
       return;

    else if (m_viewMode==CombinedView)
    {
        m_articles->show();
        // tell articleview to redisplay+reformat
        ArticleListItem *item = static_cast<ArticleListItem *>(m_articles->currentItem());
        if (item)
        {
            Feed *feed = static_cast<Feed *>(m_feeds.find(m_tree->currentItem()));
            if (feed)
                m_articleViewer->show( feed, item->article() );
        }
    }

    m_panner2->setOrientation(QSplitter::Vertical);
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
        {
            Feed *feed = static_cast<Feed *>(m_feeds.find(m_tree->currentItem()));
            if (feed)
                m_articleViewer->show( feed, item->article() );
        }
    }

    m_panner2->setOrientation(QSplitter::Horizontal);
    m_viewMode=WidescreenView;

    Settings::setViewMode( m_viewMode );
}

void aKregatorView::slotCombinedView()
{
    if (m_viewMode==CombinedView)
       return;

    m_articles->hide();
    m_viewMode=CombinedView;

    ArticleListItem *item = static_cast<ArticleListItem *>(m_articles->currentItem());
    if (item)
    {
        Feed *feed = static_cast<Feed *>(m_feeds.find(m_tree->currentItem()));
        if (feed)
            m_articleViewer->show(feed);
    }

    Settings::setViewMode( m_viewMode );
}

void aKregatorView::slotRemoveTab()
{
    QWidget *w = m_tabs->currentPage ();
    if (w==m_mainTab)
        return;
    m_tabs->removePage(w);
    if (m_tabs->count() <= 1)
        m_tabsClose->setEnabled(false);
}

void aKregatorView::slotTabChanged(QWidget *w)
{
    if (w==m_mainTab)
        m_tabsClose->setEnabled(false);
    else
        m_tabsClose->setEnabled(true);

    KParts::ReadOnlyPart *p;
    if (w==m_mainTab)
        p=m_part;
    else
        p=(static_cast<KHTMLView *>(w))->part();
    m_part->changePart(p);
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
    FeedGroup *feed = static_cast<FeedGroup *>(m_feeds.find(item));
    m_tabs->showPage(m_mainTab);

    if (feed->isGroup())
    {
        m_part->actionCollection()->action("feed_add")->setEnabled(true);
        m_part->actionCollection()->action("feed_add_group")->setEnabled(true);
        m_articles->clear();
        m_articles->setColumnText(0, feed->title());

        if (m_viewMode==CombinedView)
            m_articleViewer->beginWriting();
        slotUpdateArticleList(feed, false);
        if (m_viewMode==CombinedView)
            m_articleViewer->endWriting();
    }
    else
    {
        m_part->actionCollection()->action("feed_add")->setEnabled(false);
        m_part->actionCollection()->action("feed_add_group")->setEnabled(false);

        slotUpdateArticleList( static_cast<Feed *>(feed) );
        if (m_viewMode==CombinedView)
            m_articleViewer->show(static_cast<Feed *>(feed) );
    }

    if (item->parent())
        m_part->actionCollection()->action("feed_remove")->setEnabled(true);
    else
        m_part->actionCollection()->action("feed_remove")->setEnabled(false);

}

void aKregatorView::slotUpdateArticleList(FeedGroup *src, bool onlyUpdateNew)
{
    //kdDebug() << k_funcinfo << src->title() << endl;
    if (!src->isGroup())
    {
        if (m_viewMode==CombinedView)
            m_articleViewer->show(static_cast<Feed *>(src), false);
        slotUpdateArticleList(static_cast<Feed *>(src), false, onlyUpdateNew);
    }
    else
    {
        if (!src->item())
            return;
        for ( QListViewItem *i = src->item()->firstChild()
                ; i ; i = i->nextSibling() )
        {
            FeedGroup *g = m_feeds.find(i);
            if (g)
                slotUpdateArticleList(g, onlyUpdateNew);
        }
    }
}

void aKregatorView::slotUpdateArticleList(Feed *source, bool clear, bool onlyUpdateNew)
{
    m_articles->setUpdatesEnabled(false);
    if (clear)
    {
        m_articles->clear(); // FIXME adding could become rather slow if we store a lot of archive items?
        m_articles->setColumnText(0, source->title());
    }

    if (source->articles.count() > 0)
    {
        MyArticle::List::ConstIterator it;
        MyArticle::List::ConstIterator end = source->articles.end();
        for (it = source->articles.begin(); it != end; ++it)
        {
            if (!onlyUpdateNew || (*it).status()==MyArticle::New)
                itemAdded(new ArticleListItem( m_articles, (*it), source ));
        }
    }
    m_articles->setUpdatesEnabled(true);
    m_articles->triggerUpdate();
}

// NOTE: feed can only be added to a feed group as a child
void aKregatorView::slotFeedAdd()
{
    if (!m_tree->currentItem() || m_feeds.find(m_tree->currentItem())->isGroup() == false)
    {
        KMessageBox::error(this, i18n("You have to choose feed group before adding feed."));
        return;
    }

    QListViewItem *lastChild = m_tree->currentItem()->firstChild();
    while (lastChild && lastChild->nextSibling())
        lastChild = lastChild->nextSibling();

    addFeed(QString::null, lastChild, m_tree->currentItem());

}

void aKregatorView::addFeed(QString url, QListViewItem *after, QListViewItem* parent)
{
    FeedsTreeItem *elt;
    Feed *feed;
    AddFeedDialog *afd = new AddFeedDialog( 0, "add_feed" );

    afd->setURL(url);

    if (afd->exec() != QDialog::Accepted) return;

    QString text=afd->feedTitle;
    feed=afd->feed;

    FeedPropertiesDialog *dlg = new FeedPropertiesDialog( 0, "edit_feed" );

    dlg->setFeedName(text);
    dlg->setUrl(afd->feedURL);
    dlg->selectFeedName();

    if (dlg->exec() != QDialog::Accepted) return;

    if (!parent)
        parent=m_tree->firstChild();

    if (after)
        elt = new FeedsTreeItem(false, parent, after, text);
    else
        elt = new FeedsTreeItem(false, parent, text);

    feed->setItem(elt);

    addFeed_Internal( feed, elt,
                      dlg->feedName(),
                      dlg->url(),
                      feed->htmlUrl,
                      feed->description,
                      false, 
                      dlg->autoFetch(),
                      dlg->fetchInterval()
                    );

    setTotalUnread();

    m_part->setModified(true);
    delete afd;
    delete dlg;
}

void aKregatorView::slotFeedAddGroup()
{
    if (!m_tree->currentItem() || m_feeds.find(m_tree->currentItem())->isGroup() == false)
    {
        KMessageBox::error(this, i18n("You have to choose feed group before adding subgroup."));
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

    m_feeds.addFeedGroup(elt);
    FeedGroup *g = m_feeds.find(elt);
    if (g)
        g->setTitle( text );

    m_part->setModified(true);
}

void aKregatorView::slotFeedRemove()
{
    QListViewItem *elt = m_tree->currentItem();
    if (!elt) return;
    QListViewItem *parent = elt->parent();
    if (!parent) return; // don't delete root element! (safety valve)

    QString msg = elt->childCount() ?
        i18n("<qt>Are you sure you want to delete group<br><b>%1</b><br> and its subgroups and feeds?</qt>") :
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
    dlg->setUrl( feed->xmlUrl );
    dlg->setAutoFetch(feed->autoFetch());
    dlg->setFetchInterval(feed->fetchInterval());
    
    if (dlg->exec() != QDialog::Accepted) return;

    feed->setTitle( dlg->feedName() );
    feed->xmlUrl         = dlg->url();
    feed->setAutoFetch(dlg->autoFetch());
    feed->setFetchInterval(dlg->fetchInterval());

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

void aKregatorView::slotMarkAllRead()
{
   markAllRead(m_tree->currentItem());
}

void aKregatorView::slotOpenHomepage()
{
   QListViewItem *item=m_tree->currentItem();
   Feed *f = static_cast<Feed *>(m_feeds.find(item));
   slotOpenTab(f->htmlUrl);
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

void aKregatorView::findNumFetches(QListViewItem *item)
{
    if (item)
    {
        FeedGroup *fg = m_feeds.find(item);
        if (fg && fg->isGroup()){
            for (QListViewItem *it = item->firstChild(); it; it = it->nextSibling())
                findNumFetches(it);
        }
        else{
            m_fetches++;
        }
    }
}

void aKregatorView::fetchItem(QListViewItem *item)
{
    if (item)
    {
        FeedGroup *fg = m_feeds.find(item);
        if (fg && fg->isGroup())
        {
            kdDebug() << "Fetching folder " << fg->title() << endl;
            for (QListViewItem *it = item->firstChild(); it; it = it->nextSibling())
	    	fetchItem(it);
        }
        else
        {
            Feed *f = static_cast<Feed *>(fg);
            if (f)
	    	f->fetch();
        }
    }
}

void aKregatorView::showFetchStatus(QListViewItem *firstItem)
{
    m_fetchesDone=0;
    m_fetches=0;
    findNumFetches(firstItem);
    if (m_fetches)
    {
        m_part->setStatusBar(i18n("Fetching Feeds..."));
        m_part->setProgress(0);
    }
}

void aKregatorView::slotFetchCurrentFeed()
{
    showFetchStatus(m_tree->currentItem());
    fetchItem(m_tree->currentItem());
}

void aKregatorView::slotFetchAllFeeds()
{
    // this iterator iterates through ALL child items
    showFetchStatus(m_tree->firstChild());

    for (QListViewItemIterator it(m_tree->firstChild()); it.current(); ++it)
    {
        //kdDebug() << "Fetching subitem " << (*it)->text(0) << endl;
        Feed *f = static_cast<Feed *>(m_feeds.find(*it));
        if (f && !f->isGroup())
            f->fetch();
    }
}


void aKregatorView::slotFeedFetched(Feed *feed)
{
    // Feed finished fetching
    // If its a currenly selected feed, update view
    //kdDebug() << k_funcinfo << "BEGIN" << endl;
    if (feed->item() == m_tree->currentItem())
    {
        //kdDebug() << k_funcinfo << "Updating article list" << endl;
        slotUpdateArticleList(feed, false, true);
    }

    // TODO: perhaps schedule save?
    Archive::save(feed);

    // Also, update unread counts

    FeedsTreeItem *fti = static_cast<FeedsTreeItem *>(feed->item());
    if (fti)
        fti->setUnread(feed->unread());

    m_fetchesDone++;
    int p=int(100*((double)m_fetchesDone/(double)m_fetches));
    if (p>=100)
    {
        setTotalUnread(); // used for systray usually, which is slow..
                          // only update once after all feeds fetched.
        m_part->setStatusBar(QString::null);
    }
    m_part->setProgress(p);
    //kdDebug() << k_funcinfo << "END" << endl;
}

void aKregatorView::slotFeedFetchError(Feed *)
{
    m_fetchesDone++;
    int p=int(100*((double)m_fetchesDone/(double)m_fetches));
    if (p>=100)
        m_part->setStatusBar(QString::null);
    m_part->setProgress(p);
}

void aKregatorView::slotMouseButtonPressed(int button, QListViewItem * item, const QPoint &, int)
{
    if (item && button==Qt::MidButton)
    {
        ArticleListItem *i = static_cast<ArticleListItem *>(item);
        if(Settings::mMBBehaviour() == Settings::EnumMMBBehaviour::OpenInExternalBrowser)
            KRun::runURL(i->article().link(), "text/html", false, false);
        else
            slotOpenTab(i->article().link());
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
    m_articleViewer->show( feed, item->article() );
}

void aKregatorView::slotArticleDoubleClicked(QListViewItem *i, const QPoint &, int)
{
    ArticleListItem *item = static_cast<ArticleListItem *>(i);
    if (!item) return;
    if (!item->article().link().isValid()) return;
    // TODO : make this configurable....
    if (Settings::externalBrowserUseKdeDefault())
        KRun::runURL(item->article().link(), "text/html", false, false);
    else
    {
      QString cmd = Settings::externalBrowserCustomCommand();
      QString url = item->article().link().url();
      cmd.replace(QRegExp("%u"), url);
      KProcess *proc = new KProcess;
#if KDE_IS_VERSION(3,1,94)
      QStringList cmdAndArgs = KShell::splitArgs(cmd);
#else
      QStringList cmdAndArgs = QStringList::split(' ',cmd);
#endif
      *proc << cmdAndArgs;
//      This code will also work, but starts an extra shell process.
//      *proc << cmd;
//      proc->setUseShell(true);
      proc->start(KProcess::DontCare);
      delete proc;
    }        

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

    Feed *feed = static_cast<Feed *>(m_feeds.find(item));
    if (feed)
    {
        if (feed->title()!=text)
            m_part->setModified(true);

        feed->setTitle( text );
        if (!feed->isGroup())
            feed->updateTitle = false; // if user edited title by hand, do not update it automagically
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

    QListViewItem *currentItem = m_articles->selectedItem();

    checkItem(m_articles->firstChild());

    if(currentItem)
        m_articles->ensureItemVisible(currentItem);
}

void aKregatorView::checkItem(QListViewItem *i)
{
    ArticleListItem *item = static_cast<ArticleListItem *>(i);
    if (!item)
        return;

    while(item)
    {
        if(itemMatches(item)) {
            item->setVisible(true);
        }
        else
            item->setVisible(false);
        item = static_cast<ArticleListItem *>(item->nextSibling());
    }
}

bool aKregatorView::itemMatches (ArticleListItem *item)
{
    if (!m_currentStatusFilter || !m_currentTextFilter)
        return true;

    return m_currentTextFilter->matches( item->article() ) && m_currentStatusFilter->matches( item->article() );
}

void aKregatorView::itemAdded(ArticleListItem *item)
{
    item->setVisible(itemMatches(item));
}

void aKregatorView::slotMouseOverInfo(const KFileItem *kifi)
{
    if (kifi)
    {
        KFileItem *k=(KFileItem*)kifi;
        m_part->setStatusBar(k->url().prettyURL());//getStatusBarInfo());
    }
    else
    {
    m_part->setStatusBar(QString::null);
    }
}

#include "akregator_view.moc"
