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
#include "feeditem.h"
#include "feedgroup.h"
#include "feedgroupitem.h"
#include "feedlist.h"
#include "akregatorconfig.h"
#include "pageviewer.h"
#include "articlefilter.h"
#include "tabwidget.h"
#include "treenode.h"
#include "treenodeitem.h"

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
#include <kparts/partmanager.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qfile.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmultilineedit.h>
#include <qpopupmenu.h>
#include <qstylesheet.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qvaluevector.h>
#include <qwhatsthis.h>

using namespace Akregator;

View::~View()
{
    // if m_shuttingDown is false, slotOnShutdown was not called. That
     // means that not the whole app is shutdown, only the part. So it
    // should be no risk to do the cleanups now
    if (!m_shuttingDown)
    {
        kdDebug() << "View::~View(): slotOnShutdown() wasn't called. Calling it now." << endl;
        slotOnShutdown();
    }
    kdDebug() << "View::~View(): leaving" << endl;
    
}

View::View( Part *part, QWidget *parent, const char *name)
 : QWidget(parent, name), m_viewMode(NormalView)
{
     m_keepFlagIcon = QPixmap(locate("data", "akregator/pics/akregator_flag.png"));
    m_part = part;
    m_feedList = new FeedList();
    m_shuttingDown = false;
    m_currentFrame = 0L;
    setFocusPolicy(QWidget::StrongFocus);

    QVBoxLayout *lt = new QVBoxLayout( this );

    m_feedSplitter = new QSplitter(QSplitter::Horizontal, this, "panner1");
    m_feedSplitter->setOpaqueResize( true );
    lt->addWidget(m_feedSplitter);

    m_transaction= new FetchTransaction(this);
    connect (m_transaction, SIGNAL(fetched(Feed*)), this, SLOT(slotFeedFetched(Feed*)));
    connect (m_transaction, SIGNAL(fetchError(Feed*)), this, SLOT(slotFeedFetchError(Feed*)));
    connect (m_transaction, SIGNAL(completed()), this, SLOT(slotFetchesCompleted()));

    m_tree = new FeedsTree( m_feedSplitter, "FeedsTree" );

    m_tree->setFeedList(m_feedList);
    
    connect(m_tree, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
            this, SLOT(slotFeedTreeContextMenu(KListView*, QListViewItem*, const QPoint&)));
    
    connect(m_tree, SIGNAL(signalNodeSelected(TreeNode*)), this, SLOT(slotNodeSelected(TreeNode*)));
    
    connect(m_tree, SIGNAL(dropped (KURL::List &, TreeNodeItem*, FeedGroupItem*)),
            this, SLOT(slotFeedURLDropped (KURL::List &,
                        TreeNodeItem*, FeedGroupItem*)));

    m_feedSplitter->setResizeMode( m_tree, QSplitter::KeepSize );

    m_tabs = new TabWidget(m_feedSplitter);

    m_tabsClose = new QToolButton( m_tabs );
    m_tabsClose->setAccel(QKeySequence("Ctrl+W"));
    connect( m_tabsClose, SIGNAL( clicked() ), this,
            SLOT( slotRemoveFrame() ) );

    m_tabsClose->setIconSet( SmallIconSet( "tab_remove" ) );
    m_tabsClose->adjustSize();
    QToolTip::add(m_tabsClose, i18n("Close the current tab"));
    m_tabs->setCornerWidget( m_tabsClose, TopRight );

    connect( m_tabs, SIGNAL( currentFrameChanged(Frame *) ), this,
            SLOT( slotFrameChanged(Frame *) ) );

    QWhatsThis::add(m_tabs, i18n("You can view multiple articles in several open tabs."));

    m_mainTab = new QWidget(this, "Article Tab");
    QVBoxLayout *mainTabLayout = new QVBoxLayout( m_mainTab, 0, 2, "mainTabLayout");

    QWhatsThis::add(m_mainTab, i18n("Articles list."));

    m_searchBar = new QHBox(m_mainTab);
    m_searchBar->setMargin(2);
    m_searchBar->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed ) );
    QToolButton *clearButton = new QToolButton( m_searchBar );
    clearButton->setIconSet( SmallIconSet( QApplication::reverseLayout() ? "clear_left" : "locationbar_erase" ) );
    
    clearButton->setAutoRaise(true);
    
    QLabel* searchLabel = new QLabel(m_searchBar);
    searchLabel->setText( i18n("S&earch:") );
    
    m_searchLine = new KLineEdit(m_searchBar, "searchline");
    searchLabel->setBuddy(m_searchLine);

    m_searchCombo = new KComboBox(m_searchBar, "searchcombo");
    mainTabLayout->add(m_searchBar);
    
    if ( !Settings::showQuickFilter() )    
        m_searchBar->hide();
    
    m_searchCombo->insertItem(i18n("All Articles"));
    m_searchCombo->insertItem(i18n("New & Unread"));
    m_searchCombo->insertItem(i18n("New"));
    m_searchCombo->insertItem(i18n("Unread"));

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
    connect( m_articles, SIGNAL(signalArticleSelected(MyArticle)),
                this, SLOT( slotArticleSelected(MyArticle)) );
    connect( m_articles, SIGNAL(doubleClicked(QListViewItem*, const QPoint&, int)),
                this, SLOT( slotOpenArticleExternal(QListViewItem*, const QPoint&, int)) );

    connect(m_articles, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
            this, SLOT(slotArticleListContextMenu(KListView*, QListViewItem*, const QPoint&)));
    
    m_articleViewer = new ArticleViewer(m_articleSplitter, "article_viewer");
    m_articleViewer->setSafeMode();  // disable JS, Java, etc...

    connect( m_articleViewer, SIGNAL(urlClicked(const KURL&, bool)),
                        this, SLOT(slotOpenTab(const KURL&, bool)) );

    connect( m_articleViewer->browserExtension(), SIGNAL(mouseOverInfo(const KFileItem *)),
                                            this, SLOT(slotMouseOverInfo(const KFileItem *)) );
    
    QWhatsThis::add(m_articleViewer->widget(), i18n("Browsing area."));
    mainTabLayout->addWidget( m_articleSplitter );

    m_mainFrame=new Frame(this, m_part, m_mainTab, i18n("Articles"), false);
    connectFrame(m_mainFrame);
    m_tabs->addFrame(m_mainFrame);

    m_articleViewer->openDefault();
 
    m_feedSplitter->setSizes( Settings::splitter1Sizes() );
    m_articleSplitter->setSizes( Settings::splitter2Sizes() );

    m_searchCombo->setCurrentItem(Settings::quickFilter());
    slotSearchComboChanged(Settings::quickFilter());

    switch (Settings::viewMode())
    {
        case CombinedView:
            slotCombinedView();
            break;
        case WidescreenView:
            slotWidescreenView();
            break;
        default:
            slotNormalView();
    }

    m_fetchTimer=new QTimer(this);
    connect( m_fetchTimer, SIGNAL(timeout()), this, SLOT(slotDoIntervalFetches()) );
    m_fetchTimer->start(1000*60);

    // delete expired articles once per hour
    m_expiryTimer = new QTimer(this);
    connect(m_expiryTimer, SIGNAL(timeout()), this,
            SLOT(slotDeleteExpiredArticles()) );
    m_expiryTimer->start(3600*1000);

    QTimer::singleShot(0, this, SLOT(delayedInit()));
}

