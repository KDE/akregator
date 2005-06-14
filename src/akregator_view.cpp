/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
                  2004 Sashmit Bhaduri <smt@vfemail.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "actionmanager.h"
#include "akregator_part.h"
#include "akregator_view.h"
#include "addfeeddialog.h"
#include "propertiesdialog.h"
#include "frame.h"
#include "fetchqueue.h"
#include "feedlistview.h"
#include "articlelistview.h"
#include "articleviewer.h"
#include "viewer.h"
#include "feed.h"
#include "feeditem.h"
#include "folder.h"
#include "folderitem.h"
#include "feedlist.h"
#include "akregatorconfig.h"
#include "kernel.h"
#include "pageviewer.h"
#include "searchbar.h"
#include "storage.h"
#include "tabwidget.h"
#include "treenode.h"
#include "progressmanager.h"
#include "treenodeitem.h"
#include "notificationmanager.h"

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
#include <qdatetime.h> // for startup time measure
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
    m_displayingAboutPage = false;
    m_currentFrame = 0L;
    setFocusPolicy(QWidget::StrongFocus);

    QVBoxLayout *lt = new QVBoxLayout( this );

    m_feedSplitter = new QSplitter(QSplitter::Horizontal, this, "panner1");
    m_feedSplitter->setOpaqueResize( true );
    lt->addWidget(m_feedSplitter);

    connect (Kernel::self()->fetchQueue(), SIGNAL(fetched(Feed*)), this, SLOT(slotFeedFetched(Feed*)));
    connect (Kernel::self()->fetchQueue(), SIGNAL(signalStarted()), this, SLOT(slotFetchingStarted()));
    connect (Kernel::self()->fetchQueue(), SIGNAL(signalStopped()), this, SLOT(slotFetchingStopped()));

    m_tree = new FeedListView( m_feedSplitter, "FeedListView" );
    ActionManager::getInstance()->initFeedListView(m_tree);

    connect(m_tree, SIGNAL(signalContextMenu(KListView*, TreeNodeItem*, const QPoint&)), this, SLOT(slotFeedTreeContextMenu(KListView*, TreeNodeItem*, const QPoint&)));

    connect(m_tree, SIGNAL(signalNodeSelected(TreeNode*)), this, SLOT(slotNodeSelected(TreeNode*)));

    connect(m_tree, SIGNAL(signalDropped (KURL::List &, TreeNodeItem*,
            FolderItem*)), this, SLOT(slotFeedURLDropped (KURL::List &,
            TreeNodeItem*, FolderItem*)));

    m_tree->setFeedList(m_feedList);
    ProgressManager::self()->setFeedList(m_feedList);
    
    m_feedSplitter->setResizeMode( m_tree, QSplitter::KeepSize );

    m_tabs = new TabWidget(m_feedSplitter);

    connect( m_part, SIGNAL(signalSettingsChanged()), m_tabs, SLOT(slotSettingsChanged()));
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

    m_searchBar = new SearchBar(m_mainTab);

    if ( !Settings::showQuickFilter() )
        m_searchBar->hide();

    mainTabLayout->addWidget(m_searchBar);
    
    m_articleSplitter = new QSplitter(QSplitter::Vertical, m_mainTab, "panner2");

    m_articleList = new ArticleListView( m_articleSplitter, "articles" );
    ActionManager::getInstance()->initArticleListView(m_articleList);
    
    connect( m_articleList, SIGNAL(mouseButtonPressed(int, QListViewItem *, const QPoint &, int)), this, SLOT(slotMouseButtonPressed(int, QListViewItem *, const QPoint &, int)));

    // use selectionChanged instead of clicked
    connect( m_articleList, SIGNAL(signalArticleChosen(const Article&)),
                this, SLOT( slotArticleSelected(const Article&)) );
    connect( m_articleList, SIGNAL(signalDoubleClicked(ArticleItem*, const QPoint&, int)),
                this, SLOT( slotOpenArticleExternal(ArticleItem*, const QPoint&, int)) );

    connect(m_articleList, SIGNAL(signalContextMenu(KListView*, ArticleItem*, const QPoint&)),
            this, SLOT(slotArticleListViewContextMenu(KListView*, ArticleItem*, const QPoint&)));

    m_articleViewer = new ArticleViewer(m_articleSplitter, "article_viewer");
    m_articleViewer->setSafeMode();  // disable JS, Java, etc...
    
    ActionManager::getInstance()->initArticleViewer(m_articleViewer);

    connect(m_searchBar, SIGNAL(signalSearch(const ArticleFilter&, const ArticleFilter&)), m_articleList, SLOT(slotSetFilter(const ArticleFilter&, const ArticleFilter&)));

    connect(m_searchBar, SIGNAL(signalSearch(const ArticleFilter&, const ArticleFilter&)), m_articleViewer, SLOT(slotSetFilter(const ArticleFilter&, const ArticleFilter&)));

    connect( m_articleViewer, SIGNAL(urlClicked(const KURL&, bool)),
                        this, SLOT(slotOpenTab(const KURL&, bool)) );

    connect( m_articleViewer->browserExtension(), SIGNAL(mouseOverInfo(const KFileItem *)),
                                            this, SLOT(slotMouseOverInfo(const KFileItem *)) );

    connect( m_part, SIGNAL(signalSettingsChanged()), m_articleViewer, SLOT(slotPaletteOrFontChanged()));
    QWhatsThis::add(m_articleViewer->widget(), i18n("Browsing area."));
    mainTabLayout->addWidget( m_articleSplitter );

    m_mainFrame=new Frame(this, m_part, m_mainTab, i18n("Articles"), false);
    connectFrame(m_mainFrame);
    m_tabs->addFrame(m_mainFrame);

    m_feedSplitter->setSizes( Settings::splitter1Sizes() );
    m_articleSplitter->setSizes( Settings::splitter2Sizes() );

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

    KConfig *conf = Settings::self()->config();
    conf->setGroup("General");
    if(!conf->readBoolEntry("Disable Introduction", false)) {
        m_articleList->hide();
        m_searchBar->hide();
        m_articleViewer->displayAboutPage();
        m_tabs->setTitle(i18n("About"), m_mainTab);
        m_displayingAboutPage = true;
    }

    m_fetchTimer = new QTimer(this);
    connect( m_fetchTimer, SIGNAL(timeout()), this, SLOT(slotDoIntervalFetches()) );
    m_fetchTimer->start(1000*60);

    // delete expired articles once per hour
    m_expiryTimer = new QTimer(this);
    connect(m_expiryTimer, SIGNAL(timeout()), this,
            SLOT(slotDeleteExpiredArticles()) );
    m_expiryTimer->start(3600*1000);

    QTimer::singleShot(1000, this, SLOT(slotDeleteExpiredArticles()) );

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
    m_shuttingDown = true; // prevents slotFrameChanged from crashing

    m_articleList->slotShowNode(0);
    m_articleViewer->slotShowNode(0);

    Kernel::self()->fetchQueue()->slotAbort();
       
    m_tree->setFeedList(0);
    ProgressManager::self()->setFeedList(0);
    
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
    connect( m_part, SIGNAL(signalSettingsChanged()), page, SLOT(slotPaletteOrFontChanged()));

    connect( page, SIGNAL(setTabIcon(const QPixmap&)),
            this, SLOT(setTabIcon(const QPixmap&)));
    connect( page, SIGNAL(setWindowCaption (const QString &)),
            this, SLOT(slotTabCaption (const QString &)) );
    connect( page, SIGNAL(urlClicked(const KURL &,bool)),
            this, SLOT(slotOpenTab(const KURL &,bool)) );

    Frame *frame=new Frame(this, page, page->widget(), i18n("Untitled"));
    connectFrame(frame);
    m_tabs->addFrame(frame);

    if(!background)
        m_tabs->showPage(page->widget());
    else
        setFocus();

    //if (m_tabs->count() > 1 && m_tabs->currentPageIndex() != 0)
