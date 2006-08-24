/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
                  2004 Sashmit Bhaduri <smt@vfemail.net>
                  2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "actionmanagerimpl.h"
#include "addfeeddialog.h"
#include "articlelistview.h"
#include "articleviewer.h"
#include "akregatorconfig.h"
#include "akregator_part.h"
#include "browserframe.h"
#include "feed.h"
#include "feedlist.h"
#include "feedlistview.h"
#include "fetchqueue.h"
#include "folder.h"
#include "framemanager.h"
#include "kernel.h"
#include "listtabwidget.h"
#include "mainwidget.h"
#include "notificationmanager.h"
#include "openurlrequest.h"
#include "propertiesdialog.h"
#include "progressmanager.h"
#include "searchbar.h"
//#include "speechclient.h"
#include "tabwidget.h"
#include "tag.h"
#include "tagfolder.h"
#include "tagnode.h"
#include "tagnodelist.h"
#include "tagpropertiesdialog.h"
#include "tagset.h"
#include "treenode.h"
#include "treenodevisitor.h"

#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <kfileitem.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <krandom.h>
#include <kshell.h>
#include <kstandarddirs.h>
#include <ktoggleaction.h>
#include <ktoolinvocation.h>
#include <kurl.h>

#include <QClipboard>
#include <QPixmap>
#include <QSplitter>
#include <QTextDocument>
#include <QTimer>

namespace Akregator {

class MainWidget::EditNodePropertiesVisitor : public TreeNodeVisitor
{
    public:
        EditNodePropertiesVisitor(MainWidget* mainWidget) : m_mainWidget(mainWidget) {}

        virtual bool visitTagNode(TagNode* node)
        {
            TagPropertiesDialog* dlg = new TagPropertiesDialog(m_mainWidget);
            dlg->setTag(node->tag());
            dlg->exec();
            delete dlg;
            return true;
        }

        virtual bool visitFolder(Folder* node)
        {
            m_mainWidget->m_listTabWidget->activeView()->startNodeRenaming(node);
            return true;
        }

        virtual bool visitFeed(Feed* node)
        {
            FeedPropertiesDialog *dlg = new FeedPropertiesDialog( m_mainWidget, "edit_feed" );
            dlg->setFeed(node);
            dlg->exec();
            delete dlg;
            return true;
        }
    private:

        MainWidget* m_mainWidget;
};

class MainWidget::DeleteNodeVisitor : public TreeNodeVisitor
{
    public:
        DeleteNodeVisitor(MainWidget* view) : m_mainWidget(view) {}

        virtual bool visitTagNode(TagNode* node)
        {
            QString msg = i18n("<qt>Are you sure you want to delete tag <b>%1</b>? The tag will be removed from all articles.</qt>", node->title());
            if (KMessageBox::warningContinueCancel(0, msg, i18n("Delete Tag"), KStdGuiItem::del()) == KMessageBox::Continue)
            {
                Tag tag = node->tag();
                QList<Article> articles = m_mainWidget->m_feedList->rootNode()->articles(tag.id());
                node->setNotificationMode(false);
                for (QList<Article>::Iterator it = articles.begin(); it != articles.end(); ++it)
                    (*it).removeTag(tag.id());
                node->setNotificationMode(true);
                Kernel::self()->tagSet()->remove(tag);
                m_mainWidget->m_listTabWidget->activeView()->setFocus();
            }
            return true;
        }

        virtual bool visitFolder(Folder* node)
        {
            QString msg;
            if (node->title().isEmpty())
                msg = i18n("<qt>Are you sure you want to delete this folder and its feeds and subfolders?</qt>");
            else
                msg = i18n("<qt>Are you sure you want to delete folder <b>%1</b> and its feeds and subfolders?</qt>", node->title());

            if (KMessageBox::warningContinueCancel(0, msg, i18n("Delete Folder"), KStdGuiItem::del()) == KMessageBox::Continue)
            {
                delete node;
                m_mainWidget->m_listTabWidget->activeView()->setFocus();
            }
            return true;
        }

        virtual bool visitFeed(Feed* node)
        {
            QString msg;
            if (node->title().isEmpty())
                msg = i18n("<qt>Are you sure you want to delete this feed?</qt>");
            else
                msg = i18n("<qt>Are you sure you want to delete feed <b>%1</b>?</qt>", node->title());

            if (KMessageBox::warningContinueCancel(0, msg, i18n("Delete Feed"), KStdGuiItem::del()) == KMessageBox::Continue)
            {
                delete node;
                m_mainWidget->m_listTabWidget->activeView()->setFocus();
            }
            return true;
        }
    private:

        MainWidget* m_mainWidget;
};


MainWidget::~MainWidget()
{
    // if m_shuttingDown is false, slotOnShutdown was not called. That
     // means that not the whole app is shutdown, only the part. So it
    // should be no risk to do the cleanups now
    if (!m_shuttingDown)
    {
        kDebug() << "MainWidget::~MainWidget(): slotOnShutdown() wasn't called. Calling it now." << endl;
        slotOnShutdown();
    }
    kDebug() << "MainWidget::~MainWidget(): leaving" << endl;
}

MainWidget::MainWidget( Part *part, QWidget *parent, ActionManagerImpl* actionManager, const char *name)
 : QWidget(parent), m_viewMode(NormalView), m_actionManager(actionManager)
{
    setObjectName(name);
    m_editNodePropertiesVisitor = new EditNodePropertiesVisitor(this);
    m_deleteNodeVisitor = new DeleteNodeVisitor(this);
    m_keepFlagIcon = QPixmap(KStandardDirs::locate("data", "akregator/pics/akregator_flag.png"));

    m_actionManager->initMainWidget(this);
    m_actionManager->initFrameManager(Kernel::self()->frameManager());
    m_part = part;
    m_feedList = new FeedList();
    m_tagNodeList = new TagNodeList(m_feedList, Kernel::self()->tagSet());
    m_shuttingDown = false;
    m_displayingAboutPage = false;
    setFocusPolicy(Qt::StrongFocus);

    QVBoxLayout *lt = new QVBoxLayout( this );
    
    m_horizontalSplitter = new QSplitter(Qt::Horizontal, this);

    m_horizontalSplitter->setOpaqueResize(true);
    lt->addWidget(m_horizontalSplitter);

    connect(Kernel::self()->fetchQueue(), SIGNAL(fetched(Feed*)),
             this, SLOT(slotFeedFetched(Feed*)));
    connect(Kernel::self()->fetchQueue(), SIGNAL(signalStarted()),
             this, SLOT(slotFetchingStarted()));
    connect(Kernel::self()->fetchQueue(), SIGNAL(signalStopped()),
             this, SLOT(slotFetchingStopped()));

    connect(Kernel::self()->tagSet(), SIGNAL(signalTagAdded(const Tag&)),
            this, SLOT(slotTagCreated(const Tag&)));
    connect(Kernel::self()->tagSet(), SIGNAL(signalTagRemoved(const Tag&)),
            this, SLOT(slotTagRemoved(const Tag&)));

    m_listTabWidget = new ListTabWidget(m_horizontalSplitter);
    m_actionManager->initListTabWidget(m_listTabWidget);

    connect(m_listTabWidget, SIGNAL(signalNodeSelected(TreeNode*)),
            this, SLOT(slotNodeSelected(TreeNode*)));


    m_feedListView = new NodeListView( this, "feedtree" );
    m_listTabWidget->addView(m_feedListView, i18n("Feeds"), KGlobal::iconLoader()->loadIcon("folder", K3Icon::Small));

    connect(m_feedListView, SIGNAL(signalContextMenu(K3ListView*, TreeNode*, const QPoint&)),
            this, SLOT(slotFeedTreeContextMenu(K3ListView*, TreeNode*, const QPoint&)));

    connect(m_feedListView, SIGNAL(signalDropped (KUrl::List &, TreeNode*,
            Folder*)),
            this, SLOT(slotFeedURLDropped (KUrl::List &,
            TreeNode*, Folder*)));

    m_tagNodeListView = new NodeListView(this);
    m_listTabWidget->addView(m_tagNodeListView, i18n("Tags"), KGlobal::iconLoader()->loadIcon("rss_tag", K3Icon::Small));

    connect(m_tagNodeListView, SIGNAL(signalContextMenu(K3ListView*, TreeNode*, const QPoint&)), 
            this, SLOT(slotFeedTreeContextMenu(K3ListView*, TreeNode*, const QPoint&)));
    
    ProgressManager::self()->setFeedList(m_feedList);

    m_tabWidget = new TabWidget(m_horizontalSplitter);
    m_actionManager->initTabWidget(m_tabWidget);

    connect( m_part, SIGNAL(signalSettingsChanged()), 
             m_tabWidget, SLOT(slotSettingsChanged()));

    connect( m_tabWidget, SIGNAL(signalCurrentFrameChanged(int)),     
             Kernel::self()->frameManager(), SLOT(slotChangeFrame(int)));
    
    connect( m_tabWidget, SIGNAL(signalRemoveFrameRequest(int)),
             Kernel::self()->frameManager(), SLOT(slotRemoveFrame(int)));
    
    connect( m_tabWidget, SIGNAL(signalOpenURLRequest(OpenURLRequest&)),
             Kernel::self()->frameManager(), SLOT(slotOpenURLRequest(OpenURLRequest&)));

    connect( Kernel::self()->frameManager(), SIGNAL(signalFrameAdded(Frame*)),
             m_tabWidget, SLOT(slotAddFrame(Frame*)));
    
    connect( Kernel::self()->frameManager(), SIGNAL(signalFrameRemoved(int)),
             m_tabWidget, SLOT(slotRemoveFrame(int)));
    
    connect( Kernel::self()->frameManager(), SIGNAL(signalCurrentFrameChanged(Frame*)), 
             this, SLOT( slotFrameChanged(Frame *) ) );

    connect( Kernel::self()->frameManager(), SIGNAL(signalRequestNewFrame(int&)),
             this, SLOT( slotRequestNewFrame(int&) ) );

    m_tabWidget->setWhatsThis( i18n("You can view multiple articles in several open tabs."));

    m_mainTab = new QWidget(this);
    m_mainTab->setObjectName("Article Tab");

    QVBoxLayout *mainTabLayout = new QVBoxLayout( m_mainTab);
    mainTabLayout->setObjectName("mainTabLayout");

    m_mainTab->setWhatsThis( i18n("Articles list."));

    m_searchBar = new SearchBar(m_mainTab);

    if ( !Settings::showQuickFilter() )
        m_searchBar->hide();

    mainTabLayout->addWidget(m_searchBar);

    m_articleSplitter = new QSplitter(Qt::Vertical, m_mainTab);
    m_articleSplitter->setObjectName("panner2");

    m_articleList = new ArticleListView( m_articleSplitter, "articles" );

    m_actionManager->initArticleListView(m_articleList);

    connect( m_articleList, SIGNAL(signalMouseButtonPressed(int, const Article&, const QPoint &, int)),
             this, SLOT(slotMouseButtonPressed(int, const Article&, const QPoint &, int)));

    // use selectionChanged instead of clicked
    connect( m_articleList, SIGNAL(signalArticleChosen(const Article&)),
             this, SLOT( slotArticleSelected(const Article&)) );
    connect( m_articleList, SIGNAL(signalDoubleClicked(const Article&, const QPoint&, int)),
             this, SLOT( slotOpenArticleExternal(const Article&, const QPoint&, int)) );

    m_articleViewer = new ArticleViewer(m_articleSplitter, "article_viewer");
    m_articleViewer->setSafeMode();  // disable JS, Java, etc...

    m_actionManager->initArticleViewer(m_articleViewer);

    connect(m_searchBar, SIGNAL(signalSearch(const Akregator::Filters::ArticleMatcher&, const Akregator::Filters::ArticleMatcher&)), m_articleList, SLOT(slotSetFilter(const Akregator::Filters::ArticleMatcher&, const Akregator::Filters::ArticleMatcher&)));

    connect(m_searchBar, SIGNAL(signalSearch(const Akregator::Filters::ArticleMatcher&, const Akregator::Filters::ArticleMatcher&)),
            m_articleViewer, SLOT(slotSetFilter(const Akregator::Filters::ArticleMatcher&, const Akregator::Filters::ArticleMatcher&)));

    connect( m_articleViewer, SIGNAL(urlClicked(const KUrl&, bool)),
             this, SLOT(slotOpenTab(const KUrl&, bool)) );

    connect( m_articleViewer->browserExtension(), SIGNAL(mouseOverInfo(const KFileItem *)),
             this, SLOT(slotMouseOverInfo(const KFileItem *)) );

    connect( m_part, SIGNAL(signalSettingsChanged()), 
             m_articleViewer, SLOT(slotPaletteOrFontChanged()));
    m_articleViewer->widget()->setWhatsThis( i18n("Browsing area."));
    mainTabLayout->addWidget( m_articleSplitter );

    m_mainFrame = new MainFrame(this, m_part, m_mainTab, i18n("Articles"));

    Kernel::self()->frameManager()->slotAddFrame(m_mainFrame);

    m_horizontalSplitter->setSizes( Settings::splitter1Sizes() );
    m_articleSplitter->setSizes( Settings::splitter2Sizes() );

    KConfig *conf = Settings::self()->config();
    conf->setGroup("General");
    if(!conf->readEntry("Disable Introduction", false))
    {
        m_articleList->hide();
        m_searchBar->hide();
        m_articleViewer->displayAboutPage();
        m_mainFrame->slotSetTitle(i18n("About"));
        m_displayingAboutPage = true;
    }

    m_fetchTimer = new QTimer(this);
    connect( m_fetchTimer, SIGNAL(timeout()), 
             this, SLOT(slotDoIntervalFetches()) );
    m_fetchTimer->start(1000*60);

    // delete expired articles once per hour
    m_expiryTimer = new QTimer(this);
    connect(m_expiryTimer, SIGNAL(timeout()),
            this, SLOT(slotDeleteExpiredArticles()) );
    m_expiryTimer->start(3600*1000);

    m_markReadTimer = new QTimer(this);
    m_markReadTimer->setSingleShot(true);
    connect(m_markReadTimer, SIGNAL(timeout()), this, SLOT(slotSetCurrentArticleReadDelayed()) );

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

    QTimer::singleShot(1000, this, SLOT(slotDeleteExpiredArticles()) );
    QTimer::singleShot(0, this, SLOT(delayedInit()));
}

void MainWidget::delayedInit()
{
    // HACK, FIXME:
    // for some reason, m_part->factory() is NULL at startup of kontact,
    // and thus the article viewer GUI can't be merged when creating the view.
    // Even the delayed init didn't help. Well, we retry every half a second until
    // it works. This is kind of creative, but a dirty hack nevertheless.
    if ( !m_part->mergePart(m_articleViewer) )
        QTimer::singleShot(500, this, SLOT(delayedInit()));
}

void MainWidget::slotOnShutdown()
{
    m_shuttingDown = true; // prevents slotFrameChanged from crashing

    m_articleList->slotShowNode(0);
    m_articleViewer->slotShowNode(0);

    Kernel::self()->fetchQueue()->slotAbort();

    m_feedListView->setNodeList(0);
    ProgressManager::self()->setFeedList(0);

    delete m_feedList;
    delete m_tagNodeList;

    // close all pageviewers in a controlled way
    // fixes bug 91660, at least when no part loading data
    m_tabWidget->setCurrentIndex(m_tabWidget->count()-1); // select last page
    while (m_tabWidget->count() > 1) // remove frames until only the main frame remains
        m_tabWidget->slotRemoveCurrentFrame();

    delete m_mainTab;
    delete m_mainFrame;
    delete m_editNodePropertiesVisitor;
    delete m_deleteNodeVisitor;
}

void MainWidget::saveSettings()
{
    Settings::setSplitter1Sizes( m_horizontalSplitter->sizes() );
    Settings::setSplitter2Sizes( m_articleSplitter->sizes() );
    Settings::setViewMode( m_viewMode );
    Settings::writeConfig();
}

void MainWidget::slotOpenTab(const KUrl& url, bool background)
{
    OpenURLRequest req(url);
    req.setOptions(OpenURLRequest::NewTab);
    req.setOpenInBackground(background);
    Kernel::self()->frameManager()->slotOpenURLRequest(req);
    /*
    BrowserFrame* frame = new BrowserFrame(m_tabWidget);
    
    connect( m_part, SIGNAL(signalSettingsChanged()), frame, SLOT(slotPaletteOrFontChanged()));

    Kernel::self()->frameManager()->slotAddFrame(frame);
    
    if(!background)
        m_tabWidget->setCurrentIndex( m_tabWidget->indexOf(frame) );
    else
        setFocus();

    OpenURLRequest request(url);
    request.setFrameId(frame->id());
    Kernel::self()->frameManager()->slotOpenURLRequest(request);
    */
}

void MainWidget::slotRequestNewFrame(int& frameId)
{
    BrowserFrame* frame = new BrowserFrame(m_tabWidget);
    
    connect( m_part, SIGNAL(signalSettingsChanged()), frame, SLOT(slotPaletteOrFontChanged()));
    
    Kernel::self()->frameManager()->slotAddFrame(frame);
    
    frameId = frame->id();
}
        
void MainWidget::setTabIcon(const QPixmap& icon)
{
    Viewer* s = dynamic_cast<Viewer*>(sender());
    if (s)
    {
        m_tabWidget->setTabIcon(m_tabWidget->indexOf(s->widget()), icon);
    }
}

void MainWidget::sendArticle(bool attach)
{
    // FIXME: you have to open article to tab to be able to send...
    
    Frame* frame = Kernel::self()->frameManager()->currentFrame();
    
    if (!frame)
        return;

    QByteArray text = frame->url().prettyUrl().toLatin1();

    if(text.isEmpty() || text.isNull())
        return;

    QString title = frame->title();

    if(attach) 
    {
        KToolInvocation::invokeMailer(QString(),
                           QString(),
                           QString(),
                           title,
                           text,
                           QString(),
                           QStringList(),
                           text);
    }
    else 
    {
        KToolInvocation::invokeMailer(QString(),
                           QString(),
                           QString(),
                           QString(),
                           title,
                           QString(),
                           QStringList(),
                           text);
    }
}

bool MainWidget::importFeeds(const QDomDocument& doc)
{
    FeedList* feedList = new FeedList();
    bool parsed = feedList->readFromXML(doc);

    // FIXME: parsing error, print some message
    if (!parsed)
    {
        delete feedList;
        return false;
    }
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

bool MainWidget::loadFeeds(const QDomDocument& doc, Folder* parent)
{
    FeedList* feedList = new FeedList();
    bool parsed = feedList->readFromXML(doc);

    // parsing went wrong
    if (!parsed)
    {
        delete feedList;
        return false;
    }
    m_feedListView->setUpdatesEnabled(false);
    m_tagNodeListView->setUpdatesEnabled(false);
    if (!parent)
    {
        TagSet* tagSet = Kernel::self()->tagSet();

        Kernel::self()->setFeedList(feedList);
        ProgressManager::self()->setFeedList(feedList);
        disconnectFromFeedList(m_feedList);
        delete m_feedList;
        delete m_tagNodeList;
        m_feedList = feedList;
        connectToFeedList(m_feedList);

        m_tagNodeList = new TagNodeList(m_feedList, tagSet);
        m_feedListView->setNodeList(m_feedList);
        m_tagNodeListView->setNodeList(m_tagNodeList);

        QStringList tagIDs = m_feedList->rootNode()->tags();
        QStringList::ConstIterator end = tagIDs.end();
        for (QStringList::ConstIterator it = tagIDs.begin(); it != end; ++it)
        {
            kDebug() << *it << endl;
            // create a tag for every tag ID in the archive that is not part of the tagset
            // this is a fallback in case the tagset was corrupted,
            // so the tagging information from archive does not get lost.
            if (!tagSet->containsID(*it))
            {
                Tag tag(*it, *it);
                tagSet->insert(tag);
            }
        }
    }
    else
        m_feedList->append(feedList, parent);

    m_feedListView->setUpdatesEnabled(true);
    m_feedListView->triggerUpdate();
    m_tagNodeListView->setUpdatesEnabled(true);
    m_tagNodeListView->triggerUpdate();
    return true;
}

void MainWidget::slotDeleteExpiredArticles()
{
    TreeNode* rootNode = m_feedList->rootNode();
    if (rootNode)
        rootNode->slotDeleteExpiredArticles();
}

QDomDocument MainWidget::feedListToOPML()
{
    return m_feedList->toXML();
}

void MainWidget::addFeedToGroup(const QString& url, const QString& groupName)
{

    // Locate the group.
    TreeNode* node = m_feedListView->findNodeByTitle(groupName);

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

void MainWidget::slotNormalView()
{
    if (m_viewMode == NormalView)
    return;

    if (m_viewMode == CombinedView)
    {
        m_articleList->slotShowNode(m_listTabWidget->activeView()->selectedNode());
        m_articleList->show();

        Article article = m_articleList->currentArticle();

        if (!article.isNull())
            m_articleViewer->slotShowArticle(article);
        else
            m_articleViewer->slotShowSummary(m_listTabWidget->activeView()->selectedNode());
    }

    m_articleSplitter->setOrientation(Qt::Vertical);
    m_viewMode = NormalView;

    Settings::setViewMode( m_viewMode );
}

void MainWidget::slotWidescreenView()
{
    if (m_viewMode == WidescreenView)
    return;

    if (m_viewMode == CombinedView)
    {
        m_articleList->slotShowNode(m_listTabWidget->activeView()->selectedNode());
        m_articleList->show();

        Article article = m_articleList->currentArticle();

        if (!article.isNull())
            m_articleViewer->slotShowArticle(article);
        else
            m_articleViewer->slotShowSummary(m_listTabWidget->activeView()->selectedNode());
    }

    m_articleSplitter->setOrientation(Qt::Horizontal);
    m_viewMode = WidescreenView;

    Settings::setViewMode( m_viewMode );
}

void MainWidget::slotCombinedView()
{
    if (m_viewMode == CombinedView)
        return;

    m_articleList->slotClear();
    m_articleList->hide();
    m_viewMode = CombinedView;

    slotNodeSelected(m_listTabWidget->activeView()->selectedNode());
    Settings::setViewMode( m_viewMode );
}

void MainWidget::slotFrameChanged(Frame* frame)
{
    if (m_shuttingDown)
        return;
    
    if (frame)
        frame->setFocus();
}

void MainWidget::slotFeedTreeContextMenu(K3ListView*, TreeNode* /*node*/, const QPoint& /*p*/)
{
    m_tabWidget->setCurrentIndex( m_tabWidget->indexOf(m_mainTab) );
}

void MainWidget::slotMoveCurrentNodeUp()
{
    TreeNode* current = m_listTabWidget->activeView()->selectedNode();
    if (!current)
        return;
    TreeNode* prev = current->prevSibling();
    Folder* parent = current->parent();

    if (!prev || !parent)
        return;

    parent->removeChild(prev);
    parent->insertChild(prev, current);
    m_listTabWidget->activeView()->ensureNodeVisible(current);
}

void MainWidget::slotMoveCurrentNodeDown()
{
    TreeNode* current = m_listTabWidget->activeView()->selectedNode();
    if (!current)
        return;
    TreeNode* next = current->nextSibling();
    Folder* parent = current->parent();

    if (!next || !parent)
        return;

    parent->removeChild(current);
    parent->insertChild(current, next);
    m_listTabWidget->activeView()->ensureNodeVisible(current);
}

void MainWidget::slotMoveCurrentNodeLeft()
{
    TreeNode* current = m_listTabWidget->activeView()->selectedNode();
    if (!current || !current->parent() || !current->parent()->parent())
        return;

    Folder* parent = current->parent();
    Folder* grandparent = current->parent()->parent();

    parent->removeChild(current);
    grandparent->insertChild(current, parent);
    m_listTabWidget->activeView()->ensureNodeVisible(current);
}

void MainWidget::slotMoveCurrentNodeRight()
{
    TreeNode* current = m_listTabWidget->activeView()->selectedNode();
    if (!current || !current->parent())
        return;
    TreeNode* prev = current->prevSibling();

    if ( prev && prev->isGroup() )
    {
        Folder* fg = static_cast<Folder*>(prev);
        current->parent()->removeChild(current);
        fg->appendChild(current);
        m_listTabWidget->activeView()->ensureNodeVisible(current);
    }
}

void MainWidget::slotNodeSelected(TreeNode* node)
{
    m_markReadTimer->stop();

    if (node)
    {
        kDebug() << "node selected: " << node->title() << endl;
        kDebug() << "unread: " << node->unread() << endl;
        kDebug() << "total: " << node->totalCount() << endl;
    }

    if (m_displayingAboutPage)
    {
        m_mainFrame->slotSetTitle(i18n("Articles"));
        if (m_viewMode != CombinedView)
            m_articleList->show();
        if (Settings::showQuickFilter())
            m_searchBar->show();
        m_displayingAboutPage = false;
    }

    m_tabWidget->setCurrentIndex( m_tabWidget->indexOf(m_mainTab) );

    m_searchBar->slotClearSearch();

    if (m_viewMode == CombinedView)
        m_articleViewer->slotShowNode(node);
    else
    {
        m_articleList->slotShowNode(node);
        m_articleViewer->slotShowSummary(node);
    }

    m_actionManager->slotNodeSelected(node);

    updateTagActions();
}


void MainWidget::slotFeedAdd()
{
    Folder* group = 0;
    if (!m_feedListView->selectedNode())
        group = m_feedList->rootNode(); // all feeds
    else
    {
        //TODO: tag nodes need rework
        if ( m_feedListView->selectedNode()->isGroup())
            group = static_cast<Folder*>(m_feedListView->selectedNode());
        else
            group= m_feedListView->selectedNode()->parent();

    }

    TreeNode* lastChild = group->children().last();

    addFeed(QString::null, lastChild, group, false);
}

void MainWidget::addFeed(const QString& url, TreeNode *after, Folder* parent, bool autoExec)
{

    AddFeedDialog *afd = new AddFeedDialog( 0, "add_feed" );

    afd->setURL(KUrl::fromPercentEncoding( url.toLatin1() ));

    if (autoExec)
        afd->accept();
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

    m_feedListView->ensureNodeVisible(feed);


    delete dlg;
}

void MainWidget::slotFeedAddGroup()
{
    TreeNode* node = m_feedListView->selectedNode();
    TreeNode* after = 0;

    if (!node)
        node = m_feedListView->rootNode();

    // if a feed is selected, add group next to it
    //TODO: tag nodes need rework
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

        m_feedListView->ensureNodeVisible(newGroup);
    }
}

void MainWidget::slotFeedRemove()
{
    TreeNode* selectedNode = m_listTabWidget->activeView()->selectedNode();

    // don't delete root element! (safety valve)
    if (!selectedNode || selectedNode == m_feedList->rootNode())
        return;

    m_deleteNodeVisitor->visit(selectedNode);
}

void MainWidget::slotFeedModify()
{
    TreeNode* node = m_listTabWidget->activeView()->selectedNode();
    if (node)
        m_editNodePropertiesVisitor->visit(node);

}

void MainWidget::slotNextUnreadArticle()
{
    if (m_viewMode == CombinedView)
        m_listTabWidget->activeView()->slotNextUnreadFeed();
    
    TreeNode* sel = m_listTabWidget->activeView()->selectedNode();
    if (sel && sel->unread() > 0)
        m_articleList->slotNextUnreadArticle();
    else
        m_listTabWidget->activeView()->slotNextUnreadFeed();
}

void MainWidget::slotPrevUnreadArticle()
{
    if (m_viewMode == CombinedView)
        m_listTabWidget->activeView()->slotPrevUnreadFeed();
    
    TreeNode* sel = m_listTabWidget->activeView()->selectedNode();
    if (sel && sel->unread() > 0)
        m_articleList->slotPreviousUnreadArticle();
    else
        m_listTabWidget->activeView()->slotPrevUnreadFeed();
}

void MainWidget::slotMarkAllFeedsRead()
{
    m_feedList->rootNode()->slotMarkAllArticlesAsRead();
}

void MainWidget::slotMarkAllRead()
{
    if(!m_listTabWidget->activeView()->selectedNode()) return;
    m_listTabWidget->activeView()->selectedNode()->slotMarkAllArticlesAsRead();
}


void MainWidget::openURLDefault(const KUrl& url)
{
    OpenURLRequest req(url);
    
    switch (Settings::lMBBehaviour())
    {
        case Settings::EnumLMBBehaviour::OpenInExternalBrowser:
            req.setOptions(OpenURLRequest::ExternalBrowser);
            break;
        case Settings::EnumLMBBehaviour::OpenInBackground:
            req.setOptions(OpenURLRequest::NewTab);
            req.setOpenInBackground(true);
            break;
        default:
            req.setOptions(OpenURLRequest::NewTab);
            req.setOpenInBackground(false);
    }
    
    Kernel::self()->frameManager()->slotOpenURLRequest(req);
}

void MainWidget::slotOpenHomepage()
{
    Feed* feed = dynamic_cast<Feed *>(m_listTabWidget->activeView()->selectedNode());

    if (feed)
        openURLDefault(feed->htmlUrl());
}

void MainWidget::slotSetTotalUnread()
{
    emit signalUnreadCountChanged( m_feedList->rootNode()->unread() );
}

void MainWidget::slotDoIntervalFetches()
{
    m_feedList->rootNode()->slotAddToFetchQueue(Kernel::self()->fetchQueue(), true);
}

void MainWidget::slotFetchCurrentFeed()
{
    if ( !m_listTabWidget->activeView()->selectedNode() )
        return;
    m_listTabWidget->activeView()->selectedNode()->slotAddToFetchQueue(Kernel::self()->fetchQueue());
}

void MainWidget::slotFetchAllFeeds()
{
    m_feedList->rootNode()->slotAddToFetchQueue(Kernel::self()->fetchQueue());
}

void MainWidget::slotFetchingStarted()
{
    m_mainFrame->slotSetState(Frame::Started);
    m_actionManager->action("feed_stop")->setEnabled(true);
    m_mainFrame->slotSetStatusText(i18n("Fetching Feeds..."));
}

void MainWidget::slotFetchingStopped()
{
    m_mainFrame->slotSetState(Frame::Completed);
    m_actionManager->action("feed_stop")->setEnabled(false);
    m_mainFrame->slotSetStatusText(QString::null);
}

void MainWidget::slotFeedFetched(Feed *feed)
{
    // iterate through the articles (once again) to do notifications properly
    if (feed->articles().count() > 0)
    {
        QList<Article> articles = feed->articles();
        QList<Article>::ConstIterator it;
        QList<Article>::ConstIterator end = articles.end();
        for (it = articles.begin(); it != end; ++it)
        {
            if ((*it).status()==Article::New && ((*it).feed()->useNotification() || Settings::useNotifications()))
            {
                NotificationManager::self()->slotNotifyArticle(*it);
            }
        }
    }
}

void MainWidget::slotMouseButtonPressed(int button, const Article& article, const QPoint &, int)
{
    if (article.isNull() || button != Qt::MidButton)
        return;
    
    KUrl url = article.link();
    
    if (!url.isValid())
        return;
    
    OpenURLRequest req(url);
    
    switch (Settings::mMBBehaviour())
    {
        case Settings::EnumMMBBehaviour::OpenInExternalBrowser:
            req.setOptions(OpenURLRequest::ExternalBrowser);
            break;
        case Settings::EnumMMBBehaviour::OpenInBackground:
            req.setOptions(OpenURLRequest::NewTab);
            req.setOpenInBackground(true);
            break;
        default:
            req.setOptions(OpenURLRequest::NewTab);
            req.setOpenInBackground(false);
    }
    
    Kernel::self()->frameManager()->slotOpenURLRequest(req);
}

void MainWidget::slotAssignTag(const Tag& tag, bool assign)
{
    kDebug() << (assign ? "assigned" : "removed") << " tag \"" << tag.id() << "\"" << endl;
    QList<Article> selectedArticles = m_articleList->selectedArticles();
    for (QList<Article>::Iterator it = selectedArticles.begin(); it != selectedArticles.end(); ++it)
    {
        if (assign)
            (*it).addTag(tag.id());
        else
            (*it).removeTag(tag.id());
    }
    updateTagActions();
}
/*
void MainWidget::slotRemoveTag(const Tag& tag)
{
    kDebug() << "remove tag \"" << tag.id() << "\" from selected articles" << endl;
    QValueList<Article> selectedArticles = m_articleList->selectedArticles();
    for (QValueList<Article>::Iterator it = selectedArticles.begin(); it != selectedArticles.end(); ++it)
        (*it).removeTag(tag.id());

    updateTagActions();
}
*/
void MainWidget::slotNewTag()
{
    Tag tag(KRandom::randomString(8), "New Tag");
    Kernel::self()->tagSet()->insert(tag);
    TagNode* node = m_tagNodeList->findByTagID(tag.id());
    if (node)
        m_tagNodeListView->startNodeRenaming(node);
}

void MainWidget::slotTagCreated(const Tag& tag)
{
    if (m_tagNodeList && !m_tagNodeList->containsTagId(tag.id()))
    {
        TagNode* tagNode = new TagNode(tag, m_feedList->rootNode());
        m_tagNodeList->rootNode()->appendChild(tagNode);
    }
}

void MainWidget::slotTagRemoved(const Tag& /*tag*/)
{
}

void MainWidget::slotArticleSelected(const Article& article)
{
    if (m_viewMode == CombinedView)
        return;

    m_markReadTimer->stop();

    Feed *feed = article.feed();
    if (!feed)
        return;

    Article a(article);
    if (a.status() != Article::Read)
    {
        int delay;

        if ( Settings::useMarkReadDelay() )
        {
            delay = Settings::markReadDelay();

            if (delay > 0)
                m_markReadTimer->start( delay*1000 );
            else
                a.setStatus(Article::Read);
        }
    }

    KToggleAction* maai = static_cast<KToggleAction*>(m_actionManager->action("article_set_status_important"));
    maai->setChecked(a.keep());

    kDebug() << "selected: " << a.guid() << endl;

    updateTagActions();

    m_articleViewer->slotShowArticle(a);
}

void MainWidget::slotOpenArticleExternal(const Article& article, const QPoint&, int)
{
    if (!article.isNull() && article.link().isValid())
    {
        OpenURLRequest req(article.link());
        req.setOptions(OpenURLRequest::ExternalBrowser);
        Kernel::self()->frameManager()->slotOpenURLRequest(req);
    }
}


void MainWidget::slotOpenCurrentArticle()
{
    openArticleLink(m_articleList->currentArticle(), false);
}

void MainWidget::openArticleLink(const Article& article, bool background)
{
    if (article.isNull())
        return;

    KUrl url = article.link(); 
    
    if (url.isValid())
    {
        OpenURLRequest req(url);
        req.setOpenInBackground(background);
        Kernel::self()->frameManager()->slotOpenURLRequest(req);
    }
}

void MainWidget::slotOpenCurrentArticleExternal()
{
    slotOpenArticleExternal(m_articleList->currentArticle(), QPoint(), 0);
}

void MainWidget::slotOpenCurrentArticleBackgroundTab()
{
    openArticleLink(m_articleList->currentArticle(), true);
}

void MainWidget::slotCopyLinkAddress()
{
    Article article = m_articleList->currentArticle();

    if(article.isNull())
       return;

    QString link;
    if (article.link().isValid())
    {
        link = article.link().url();
        QClipboard *cb = QApplication::clipboard();
        cb->setText(link, QClipboard::Clipboard);
        cb->setText(link, QClipboard::Selection);
    }
}

void MainWidget::slotFeedURLDropped(KUrl::List &urls, TreeNode* after, Folder* parent)
{
    KUrl::List::iterator it;
    for ( it = urls.begin(); it != urls.end(); ++it )
    {
        addFeed((*it).prettyUrl(), after, parent, false);
    }
}

void MainWidget::slotToggleShowQuickFilter()
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

void MainWidget::slotArticleDelete()
{

    if ( m_viewMode == CombinedView )
        return;

    QList<Article> articles = m_articleList->selectedArticles();

    QString msg;
    switch (articles.count())
    {
        case 0:
            return;
        case 1:
            msg = i18n("<qt>Are you sure you want to delete article <b>%1</b>?</qt>", Qt::escape(articles.first().title()));
            break;
        default:
            msg = i18n("<qt>Are you sure you want to delete the %1 selected articles?</qt>", articles.count());
    }

    if (KMessageBox::warningContinueCancel(0, msg, i18n("Delete Article"), KStdGuiItem::del()) == KMessageBox::Continue)
    {
        if (m_listTabWidget->activeView()->selectedNode())
            m_listTabWidget->activeView()->selectedNode()->setNotificationMode(false);

        QList<Feed*> feeds;
        for (QList<Article>::Iterator it = articles.begin(); it != articles.end(); ++it)
        {
            Feed* feed = (*it).feed();
            if (!feeds.contains(feed))
                feeds.append(feed);
            feed->setNotificationMode(false);
            (*it).setDeleted();
        }

        for (QList<Feed*>::Iterator it = feeds.begin(); it != feeds.end(); ++it)
        {
            (*it)->setNotificationMode(true);
        }

        if (m_listTabWidget->activeView()->selectedNode())
            m_listTabWidget->activeView()->selectedNode()->setNotificationMode(true);
    }
}


void MainWidget::slotArticleToggleKeepFlag(bool /*enabled*/)
{
    QList<Article> articles = m_articleList->selectedArticles();

    if (articles.isEmpty())
        return;

    bool allFlagsSet = true;
    for (QList<Article>::Iterator it = articles.begin(); allFlagsSet && it != articles.end(); ++it)
        if (!(*it).keep())
            allFlagsSet = false;

    for (QList<Article>::Iterator it = articles.begin(); it != articles.end(); ++it)
        (*it).setKeep(!allFlagsSet);
}

void MainWidget::slotSetSelectedArticleRead()
{
    QList<Article> articles = m_articleList->selectedArticles();

    if (articles.isEmpty())
        return;

    for (QList<Article>::Iterator it = articles.begin(); it != articles.end(); ++it)
        (*it).setStatus(Article::Read);
}

void MainWidget::slotTextToSpeechRequest()
{
    
    if (Kernel::self()->frameManager()->currentFrame() == m_mainFrame)
    {
        if (m_viewMode != CombinedView)
        {
            // in non-combined view, read selected articles
#warning "kde4:readd speechclient";
            //SpeechClient::self()->slotSpeak(m_articleList->selectedArticles());
            // TODO: if article viewer has a selection, read only the selected text?
        }
        else
        {
            if (m_listTabWidget->activeView()->selectedNode())
            {
                //TODO: read articles in current node, respecting quick filter!
            }
        }
    }
    else
    {
        // TODO: read selected page viewer
    }
}

void MainWidget::slotSetSelectedArticleUnread()
{
    QList<Article> articles = m_articleList->selectedArticles();

    if (articles.isEmpty())
        return;

    for (QList<Article>::Iterator it = articles.begin(); it != articles.end(); ++it)
        (*it).setStatus(Article::Unread);
}

void MainWidget::slotSetSelectedArticleNew()
{
    QList<Article> articles = m_articleList->selectedArticles();

    if (articles.isEmpty())
        return;

    for (QList<Article>::Iterator it = articles.begin(); it != articles.end(); ++it)
        (*it).setStatus(Article::New);
}

void MainWidget::slotSetCurrentArticleReadDelayed()
{
    Article article = m_articleList->currentArticle();

    if (article.isNull())
        return;

    article.setStatus(Article::Read);
}

void MainWidget::slotMouseOverInfo(const KFileItem *kifi)
{
    if (kifi)
    {
        KFileItem *k=(KFileItem*)kifi;
        m_mainFrame->slotSetStatusText(k->url().prettyUrl());//getStatusBarInfo());
    }
    else
    {
        m_mainFrame->slotSetStatusText(QString::null);
    }
}

void MainWidget::readProperties(KConfig* config)
{
    // read filter settings
    m_searchBar->slotSetText(config->readEntry("searchLine"));
    m_searchBar->slotSetStatus(config->readEntry("searchCombo").toInt());
}

void MainWidget::saveProperties(KConfig* config)
{
    // save filter settings
    config->writeEntry("searchLine", m_searchBar->text());
    config->writeEntry("searchCombo", m_searchBar->status());
}

void MainWidget::connectToFeedList(FeedList* feedList)
{
    connect(feedList->rootNode(), SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotSetTotalUnread()));
    slotSetTotalUnread();
}

void MainWidget::disconnectFromFeedList(FeedList* feedList)
{
    disconnect(feedList->rootNode(), SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotSetTotalUnread()));
}

void MainWidget::updateTagActions()
{
    QStringList tags;

    QList<Article> selectedArticles = m_articleList->selectedArticles();

    for (QList<Article>::ConstIterator it = selectedArticles.begin(); it != selectedArticles.end(); ++it)
    {
        QStringList atags = (*it).tags();
        for (QStringList::ConstIterator it2 = atags.begin(); it2 != atags.end(); ++it2)
        {
            if (!tags.contains(*it2))
                tags += *it2;
        }
    }
    m_actionManager->slotUpdateTagActions(!selectedArticles.isEmpty(), tags);
}

} // namespace Akregator

#include "mainwidget.moc"