void View::delayedInit()
{
    // HACK, FIXME:
    // for some reason, m_part->factory() is NULL at startup of kontact,
    // and thus the article viewer GUI can't be merged when creating the view.
    // Even the delayed init didn't help. Well, we retry every half a second until
    // it works. This is kind of creative, but a dirty hack nevertheless.
    if ( !m_part->mergePart(m_articleViewer) )
        QTimer::singleShot(500, this, SLOT(delayedInit()));
}

void View::slotOnShutdown()
{
    kdDebug() << "entering View::slotOnShutdown()" << endl;
    m_shuttingDown = true; // prevents slotFrameChanged from crashing

    m_transaction->stop();
    delete m_feedList;
    
    // close all pageviewers in a controlled way
    // fixes bug 91660, at least when no part loading data
    m_tabs->setCurrentPage(m_tabs->count()-1); // select last page
    while (m_tabs->count() > 1) // remove frames until only the main frame remains
        slotRemoveFrame();
    
    delete m_mainTab;
    delete m_mainFrame;
}

void View::saveSettings()
{
    Settings::setSplitter1Sizes( m_feedSplitter->sizes() );
    Settings::setSplitter2Sizes( m_articleSplitter->sizes() );
    Settings::setViewMode( m_viewMode );
    Settings::writeConfig();
}

void View::slotOpenTab(const KURL& url, bool background)
{
    PageViewer* page = new PageViewer(this, "page");
    
    connect( page, SIGNAL(setTabIcon(const QPixmap&)),
            this, SLOT(setTabIcon(const QPixmap&)));
    connect( page, SIGNAL(setWindowCaption (const QString &)),
            this, SLOT(slotTabCaption (const QString &)) );
    connect( page, SIGNAL(urlClicked(const KURL &,bool)),
            this, SLOT(slotOpenTab(const KURL &,bool)) );

    Frame *frame=new Frame(this, page, page->widget(), i18n("Untitled"));
    connectFrame(frame);
    m_tabs->addFrame(frame);

    if(!background) {
        m_tabs->showPage(page->widget());
    } else {
        setFocus();
    }
    if (m_tabs->count() > 1 && m_tabs->currentPageIndex() != 0)
        m_tabsClose->setEnabled(true);
    page->openURL(url);
}


void View::setTabIcon(const QPixmap& icon)
{
    const PageViewer *s = dynamic_cast<const PageViewer*>(sender());
    if (s) {
        m_tabs->setTabIconSet(const_cast<PageViewer*>(s)->widget(), icon);
    }
}

void View::connectFrame(Frame *f)
{
    connect(f, SIGNAL(statusText(const QString &)), this, SLOT(slotStatusText(const QString&)));
    connect(f, SIGNAL(captionChanged (const QString &)), this, SLOT(slotCaptionChanged (const QString &)));
    connect(f, SIGNAL(loadingProgress(int)), this, SLOT(slotLoadingProgress(int)) );
    connect(f, SIGNAL(started()), this, SLOT(slotStarted()));
    connect(f, SIGNAL(completed()), this, SLOT(slotCompleted()));
    connect(f, SIGNAL(canceled(const QString &)), this, SLOT(slotCanceled(const QString&)));
}