//        m_tabsClose->setEnabled(true);
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
    if (sender() == m_currentFrame)
        emit setStatusBarText(c);
}

void View::slotCaptionChanged(const QString &c)
{
    if (sender() == m_currentFrame)
        emit setWindowCaption(c);
}

void View::slotStarted()
{
    if (sender() == m_currentFrame)
        emit signalStarted(0);
}

void View::slotCanceled(const QString &s)
{
    if (sender() == m_currentFrame)
        emit signalCanceled(s);
}

void View::slotCompleted()
{
    if (sender() == m_currentFrame)
        emit signalCompleted();
}

void View::slotLoadingProgress(int percent)
{
    if (sender() == m_currentFrame)
        emit setProgress(percent);
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
        delete feedList;
        return false;
    }
    
    Folder* fg = new Folder(title);
    m_feedList->rootNode()->appendChild(fg);
    m_feedList->append(feedList, fg);

    return true;
}

bool View::loadFeeds(const QDomDocument& doc, Folder* parent)
{
    FeedList* feedList = FeedList::fromOPML(doc);

    // parsing went wrong
    if (!feedList)
        return false;

    m_tree->setUpdatesEnabled(false);

    if (!parent)
    {
        m_tree->setFeedList(feedList);
        ProgressManager::self()->setFeedList(feedList);
        disconnectFromFeedList(m_feedList);
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

    Folder* group = 0;
    if (!node || !node->isGroup())
    {
        Folder* g = new Folder( groupName );
        m_feedList->rootNode()->appendChild(g);
        group = g;
    }
    else
        group = static_cast<Folder*>(node);

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
        m_articleList->slotShowNode(m_tree->selectedNode());
        m_articleList->show();

        ArticleItem* item = m_articleList->selectedItem();

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
        m_articleList->slotShowNode(m_tree->selectedNode());
        m_articleList->show();

        // tell articleview to redisplay+reformat
        ArticleItem* item = m_articleList->selectedItem();
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

    m_articleList->slotClear();
    m_articleList->hide();
    m_viewMode = CombinedView;

    slotNodeSelected(m_tree->selectedNode());
    Settings::setViewMode( m_viewMode );
}

void View::slotRemoveFrame()
{
    Frame *f = m_tabs->currentFrame();
    if (f == m_mainFrame)
        return;

    if (f->part() != m_part)
        f->part()->deleteLater();

    m_tabs->removeFrame(f);

    if (m_tabs->count() <= 1)
        m_tabsClose->setEnabled(false);
}

void View::slotFrameChanged(Frame *f)
{
    if (m_shuttingDown)
        return;

    m_currentFrame=f;

    m_tabsClose->setEnabled(f != m_mainFrame);

    emit setWindowCaption(f->caption());
    emit setProgress(f->progress());
    emit setStatusBarText(f->statusText());

    m_part->mergePart(m_articleViewer);

    if (f->part() == m_part)
        m_part->mergePart(m_articleViewer);
    else
        m_part->mergePart(f->part());

    f->widget()->setFocus();

    switch (f->state())
    {
        case Frame::Started:
            emit signalStarted(0);
            break;
        case Frame::Canceled:
            emit signalCanceled(QString::null);
            break;
        case Frame::Idle:
        case Frame::Completed:
        default:
            emit signalCompleted();
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

void View::slotFeedTreeContextMenu(KListView*, TreeNodeItem* item, const QPoint& p)
{
    m_tabs->showPage(m_mainTab);
    item->showContextMenu(p);
}

void View::slotArticleListViewContextMenu(KListView*, ArticleItem* item, const QPoint& p)
{
    if (!item)
        return;

    QWidget* w = m_part->factory()->container("article_popup", m_part);
    if (w)
        static_cast<QPopupMenu *>(w)->exec(p);
}


void View::slotMoveCurrentNodeUp()
{
    TreeNode* current = m_tree->selectedNode();
    if (!current)
        return;
    TreeNode* prev = current->prevSibling();
    Folder* parent = current->parent();

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
    Folder* parent = current->parent();

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

    Folder* parent = current->parent();
    Folder* grandparent = current->parent()->parent();

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
        Folder* fg = static_cast<Folder*>(prev);
        current->parent()->removeChild(current);
        fg->appendChild(current);
        m_tree->ensureNodeVisible(current);
    }
}

void View::slotNodeSelected(TreeNode* node)
{
    if (node)
    {
        kdDebug() << "node selected: " << node->title() << endl;
        kdDebug() << "unread: " << node->unread() << endl;
        kdDebug() << "total: " << node->totalCount() << endl;
    }

    if (m_displayingAboutPage)
    {
        m_tabs->setTitle(i18n("Articles"), m_mainTab);
        if (m_viewMode != CombinedView)
            m_articleList->show();
        if (Settings::showQuickFilter())
            m_searchBar->show();
        m_displayingAboutPage = false;
    }

    m_tabs->showPage(m_mainTab);

    m_searchBar->slotClearSearch();
    
    if (m_viewMode == CombinedView)
        m_articleViewer->slotShowNode(node);
    else
    {
        m_articleList->slotShowNode(node);
        m_articleViewer->slotShowSummary(node);
    }

    if (ActionManager::getInstance()->action("feed_remove") )
    {
        if (node != m_feedList->rootNode() )
            ActionManager::getInstance()->action("feed_remove")->setEnabled(true);
        else
            ActionManager::getInstance()->action("feed_remove")->setEnabled(false);
    }
    
}


void View::slotFeedAdd()
{
    Folder* group = 0;
    if (!m_tree->selectedNode())
        group = m_feedList->rootNode(); // all feeds
    else
    {
        if ( m_tree->selectedNode()->isGroup())
            group = static_cast<Folder*>(m_tree->selectedNode());
        else
            group= m_tree->selectedNode()->parent();

    }

    TreeNode* lastChild = group->children().last();

    addFeed(QString::null, lastChild, group, false);
}

void View::addFeed(const QString& url, TreeNode *after, Folder* parent, bool autoExec)
{

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

    Feed* feed = afd->feed;
    delete afd;

    FeedPropertiesDialog *dlg = new FeedPropertiesDialog( 0, "edit_feed" );
    dlg->setFeed(feed);

    dlg->selectFeedName();

    if (!autoExec)
        if (dlg->exec() != QDialog::Accepted)
        {
            delete feed;
            delete dlg;
            return;
        }

    if (!parent)
        parent = m_feedList->rootNode();

    parent->insertChild(feed, after);

    m_tree->ensureNodeVisible(feed);


    delete dlg;
}

void View::slotFeedAddGroup()
{
    TreeNode* node = m_tree->selectedNode();
    TreeNode* after = 0;

    if (!node)
        node = m_tree->rootNode();

    // if a feed is selected, add group next to it
    if (!node->isGroup())
    {
        after = node;
        node = node->parent();
    }

    Folder* currentGroup = static_cast<Folder*> (node);

    bool Ok;

    QString text = KInputDialog::getText(i18n("Add Folder"), i18n("Folder name:"), "", &Ok);

    if (Ok)
    {
        Folder* newGroup = new Folder(text);
        if (!after)
            currentGroup->appendChild(newGroup);
        else
            currentGroup->insertChild(newGroup, after);

        m_tree->ensureNodeVisible(newGroup);
    }
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
    if (KMessageBox::warningContinueCancel(0, msg, selectedNode->isGroup() ? i18n("Delete Folder") : i18n("Delete Feed"), KStdGuiItem::del()) == KMessageBox::Continue)
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
        m_tree->selectedItem()->startRename(0);
        return;
    }

    Feed *feed = static_cast<Feed *>(node);
    if (!feed)
        return;

    FeedPropertiesDialog *dlg = new FeedPropertiesDialog( 0, "edit_feed" );

    dlg->setFeed(feed);

    dlg->exec();

    delete dlg;
}

void View::slotNextTab()
{
    m_tabs->slotNextTab();
}

void View::slotPreviousTab()
{
    m_tabs->slotPreviousTab();
}

void View::slotNextUnreadArticle()
{
    TreeNode* sel = m_tree->selectedNode();
    if (sel && sel->unread() > 0)
        m_articleList->slotNextUnreadArticle();
    else
        m_tree->slotNextUnreadFeed();
}

void View::slotPrevUnreadArticle()
{
    TreeNode* sel = m_tree->selectedNode();
    if (sel && sel->unread() > 0)
        m_articleList->slotPreviousUnreadArticle();
    else
        m_tree->slotPrevUnreadFeed();
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

    switch (Settings::lMBBehaviour())
    {
        case Settings::EnumLMBBehaviour::OpenInExternalBrowser:
            displayInExternalBrowser(feed->htmlUrl());
            break;
        case Settings::EnumLMBBehaviour::OpenInBackground:
            slotOpenTab(feed->htmlUrl(), true);
            break;
        default:
            slotOpenTab(feed->htmlUrl(), false);
    }
}

void View::slotSetTotalUnread()
{
    emit signalUnreadCountChanged( m_feedList->rootNode()->unread() );
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

            uint lastFetch = Backend::Storage::getInstance()->lastFetchFor(f->xmlUrl());

            uint now = QDateTime::currentDateTime().toTime_t();

            if ( interval > 0 && now - lastFetch >= (uint)interval )
            {
                kdDebug() << "interval fetch: " << f->title() << endl;
                Kernel::self()->fetchQueue()->addFeed(f);
                fetch = true;
            }
        }

        i = i->next();
    }
}