void View::slotStatusText(const QString &c)
{
    const Frame *f = dynamic_cast<const Frame *>(sender());
    if (!f) return;
    if (m_currentFrame != f) return;

    m_part->setStatusBar(c);
}

void View::slotCaptionChanged(const QString &c)
{
    const Frame *f = dynamic_cast<const Frame *>(sender());
    if (!f) return;
    if (m_currentFrame != f) return;

    m_part->setCaption(c);
}

void View::slotStarted()
{
    const Frame *f = dynamic_cast<const Frame *>(sender());
    if (!f) return;
    if (m_currentFrame != f) return;

    m_part->setStarted(f->part());
}

void View::slotCanceled(const QString &s)
{
    const Frame *f = dynamic_cast<const Frame *>(sender());
    if (!f) return;
    if (m_currentFrame != f) return;

    m_part->setCanceled(f->part(), s);
}

void View::slotCompleted()
{
    const Frame *f = dynamic_cast<const Frame *>(sender());
    if (!f) return;
    if (m_currentFrame != f) return;

    m_part->setCompleted(f->part());
}

void View::slotLoadingProgress(int percent)
{
    const Frame *f = dynamic_cast<const Frame *>(sender());
    if (!f) return;
    if (m_currentFrame != f) return;

    m_part->setProgress(percent);
}

QString View::getTitleNodeText(const QDomDocument &doc)
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


bool View::importFeeds(const QDomDocument& doc)
{
    FeedList* feedList = FeedList::fromOPML(doc);

    // FIXME: parsing error, print some message
    if (!feedList)
        return false;
    
    QString title = feedList->title();

    if (title.isEmpty())
        title = i18n("Imported Folder");
    
    bool ok;
    title = KInputDialog::getText(i18n("Add Imported Folder"), i18n("Imported folder name:"), title, &ok);

    if (!ok)
    {
        delete m_feedList;
        return false;
    }
    FeedGroup* fg = new FeedGroup(title);
    m_feedList->rootNode()->appendChild(fg);
    m_feedList->append(feedList, fg);

    return true;
}

bool View::loadFeeds(const QDomDocument& doc, FeedGroup* parent)
{
    FeedList* feedList = FeedList::fromOPML(doc);

    // parsing went wrong
    if (!feedList)
        return false;
    
    m_tree->setUpdatesEnabled(false);
    
    if (!parent)
    {
        m_tree->setFeedList(feedList);
        disconnectFromFeedList(feedList);
        delete m_feedList;
        m_feedList = feedList;
        connectToFeedList(feedList);
    }
    else
        m_feedList->append(feedList, parent);     

    m_tree->setUpdatesEnabled(true);
    m_tree->triggerUpdate();
    
    return true;
}

void View::slotDeleteExpiredArticles()
{
    TreeNode* rootNode = m_feedList->rootNode();
    if (rootNode)
        rootNode->slotDeleteExpiredArticles();
}

QDomDocument View::feedListToOPML()
{
    return m_feedList->toOPML();
}

void View::addFeedToGroup(const QString& url, const QString& groupName)
{
    
    // Locate the group.
    TreeNode* node = m_tree->findNodeByTitle(groupName);
                    
    FeedGroup* group = 0;
    if (!node || !node->isGroup())
    {
        FeedGroup* g = new FeedGroup( groupName );
        m_feedList->rootNode()->appendChild(g);
        group = g;
    }
    else
        group = static_cast<FeedGroup*>(node);
    
    // Invoke the Add Feed dialog with url filled in.
    if (group)
        addFeed(url, 0, group, true);
}

void View::slotNormalView()
{
    if (m_viewMode == NormalView)
    return;

    if (m_viewMode == CombinedView)
    {
        m_articles->slotShowNode(m_tree->selectedNode());
        m_articles->show();

        ArticleListItem* item = static_cast<ArticleListItem *>(m_articles->selectedItem());

        if (item)
            m_articleViewer->slotShowArticle(item->article());
        else
            m_articleViewer->slotShowSummary(m_tree->selectedNode());
    }

    m_articleSplitter->setOrientation(QSplitter::Vertical);
    m_viewMode = NormalView;

    Settings::setViewMode( m_viewMode );
}

void View::slotWidescreenView()
{
    if (m_viewMode == WidescreenView)
    return;
    
    if (m_viewMode == CombinedView)
    {
        m_articles->slotShowNode(m_tree->selectedNode());
        m_articles->show();
        
        // tell articleview to redisplay+reformat
        ArticleListItem* item = static_cast<ArticleListItem *>(m_articles->selectedItem());
        if (item)
            m_articleViewer->slotShowArticle(item->article());
        else
            m_articleViewer->slotShowSummary(m_tree->selectedNode());
    }

    m_articleSplitter->setOrientation(QSplitter::Horizontal);
    m_viewMode = WidescreenView;

    Settings::setViewMode( m_viewMode );
}