void View::slotFetchCurrentFeed()
{
    if ( !m_tree->selectedNode() )
        return;
    m_tree->selectedNode()->slotAddToFetchQueue(Kernel::self()->fetchQueue());
}

void View::slotFetchAllFeeds()
{
    m_feedList->rootNode()->slotAddToFetchQueue(Kernel::self()->fetchQueue());
}

void View::slotFetchingStarted()
{
    m_mainFrame->setState(Frame::Started);
    ActionManager::getInstance()->action("feed_stop")->setEnabled(true);
    m_mainFrame->setStatusText(i18n("Fetching Feeds..."));
}

void View::slotFetchingStopped()
{
    m_mainFrame->setState(Frame::Completed);
    ActionManager::getInstance()->action("feed_stop")->setEnabled(false);
    m_mainFrame->setStatusText(QString::null);
}

void View::slotFeedFetched(Feed *feed)
{
    // iterate through the articles (once again) to do notifications properly
    if (feed->articles().count() > 0)
    {
        QValueList<Article> articles = feed->articles();
        QValueList<Article>::ConstIterator it;
        QValueList<Article>::ConstIterator end = articles.end();
        for (it = articles.begin(); it != end; ++it)
        {
            if ((*it).status()==Article::New && ((*it).feed()->useNotification() || Settings::useNotifications()))
            {
                NotificationManager::self()->slotNotifyArticle(*it);
            }
        }
    }
}