void View::slotCombinedView()
{
    if (m_viewMode == CombinedView)
        return;

    m_articles->slotClear();
    m_articles->hide();
    m_viewMode = CombinedView;
    
    slotNodeSelected(m_tree->selectedNode());
    Settings::setViewMode( m_viewMode );
}


void View::startOperation()
{
    m_mainFrame->setState(Frame::Started);
    m_part->actionCollection()->action("feed_fetch")->setEnabled(false);
    m_part->actionCollection()->action("feed_fetch_all")->setEnabled(false);
    m_mainFrame->setProgress(0);
}

void View::endOperation()
{
    m_mainFrame->setState(Frame::Completed);
    m_part->actionCollection()->action("feed_fetch")->setEnabled(true);
    m_part->actionCollection()->action("feed_fetch_all")->setEnabled(true);
    m_mainFrame->setProgress(100);
}

void View::operationError(/*const QString& msg*/)
{
    m_mainFrame->setState(Frame::Canceled);
    m_part->actionCollection()->action("feed_fetch")->setEnabled(true);
    m_part->actionCollection()->action("feed_fetch_all")->setEnabled(true);
    m_mainFrame->setProgress(-1);
}

void View::slotRemoveFrame()
{
    Frame *f = m_tabs->currentFrame();
    if (f == m_mainFrame)
        return;
    
    m_tabs->removeFrame(f);
    
    if (f->part() != m_part)
        delete f->part();
    
    delete f;
    if (m_tabs->count() <= 1)
        m_tabsClose->setEnabled(false);
}

void View::slotFrameChanged(Frame *f)
{
    if (m_shuttingDown)
        return;
    
    m_currentFrame=f;

    m_tabsClose->setEnabled(f != m_mainFrame);

    m_part->setCaption(f->caption());
    m_part->setProgress(f->progress());
    m_part->setStatusBar(f->statusText());

    m_part->mergePart(m_articleViewer);
    
    if (f == m_mainFrame)
        m_mainFrame->widget()->setFocus();
    
    if (f->part() == m_part)
        m_part->mergePart(m_articleViewer);
    else
        m_part->mergePart(f->part());
    
    switch (f->state())
    {
        case Frame::Started:
            m_part->setStarted(f->part());
            break;
        case Frame::Canceled:
            m_part->setCanceled(f->part(), QString::null);
            break;
        case Frame::Idle:
        case Frame::Completed:
        default:
            m_part->setCompleted(f->part());
    }
}

void View::slotTabCaption(const QString &caption)
{
    if (!caption.isEmpty())
    {
        PageViewer *pv=(PageViewer *)sender();
        m_tabs->setTitle(caption, pv->widget());
        pv->slotSetCaption(caption);
    }
}

void View::slotFeedTreeContextMenu(KListView*, QListViewItem* item, const QPoint& p)
{
    TreeNodeItem* ti = static_cast<TreeNodeItem*>(item); 
    TreeNode* node = ti ? ti->node() : 0;

    if (!node)
        return;
    
    m_tabs->showPage(m_mainTab);
    
    QWidget *w;
    if (node->isGroup())
        w = m_part->factory()->container("feedgroup_popup", m_part);
    else
        w = m_part->factory()->container("feeds_popup", m_part);
    if (w)
        static_cast<QPopupMenu *>(w)->exec(p);
}

void View::slotArticleListContextMenu(KListView*, QListViewItem* item, const QPoint& p)
{
    ArticleListItem* ali = static_cast<ArticleListItem*> (item);
    if (!ali)
        return;
    KToggleAction* ka = static_cast<KToggleAction*> (m_part->actionCollection()->action("article_toggle_keep"));
    if (ka)
        ka->setChecked( ali->article().keep() );
    QWidget* w = m_part->factory()->container("article_popup", m_part);
    if (w)
        static_cast<QPopupMenu *>(w)->exec(p);
}

void View::slotPreviousArticle()
{
    m_articles->slotPreviousArticle(); 
}    

void View::slotNextArticle()
{ 
    m_articles->slotNextArticle(); 
}

void View::slotFeedsTreeUp()
{
    m_tree->slotItemUp();
}

void View::slotFeedsTreeDown()
{
    m_tree->slotItemDown();
}

void View::slotFeedsTreeLeft()
{
    m_tree->slotItemLeft();
}

void View::slotFeedsTreeRight()
{
    m_tree->slotItemRight();
}

void View::slotFeedsTreePageUp()
{
}

void View::slotFeedsTreePageDown()
{
}

void View::slotFeedsTreeHome()
{
    m_tree->slotItemBegin();
}

void View::slotFeedsTreeEnd()
{
    m_tree->slotItemEnd();
}


void View::slotMoveCurrentNodeUp()
{
    TreeNode* current = m_tree->selectedNode();
    if (!current)
        return;
    TreeNode* prev = current->prevSibling();
    FeedGroup* parent = current->parent();
    
    if (!prev || !parent)
        return;
    
    parent->removeChild(prev);
    parent->insertChild(prev, current);
    m_tree->ensureNodeVisible(current);
}

void View::slotMoveCurrentNodeDown()
{
    TreeNode* current = m_tree->selectedNode();
    if (!current)
        return;
    TreeNode* next = current->nextSibling();
    FeedGroup* parent = current->parent();
    
    if (!next || !parent)
        return;
    
    parent->removeChild(current);
    parent->insertChild(current, next);
    m_tree->ensureNodeVisible(current);
}

void View::slotMoveCurrentNodeLeft()
{
    TreeNode* current = m_tree->selectedNode();
    if (!current || !current->parent() || !current->parent()->parent())
        return;
    
    FeedGroup* parent = current->parent();
    FeedGroup* grandparent = current->parent()->parent();

    parent->removeChild(current);
    grandparent->insertChild(current, parent);
    m_tree->ensureNodeVisible(current);
}

void View::slotMoveCurrentNodeRight()
{
    TreeNode* current = m_tree->selectedNode();
    if (!current || !current->parent())
        return;
    TreeNode* prev = current->prevSibling();
    
    if ( prev && prev->isGroup() )
    {
        FeedGroup* fg = static_cast<FeedGroup*>(prev);
        current->parent()->removeChild(current);
        fg->appendChild(current);
        m_tree->ensureNodeVisible(current);
    }    
}

void View::slotNodeSelected(TreeNode* node)
{
    m_tabs->showPage(m_mainTab);

    if (m_viewMode == CombinedView)
        m_articleViewer->slotShowNode(node);
    else
    {
        m_articles->slotShowNode(node);
        m_articleViewer->slotShowSummary(node);
    }

    if (m_part->actionCollection()->action("feed_remove") )
    {
        if (node != m_feedList->rootNode() )
            m_part->actionCollection()->action("feed_remove")->setEnabled(true);
        else
            m_part->actionCollection()->action("feed_remove")->setEnabled(false);
    }
}


void View::slotFeedAdd()
{
    FeedGroup* group = 0;
    if (!m_tree->selectedNode())
        group = m_feedList->rootNode(); // all feeds
    else
    {
        if ( m_tree->selectedNode()->isGroup())
            group = static_cast<FeedGroup*>(m_tree->selectedNode());
        else 
            group= m_tree->selectedNode()->parent(); 
            
    }

    TreeNode* lastChild = group->children().last();
    
    addFeed(QString::null, lastChild, group, false);
}

void View::addFeed(const QString& url, TreeNode *after, FeedGroup* parent, bool autoExec)
{
    Feed *feed;
    AddFeedDialog *afd = new AddFeedDialog( 0, "add_feed" );
    
    afd->setURL(KURL::decode_string(url));

    if (autoExec)
        afd->slotOk();
    else
    {
        if (afd->exec() != QDialog::Accepted) 
        {  
            delete afd;  
            return;
        }    
    }
    
    feed = afd->feed;
    FeedPropertiesDialog *dlg = new FeedPropertiesDialog( 0, "edit_feed" );
    
    dlg->setFeedName( feed->title() );
    dlg->setUrl(afd->feedURL);
    dlg->selectFeedName();
    dlg->setMaxArticleAge(60);
    dlg->setMaxArticleNumber(1000);
    dlg->setFetchInterval(30);
    if (!autoExec)
        if (dlg->exec() != QDialog::Accepted) 
        {
            delete dlg;   
            return;
        }    

    feed->setNotificationMode(false);    
    feed->setTitle(dlg->feedName());
    feed->setXmlUrl(dlg->url());
    feed->setArchiveMode(dlg->archiveMode());
    feed->setMaxArticleAge(dlg->maxArticleAge());
    feed->setMaxArticleNumber(dlg->maxArticleNumber());
    feed->setNotificationMode(true, true);    
    feed->setCustomFetchIntervalEnabled(dlg->autoFetch());
    feed->setFetchInterval(dlg->fetchInterval());

    Archive::load(feed);
    if (!parent)
        parent = m_feedList->rootNode();
    
    parent->insertChild(feed, after);
        
    m_tree->ensureNodeVisible(feed);
    
    delete afd;
    delete dlg;
}

void View::slotFeedAddGroup()
{
TreeNode* node = m_tree->selectedNode();

    if (!node || !node->isGroup())
    {
        KMessageBox::error(this, i18n("You have to choose a folder before adding a subfolder."));
        return;
    }

    FeedGroup* currentGroup = static_cast<FeedGroup*> (node);
    
    bool Ok;    
    QString text = KInputDialog::getText(i18n("Add Folder"), i18n("Folder name:"), "", &Ok);
    
    if (!Ok) 
        return;

    FeedGroup* newGroup = new FeedGroup(text);
    currentGroup->appendChild(newGroup);
    
    //m_tree->ensureItemVisible(elt); // FIXME

    //m_part->setModified(true);
}

void View::slotFeedRemove()
{
    TreeNode* selectedNode = m_tree->selectedNode();
    
    // don't delete root element! (safety valve)
    if (!selectedNode || selectedNode == m_feedList->rootNode())
        return;
    
    QString msg;
   
    if (selectedNode->title().isEmpty()) {
        msg = selectedNode->isGroup() ?
            i18n("<qt>Are you sure you want to delete this folder and its feeds and subfolders?</qt>") :
            i18n("<qt>Are you sure you want to delete this feed?</qt>");
    } else {
        msg = selectedNode->isGroup() ?
            i18n("<qt>Are you sure you want to delete folder<br><b>%1</b><br> and its feeds and subfolders?</qt>") :
            i18n("<qt>Are you sure you want to delete feed<br><b>%1</b>?</qt>");
        msg = msg.arg(selectedNode->title());
    }
    if (KMessageBox::warningContinueCancel(0, msg, i18n("Delete Feed"), KStdGuiItem::del()) == KMessageBox::Continue)
    {
        delete selectedNode;
        m_tree->setFocus();
     }
}