void View::slotMouseButtonPressed(int button, QListViewItem * item, const QPoint &, int)
{
    ArticleItem *i = static_cast<ArticleItem *>(item);
    if (!i)
        return;

    if (button == Qt::MidButton)
    {
        switch (Settings::mMBBehaviour())
        {
            case Settings::EnumMMBBehaviour::OpenInExternalBrowser:
                displayInExternalBrowser(i->article().link());
                break;
            case Settings::EnumMMBBehaviour::OpenInBackground:
                slotOpenTab(i->article().link(),true);
                break;
            default:
                slotOpenTab(i->article().link());
        }
    }
}

void View::slotArticleSelected(const Article& article)
{

    if (m_viewMode == CombinedView)
        return;

    Feed *feed = article.feed();
    if (!feed)
        return;

    Article a(article);
    if (a.status() != Article::Read)
    {
        m_articleList->setReceiveUpdates(false, false);
        a.setStatus(Article::Read);
        m_articleList->setReceiveUpdates(true, false);
    }

    KToggleAction*  maai = dynamic_cast<KToggleAction*>(ActionManager::getInstance()->action("article_set_status_important"));
    maai->setChecked(a.keep());
    kdDebug() << "selected: " << a.guid() << endl;
    
    m_articleViewer->slotShowArticle(a);
}