void View::slotFeedModify()
{
    TreeNode* node = m_tree->selectedNode();
    
    if (node && node->isGroup())
    {
        m_tree->selectedItem()->setRenameEnabled(0, true);
        m_tree->selectedItem()->startRename(0);
        return;
    }

    Feed *feed = static_cast<Feed *>(node);
    if (!feed)
        return;

    FeedPropertiesDialog *dlg = new FeedPropertiesDialog( 0, "edit_feed" );

    dlg->setFeedName( feed->title() );
    dlg->setUrl( feed->xmlUrl() );
    dlg->setAutoFetch(feed->useCustomFetchInterval());
    dlg->setFetchInterval(feed->fetchInterval());
    dlg->setArchiveMode(feed->archiveMode());
    dlg->setMaxArticleAge(feed->maxArticleAge());
    dlg->setMaxArticleNumber(feed->maxArticleNumber());
    
    if (dlg->exec() == QDialog::Accepted) 
    {   
        feed->setNotificationMode(false);
        feed->setTitle( dlg->feedName() );
        feed->setXmlUrl( dlg->url() );
        feed->setCustomFetchIntervalEnabled(dlg->autoFetch());
        feed->setFetchInterval(dlg->fetchInterval());
        feed->setArchiveMode(dlg->archiveMode());
        feed->setMaxArticleAge(dlg->maxArticleAge());
        feed->setMaxArticleNumber(dlg->maxArticleNumber());
        feed->setNotificationMode(true, true);
        //m_part->setModified(true);
        if ( feed->isMerged() )
            Archive::save(feed);

    }
    delete dlg;
}

void View::slotPrevFeed()
{
    m_tree->slotPrevFeed();
}
    
void View::slotNextFeed()
{
    m_tree->slotNextFeed();
}

void View::slotNextUnreadArticle()
{
    m_articles->slotNextUnreadArticle();
}

void View::slotPrevUnreadArticle()
{
    m_articles->slotPreviousUnreadArticle();
}

void View::slotPrevUnreadFeed()
{
    m_tree->slotPrevUnreadFeed();
}

void View::slotNextUnreadFeed()
{
    m_tree->slotNextUnreadFeed();
}

void View::slotMarkAllFeedsRead()
{
    m_feedList->rootNode()->slotMarkAllArticlesAsRead();
}

void View::slotMarkAllRead()
{
    if(!m_tree->selectedNode()) return;
    m_tree->selectedNode()->slotMarkAllArticlesAsRead();
}

void View::slotOpenHomepage()
{
Feed* feed = static_cast<Feed *>(m_tree->selectedNode());

if (!feed || feed->isGroup())
    return;

if(Settings::mMBBehaviour() == Settings::EnumMMBBehaviour::OpenInExternalBrowser)
    displayInExternalBrowser(feed->htmlUrl());
else
    slotOpenTab(feed->htmlUrl(), Settings::backgroundTabForArticles());
}

void View::slotSetTotalUnread()
{
    emit signalUnreadCountChanged( m_feedList->rootNode()->unread() );
}

void View::showFetchStatus()
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
void View::displayInExternalBrowser(const KURL &url)
{
    if (!url.isValid()) return;
    if (Settings::externalBrowserUseKdeDefault())
        kapp->invokeBrowser(url.url(), "0");
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

void View::slotDoIntervalFetches()
{
    if ( m_transaction->isRunning() || m_part->isLoading() )
        return;

    bool fetch = false;
    TreeNode* i = m_feedList->rootNode()->firstChild();

    while ( i && i != m_feedList->rootNode() )
    {
        if ( !i->isGroup() )
        {
            Feed* f = static_cast<Feed*> (i);

            int interval = -1;

            if ( f->useCustomFetchInterval() )
                interval = f->fetchInterval() * 60;
            else
                if ( Settings::useIntervalFetch() )
                    interval = Settings::autoFetchInterval() * 60;

            uint lastFetch = IntervalManager::self()->lastFetchTime(f->xmlUrl());
            
            uint now = QDateTime::currentDateTime().toTime_t();
            
            if ( interval > 0 && now - lastFetch >= (uint)interval )
            {
                kdDebug() << "AkregatorView::slotDoIntervalFetches: interval fetch " << f->title() << endl;
                m_transaction->addFeed(f);
                fetch = true;
            }
        }

        i = i->next();
    }
    
    if (fetch)
    {
        startOperation();
        m_transaction->start();
    }
}

void View::slotFetchCurrentFeed()
{
    if ( !m_tree->selectedNode() )
        return;
    showFetchStatus();
    m_tree->selectedNode()->slotAddToFetchTransaction(m_transaction);
    startOperation();
    m_transaction->start();
}

void View::slotFetchAllFeeds()
{
    // this iterator iterates through ALL child items
    showFetchStatus();

    m_feedList->rootNode()->slotAddToFetchTransaction(m_transaction);
    startOperation();
    m_transaction->start();
}

void View::slotFetchesCompleted()
{
    endOperation();
    m_mainFrame->setStatusText(QString::null);
}

void View::slotFeedFetched(Feed *feed)
{
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

    int p=int(100*((double)m_transaction->fetchesDone()/(double)m_transaction->totalFetches()));
    m_mainFrame->setProgress(p);
}

void View::slotFeedFetchError(Feed* /*feed*/)
{
    int p = int(100*((double)m_transaction->fetchesDone()/(double)m_transaction->totalFetches()));
    m_mainFrame->setProgress(p);
}

void View::slotMouseButtonPressed(int button, QListViewItem * item, const QPoint &, int)
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

void View::slotArticleSelected(MyArticle article)
{
    
    if (m_viewMode == CombinedView) 
        return; 

    Feed *feed = article.feed();
    if (!feed) 
        return;

    KToggleAction* ka = static_cast<KToggleAction*> (m_part->actionCollection()->action("article_toggle_keep"));
    if (ka)
        ka->setChecked( article.keep() );
    
    if (article.status() != MyArticle::Read)
    {
        article.setStatus(MyArticle::Read);
        //item->repaint();
        int unread = feed->unread();
        unread--;
        m_articles->setReceiveUpdates(false);
        feed->setUnread(unread);
        m_articles->setReceiveUpdates(true, false);
        
        // TODO: schedule this save.. don't want to save a huge file for one change
        Archive::save(feed);
    }
    m_articleViewer->slotShowArticle( article );
}

void View::slotOpenArticleExternal(QListViewItem* i, const QPoint&, int)
{
    ArticleListItem *item = static_cast<ArticleListItem *>(i);
    if (!item) 
        return;
    // TODO : make this configurable....
    displayInExternalBrowser(item->article().link());
}   


void View::slotOpenCurrentArticle()
{
    ArticleListItem *item = static_cast<ArticleListItem *>(m_articles->currentItem());
    if (!item)
        return;
    
    MyArticle article = item->article();
    QString link;
    if (article.link().isValid() || (article.guidIsPermaLink() && KURL(article.guid()).isValid()))
    {
        // in case link isn't valid, fall back to the guid permaLink.
        if (article.link().isValid())
            link = article.link().url();
        else
            link = article.guid();
        slotOpenTab(link, Settings::backgroundTabForArticles());
    }
}

void View::slotOpenCurrentArticleExternal()
{
    slotOpenArticleExternal(m_articles->currentItem(), QPoint(), 0);
}

void View::slotOpenCurrentArticleBackgroundTab()
{
    ArticleListItem *item = static_cast<ArticleListItem *>(m_articles->currentItem());
    if (!item)
        return;

    MyArticle article = item->article();
    QString link;
    if (article.link().isValid() || (article.guidIsPermaLink() && KURL(article.guid()).isValid()))
    {
        // in case link isn't valid, fall back to the guid permaLink.
        if (article.link().isValid())
            link = article.link().url();
        else
            link = article.guid();
        slotOpenTab(link, true);
    }
}

void View::slotPrint()
{
    m_articleViewer->slotPrint();
}

void View::slotFeedURLDropped(KURL::List &urls, TreeNodeItem* after, FeedGroupItem* parent)
{
    FeedGroup* pnode = parent ? parent->node() : 0;
    TreeNode* afternode = after ? after->node() : 0;
    KURL::List::iterator it;
    for ( it = urls.begin(); it != urls.end(); ++it )
    {
        addFeed((*it).prettyURL(), afternode, pnode, false);
    }
}

void View::slotSearchComboChanged(int index)
{
    Settings::setQuickFilter( index );
    updateSearch();
}

// from klistviewsearchline
void View::slotSearchTextChanged(const QString &search)
{
    m_queuedSearches++;
    m_queuedSearch = search;
    QTimer::singleShot(200, this, SLOT(slotActivateSearch()));
}

void View::slotActivateSearch()
{
    m_queuedSearches--;

    if(m_queuedSearches == 0)
        updateSearch(m_queuedSearch);
}

void View::updateSearch(const QString &s)
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
            case 1: // New & Unread
            {
                Criterion crit1( Criterion::Status, Criterion::Equals, MyArticle::New);
                Criterion crit2( Criterion::Status, Criterion::Equals, MyArticle::Unread);
                statusCriteria << crit1;
                statusCriteria << crit2;
                break;
            }
            case 2: // New
            {
                Criterion crit( Criterion::Status, Criterion::Equals, MyArticle::New);
                statusCriteria << crit;
                break;
            }
            case 3: // Unread
            {
                Criterion crit( Criterion::Status, Criterion::Equals, MyArticle::Unread);
                statusCriteria << crit;
                break;
            }
            default:
                break;
        }
    }

    m_currentTextFilter = new ArticleFilter(textCriteria, ArticleFilter::LogicalOr, ArticleFilter::Notify);
    m_currentStatusFilter = new ArticleFilter(statusCriteria, ArticleFilter::LogicalOr, ArticleFilter::Notify);


    m_articleViewer->slotSetFilter(*m_currentTextFilter, *m_currentStatusFilter);
    m_articles->slotSetFilter(*m_currentTextFilter, *m_currentStatusFilter);
}