void View::slotOpenArticleExternal(ArticleItem* item, const QPoint&, int)
{
    if (!item)
        return;
    // TODO : make this configurable....
    displayInExternalBrowser(item->article().link());
}


void View::slotOpenCurrentArticle()
{
    ArticleItem *item = m_articleList->currentItem();
    if (!item)
        return;

    Article article = item->article();
    QString link;
    if (article.link().isValid() || (article.guidIsPermaLink() && KURL(article.guid()).isValid()))
    {
        // in case link isn't valid, fall back to the guid permaLink.
        if (article.link().isValid())
            link = article.link().url();
        else
            link = article.guid();
        slotOpenTab(link, false);
    }
}

void View::slotOpenCurrentArticleExternal()
{
    slotOpenArticleExternal(m_articleList->currentItem(), QPoint(), 0);
}

void View::slotOpenCurrentArticleBackgroundTab()
{
    ArticleItem *item = m_articleList->currentItem();
    if (!item)
        return;

    Article article = item->article();
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

void View::slotFeedURLDropped(KURL::List &urls, TreeNodeItem* after, FolderItem* parent)
{
    Folder* pnode = parent ? parent->node() : 0;
    TreeNode* afternode = after ? after->node() : 0;
    KURL::List::iterator it;
    for ( it = urls.begin(); it != urls.end(); ++it )
    {
        addFeed((*it).prettyURL(), afternode, pnode, false);
    }
}

void View::slotToggleShowQuickFilter()
{
    if ( Settings::showQuickFilter() )
    {
        Settings::setShowQuickFilter(false);
        m_searchBar->slotClearSearch();
        m_searchBar->hide();
    }
    else
    {
        Settings::setShowQuickFilter(true);
        if (!m_displayingAboutPage)
            m_searchBar->show();
    }

}

void View::slotArticleDelete()
{

    if ( m_viewMode == CombinedView )
        return;

    QValueList<ArticleItem*> items = m_articleList->selectedArticleItems(false);

    QString msg;
    switch (items.count())
    {
        case 0:
            return;
        case 1:
            msg = i18n("<qt>Are you sure you want to delete article <b>%1</b>?</qt>").arg(QStyleSheet::escape(items.first()->article().title()));
            break;
        default:
            msg = i18n("<qt>Are you sure you want to delete the %1 selected articles?</qt>").arg(items.count());
    }
    
    if (KMessageBox::warningContinueCancel(0, msg, i18n("Delete Article"), KStdGuiItem::del()) == KMessageBox::Continue)
    {
        if (m_tree->selectedNode())
            m_tree->selectedNode()->setNotificationMode(false);
            
        QValueList<Feed*> feeds;
        for (QValueList<ArticleItem*>::ConstIterator it = items.begin(); it != items.end(); ++it)
        {
            Article article = (*it)->article();
            Feed* feed = article.feed();
            if (!feeds.contains(feed))
                feeds.append(feed);
            feed->setNotificationMode(false);    
            article.setDeleted();
        }

        if (items.count() == 1)
        {
            ArticleItem* ali = *(items.begin());
            if ( ali->nextSibling() )
                ali = ali->nextSibling();
            else
                ali = ali->itemAbove();
            m_articleList->setCurrentItem(ali);
            m_articleList->setSelected(ali, true);
        }
        for (QValueList<Feed*>::Iterator it = feeds.begin(); it != feeds.end(); ++it)
            (*it)->setNotificationMode(true);
        if (m_tree->selectedNode())
            m_tree->selectedNode()->setNotificationMode(true);    
    }
}


void View::slotArticleToggleKeepFlag(bool /*enabled*/)
{
    QValueList<ArticleItem*> items = m_articleList->selectedArticleItems(false);

    if (items.isEmpty())
        return;

    bool allFlagsSet = true;
    for (QValueList<ArticleItem*>::ConstIterator it = items.begin(); allFlagsSet && it != items.end(); ++it)
        if (!(*it)->article().keep())
            allFlagsSet = false;

    for (QValueList<ArticleItem*>::ConstIterator it = items.begin(); it != items.end(); ++it)
        (*it)->article().setKeep(!allFlagsSet);
    m_articleList->slotUpdate();    
}

void View::slotSetSelectedArticleRead()
{
    QValueList<ArticleItem*> items = m_articleList->selectedArticleItems(false);

    if (items.isEmpty())
        return;

    m_articleList->setReceiveUpdates(false, false);        
    for (QValueList<ArticleItem*>::ConstIterator it = items.begin(); it != items.end(); ++it)
        (*it)->article().setStatus(Article::Read);
    m_articleList->setReceiveUpdates(true, false);
}

void View::slotSetSelectedArticleUnread()
{
    QValueList<ArticleItem*> items = m_articleList->selectedArticleItems(false);

    if (items.isEmpty())
        return;

    m_articleList->setReceiveUpdates(false, false);
    for (QValueList<ArticleItem*>::ConstIterator it = items.begin(); it != items.end(); ++it)
        (*it)->article().setStatus(Article::Unread);
    m_articleList->setReceiveUpdates(true, false);
}

void View::slotSetSelectedArticleNew()
{
    QValueList<ArticleItem*> items = m_articleList->selectedArticleItems(false);
    
    if (items.isEmpty())
        return;

    m_articleList->setReceiveUpdates(false, false);        
    
    for (QValueList<ArticleItem*>::ConstIterator it = items.begin(); it != items.end(); ++it)
        (*it)->article().setStatus(Article::New);
    m_articleList->setReceiveUpdates(true, false);
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

void View::readProperties(KConfig* config)
{
    // read filter settings
    m_searchBar->slotSetText(config->readEntry("searchLine"));
    m_searchBar->slotSetStatus(config->readEntry("searchCombo").toInt());
}

void View::saveProperties(KConfig* config)
{
    // save filter settings
    config->writeEntry("searchLine", m_searchBar->text());
    config->writeEntry("searchCombo", m_searchBar->status());
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