void View::slotToggleShowQuickFilter()
{
    if ( Settings::showQuickFilter() )
    {
        Settings::setShowQuickFilter(false);
        m_searchBar->hide();
        m_searchLine->clear();
        m_searchCombo->setCurrentItem(0);
        updateSearch();
    }
    else
    {
        Settings::setShowQuickFilter(true);
        m_searchBar->show();
    }
    
}

void View::slotArticleDelete()
{

    if ( m_viewMode == CombinedView )
        return;
    
    ArticleListItem* ali = dynamic_cast<ArticleListItem*>(m_articles->selectedItem());

    if (!ali)
        return;

    QString msg = i18n("<qt>Are you sure you want to delete article <b>%1</b>?</qt>").arg(QStyleSheet::escape(ali->article().title()));
                
    if (KMessageBox::warningContinueCancel(0, msg, i18n("Delete Article"), KStdGuiItem::del()) == KMessageBox::Continue)
    {
        MyArticle article = ali->article();
        article.setDeleted();
        if ( ali->nextSibling() )
            ali = dynamic_cast<ArticleListItem*>(ali->nextSibling());
        else
            ali = dynamic_cast<ArticleListItem*>(ali->itemAbove());

        if (ali)
        {
            m_articles->setCurrentItem(ali);
            m_articles->setSelected(ali, true);
        }
        else
        {
            m_articleViewer->slotClear();
        }
        m_articles->slotUpdate();
        Archive::save(article.feed());
    }
}

    
void View::slotArticleToggleKeepFlag()
{
    ArticleListItem* ali = static_cast<ArticleListItem*>(m_articles->selectedItem());

    if (!ali)
        return;

    bool keep = !ali->article().keep();
    
    if (keep)
        ali->setPixmap(0, m_keepFlagIcon);
    else
        ali->setPixmap(0, QPixmap() );
    KToggleAction* ka = static_cast<KToggleAction*>    (m_part->actionCollection()->action("article_toggle_keep"));
    if (ka)
        ka->setChecked( keep );
    
    ali->article().setKeep(keep);
    Archive::save( ali->article().feed() );
}

void View::slotSetSelectedArticleUnread()
{
    ArticleListItem* ali = static_cast<ArticleListItem*>(m_articles->selectedItem());

    if (!ali)
        return;

    MyArticle article = ali->article();
    Feed* feed = article.feed();
    if (article.status() != MyArticle::Unread)
    {
        article.setStatus(MyArticle::Unread);
        int unread = feed->unread();
        unread++;
        m_articles->setReceiveUpdates(false);
        feed->setUnread(unread);
        m_articles->setReceiveUpdates(true, false);
        
        // TODO: schedule this save.. don't want to save a huge file for one change
        Archive::save(feed);
    }

}

void View::slotSetSelectedArticleNew()
{
    ArticleListItem* ali = static_cast<ArticleListItem*>(m_articles->selectedItem());

    if (!ali)
        return;

    MyArticle article = ali->article();
    Feed* feed = article.feed();
    if (article.status() != MyArticle::New)
    {
        article.setStatus(MyArticle::New);
        int unread = feed->unread();
        unread++;
        m_articles->setReceiveUpdates(false);
        feed->setUnread(unread);
        m_articles->setReceiveUpdates(true, false);
        
        // TODO: schedule this save.. don't want to save a huge file for one change
        Archive::save(feed);
    }

}

void View::slotMouseOverInfo(const KFileItem *kifi)
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

void View::readProperties(KConfig* config) // this is called when session is being restored
{
    // load the standard feedlist, fixes #84528, at least partially -tpr 20041025
    m_part->openStandardFeedList();
    
    // read filter settings 
    m_searchLine->setText(config->readEntry("searchLine"));
    m_searchCombo->setCurrentItem(config->readEntry("searchCombo").toInt());
    slotSearchComboChanged(config->readEntry("searchCombo").toInt());

    // read the position of the selected feed

    QString selectedFeed = config->readEntry("selectedFeed");
    if ( selectedFeed.isNull() )
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
            if ( selectedArticleEntry.isNull() )
            {
                QListViewItem* selectedArticle = m_articles->findItem(selectedArticleEntry, 0);
                if ( selectedArticle )
                    m_articles->setSelected(selectedArticle, true);
            }
        } // if viewMode != combinedView
    } // if selectedFeed is set
}

// this is called when using session management and session is going to close
void View::saveProperties(KConfig* config)
{   
    // save the feedlist, fixes #84528, at least partially -tpr 20041025
    m_part->saveFeedList();
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

void View::connectToFeedList(FeedList* feedList)
{
    connect(feedList->rootNode(), SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotSetTotalUnread()));
    slotSetTotalUnread();
}

void View::disconnectFromFeedList(FeedList* feedList)
{
    disconnect(feedList->rootNode(), SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotSetTotalUnread()));
}
#include "akregator_view.moc"

// vim: set et ts=4 sts=4 sw=4:
