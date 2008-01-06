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

#include "mainwidget.h"
#include "actionmanagerimpl.h"
#include "addfeeddialog.h"
#include "articlelistview.h"
#include "articleviewer.h"
#include "articlejobs.h"
#include "akregatorconfig.h"
#include "akregator_part.h"
#include "browserframe.h"
#include "feed.h"
#include "feedlist.h"
#include "fetchqueue.h"
#include "folder.h"
#include "framemanager.h"
#include "kernel.h"
#include "notificationmanager.h"
#include "openurlrequest.h"
#include "propertiesdialog.h"
#include "progressmanager.h"
#include "searchbar.h"
#include "selectioncontroller.h"
//#include "speechclient.h"
#include "subscriptionlistmodel.h"
#include "subscriptionlistview.h"
#include "tabwidget.h"
#include "treenode.h"
#include "treenodevisitor.h"
#include "types.h"

#include <solid/networking.h>

#include <kaction.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <kfileitem.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
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


class Akregator::MainWidget::EditNodePropertiesVisitor : public TreeNodeVisitor
{
    public:
        EditNodePropertiesVisitor(MainWidget* mainWidget) : m_mainWidget(mainWidget) {}

        virtual bool visitFolder(Folder* node)
        {
            m_mainWidget->m_feedListView->startNodeRenaming(node);
            return true;
        }

        virtual bool visitFeed(Feed* node)
        {
            QPointer<FeedPropertiesDialog> dlg = new FeedPropertiesDialog( m_mainWidget, "edit_feed" );
            dlg->setFeed(node);
            dlg->exec();
            delete dlg;
            return true;
        }
    private:

        MainWidget* m_mainWidget;
};

class Akregator::MainWidget::DeleteNodeVisitor : public TreeNodeVisitor
{
    public:
        DeleteNodeVisitor(MainWidget* view) : m_mainWidget(view) {}

        virtual bool visitFolder(Folder* node)
        {
            QString msg;
            if (node->title().isEmpty())
                msg = i18n("<qt>Are you sure you want to delete this folder and its feeds and subfolders?</qt>");
            else
                msg = i18n("<qt>Are you sure you want to delete folder <b>%1</b> and its feeds and subfolders?</qt>", node->title());

            if (KMessageBox::warningContinueCancel(0, msg, i18n("Delete Folder"), KStandardGuiItem::del()) == KMessageBox::Continue)
            {
                delete node;
                m_mainWidget->m_feedListView->setFocus();
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

            if (KMessageBox::warningContinueCancel(0, msg, i18n("Delete Feed"), KStandardGuiItem::del()) == KMessageBox::Continue)
            {
                delete node;
                m_mainWidget->m_feedListView->setFocus();
            }
            return true;
        }
    private:

        MainWidget* m_mainWidget;
};


Akregator::MainWidget::~MainWidget()
{
    // if m_shuttingDown is false, slotOnShutdown was not called. That
     // means that not the whole app is shutdown, only the part. So it
    // should be no risk to do the cleanups now
    if (!m_shuttingDown)
    {
        kDebug() <<"Akregator::MainWidget::~MainWidget(): slotOnShutdown() wasn't called. Calling it now.";
        slotOnShutdown();
    }
    kDebug() <<"Akregator::MainWidget::~MainWidget(): leaving";
}

Akregator::MainWidget::MainWidget( Part *part, QWidget *parent, ActionManagerImpl* actionManager, const char *name)
 : QWidget(parent), m_viewMode(NormalView), m_actionManager(actionManager)
{
    setObjectName(name);
    m_editNodePropertiesVisitor = new EditNodePropertiesVisitor(this);
    m_deleteNodeVisitor = new DeleteNodeVisitor(this);

    m_actionManager->initMainWidget(this);
    m_actionManager->initFrameManager(Kernel::self()->frameManager());
    m_part = part;
    m_feedList = new FeedList( Kernel::self()->storage() );
    m_shuttingDown = false;
    m_displayingAboutPage = false;
    setFocusPolicy(Qt::StrongFocus);

    QVBoxLayout *lt = new QVBoxLayout( this );
    lt->setMargin(0);

    m_horizontalSplitter = new QSplitter(Qt::Horizontal, this);

    m_horizontalSplitter->setOpaqueResize(true);
    lt->addWidget(m_horizontalSplitter);

    connect(Kernel::self()->fetchQueue(), SIGNAL(fetched(Akregator::Feed*)),
             this, SLOT(slotFeedFetched(Akregator::Feed*)));
    connect(Kernel::self()->fetchQueue(), SIGNAL(signalStarted()),
             this, SLOT(slotFetchingStarted()));
    connect(Kernel::self()->fetchQueue(), SIGNAL(signalStopped()),
             this, SLOT(slotFetchingStopped()));

    m_feedListView = new SubscriptionListView( m_horizontalSplitter );
    m_feedListView->setObjectName( "feedtree" );

    m_actionManager->initSubscriptionListView( m_feedListView );

    connect(m_feedListView, SIGNAL(signalContextMenu(K3ListView*, Akregator::TreeNode*, const QPoint&)),
            this, SLOT(slotFeedTreeContextMenu(K3ListView*, Akregator::TreeNode*, const QPoint&)));

    connect(m_feedListView, SIGNAL(signalDropped (KUrl::List &, Akregator::TreeNode*,
            Akregator::Folder*)),
            this, SLOT(slotFeedUrlDropped (KUrl::List &,
            Akregator::TreeNode*, Akregator::Folder*)));

    ProgressManager::self()->setFeedList(m_feedList);

    m_tabWidget = new TabWidget(m_horizontalSplitter);
    m_actionManager->initTabWidget(m_tabWidget);

    connect( m_part, SIGNAL(signalSettingsChanged()),
             m_tabWidget, SLOT(slotSettingsChanged()));

    connect( m_tabWidget, SIGNAL(signalCurrentFrameChanged(int)),
             Kernel::self()->frameManager(), SLOT(slotChangeFrame(int)));

    connect( m_tabWidget, SIGNAL(signalRemoveFrameRequest(int)),
             Kernel::self()->frameManager(), SLOT(slotRemoveFrame(int)));

    connect( m_tabWidget, SIGNAL(signalOpenUrlRequest(Akregator::OpenUrlRequest&)),
             Kernel::self()->frameManager(), SLOT(slotOpenUrlRequest(Akregator::OpenUrlRequest&)));

    connect( Kernel::self()->frameManager(), SIGNAL(signalFrameAdded(Akregator::Frame*)),
             m_tabWidget, SLOT(slotAddFrame(Akregator::Frame*)));

    connect( Kernel::self()->frameManager(), SIGNAL(signalSelectFrame(int)),
             m_tabWidget, SLOT(slotSelectFrame(int)) );

    connect( Kernel::self()->frameManager(), SIGNAL(signalFrameRemoved(int)),
             m_tabWidget, SLOT(slotRemoveFrame(int)));

    connect( Kernel::self()->frameManager(), SIGNAL(signalRequestNewFrame(int&)),
             this, SLOT( slotRequestNewFrame(int&) ) );

    m_tabWidget->setWhatsThis( i18n("You can view multiple articles in several open tabs."));

    m_mainTab = new QWidget(this);
    m_mainTab->setObjectName("Article Tab");

    QVBoxLayout *mainTabLayout = new QVBoxLayout( m_mainTab);
    mainTabLayout->setMargin(0);

    m_mainTab->setWhatsThis( i18n("Articles list."));

    m_searchBar = new SearchBar(m_mainTab);

    if ( !Settings::showQuickFilter() )
        m_searchBar->hide();

    mainTabLayout->addWidget(m_searchBar);

    m_articleSplitter = new QSplitter(Qt::Vertical, m_mainTab);
    m_articleSplitter->setObjectName("panner2");

    m_articleListView = new ArticleListView( m_articleSplitter );

    m_selectionController = new SelectionController( this );
    m_selectionController->setArticleLister( m_articleListView );
    m_selectionController->setFeedSelector( m_feedListView );
    m_selectionController->setFeedList( m_feedList );

    connect(m_searchBar, SIGNAL( signalSearch( std::vector<boost::shared_ptr<const Akregator::Filters::AbstractMatcher> > ) ), m_selectionController, SLOT( setFilters( std::vector<boost::shared_ptr<const Akregator::Filters::AbstractMatcher> > ) ) );

    FolderExpansionHandler* expansionHandler = new FolderExpansionHandler( this );
    connect( m_feedListView, SIGNAL( expanded( QModelIndex ) ), expansionHandler, SLOT( itemExpanded( QModelIndex ) ) );
    connect( m_feedListView, SIGNAL( collapsed( QModelIndex ) ), expansionHandler, SLOT( itemCollapsed( QModelIndex ) ) );

    m_selectionController->setFolderExpansionHandler( expansionHandler );

    connect( m_selectionController, SIGNAL( currentSubscriptionChanged( Akregator::TreeNode* ) ),
             this, SLOT( slotNodeSelected( Akregator::TreeNode* ) ) );

    connect( m_selectionController, SIGNAL( currentArticleChanged( Akregator::Article ) ),
             this, SLOT( slotArticleSelected( Akregator::Article ) ) );

    connect( m_selectionController, SIGNAL( articleDoubleClicked( Akregator::Article ) ),
             this, SLOT( slotOpenArticleInBrowser( Akregator::Article )) );

    void currentArticleIndexChanged( const QModelIndex& index );
    m_actionManager->initArticleListView(m_articleListView);

    connect( m_articleListView, SIGNAL(signalMouseButtonPressed(int, const Akregator::Article&, const QPoint &, int)),
             this, SLOT(slotMouseButtonPressed(int, const Akregator::Article&, const QPoint &, int)));


    connect( m_part, SIGNAL(signalSettingsChanged()),
             m_articleListView, SLOT(slotPaletteOrFontChanged()));

    m_articleViewer = new ArticleViewer(m_articleSplitter);

    m_actionManager->initArticleViewer(m_articleViewer);

    connect( m_articleViewer, SIGNAL(signalOpenUrlRequest(Akregator::OpenUrlRequest&)),
             Kernel::self()->frameManager(), SLOT(slotOpenUrlRequest(Akregator::OpenUrlRequest&)) );

    connect( m_articleViewer->part()->browserExtension(), SIGNAL(mouseOverInfo(const KFileItem&)),
             this, SLOT(slotMouseOverInfo(const KFileItem&)) );

    connect( m_part, SIGNAL(signalSettingsChanged()),
             m_articleViewer, SLOT(slotPaletteOrFontChanged()));
    m_articleViewer->part()->widget()->setWhatsThis( i18n("Browsing area."));
    mainTabLayout->addWidget( m_articleSplitter );

    m_mainFrame = new MainFrame(this, m_part, m_mainTab, i18n("Articles"));

    Kernel::self()->frameManager()->slotAddFrame(m_mainFrame);

    const QList<int> sp1sizes = Settings::splitter1Sizes();
    if ( sp1sizes.count() >= m_horizontalSplitter->count() )
        m_horizontalSplitter->setSizes( sp1sizes );
    const QList<int> sp2sizes = Settings::splitter2Sizes();
    if ( sp2sizes.count() >= m_articleSplitter->count() )
        m_articleSplitter->setSizes( sp2sizes );

    KConfigGroup conf(Settings::self()->config(), "General");
    if(!conf.readEntry("Disable Introduction", false))
    {
        m_articleListView->hide();
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

void Akregator::MainWidget::delayedInit()
{
    // HACK, FIXME:
    // for some reason, m_part->factory() is NULL at startup of kontact,
    // and thus the article viewer GUI can't be merged when creating the view.
    // Even the delayed init didn't help. Well, we retry every half a second until
    // it works. This is kind of creative, but a dirty hack nevertheless.
    if ( !m_part->mergePart(m_articleViewer->part()) )
        QTimer::singleShot(500, this, SLOT(delayedInit()));
}

void Akregator::MainWidget::slotOnShutdown()
{
    m_shuttingDown = true;

    Kernel::self()->fetchQueue()->slotAbort();

    m_selectionController->setFeedList( 0 );
    ProgressManager::self()->setFeedList(0);

    delete m_feedList;

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

void Akregator::MainWidget::saveSettings()
{
    Settings::setSplitter1Sizes( m_horizontalSplitter->sizes() );
    Settings::setSplitter2Sizes( m_articleSplitter->sizes() );
    Settings::setViewMode( m_viewMode );
    Settings::self()->writeConfig();
}


void Akregator::MainWidget::slotRequestNewFrame(int& frameId)
{
    BrowserFrame* frame = new BrowserFrame(m_tabWidget);

    connect( m_part, SIGNAL(signalSettingsChanged()), frame, SLOT(slotPaletteOrFontChanged()));

    Kernel::self()->frameManager()->slotAddFrame(frame);

    frameId = frame->id();
}

void Akregator::MainWidget::setTabIcon(const QPixmap& icon)
{
    ArticleViewer* s = dynamic_cast<ArticleViewer*>(sender());
    if (s)
    {
        m_tabWidget->setTabIcon(m_tabWidget->indexOf(s->part()->widget()), icon);
    }
}

void Akregator::MainWidget::sendArticle(bool attach)
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

bool Akregator::MainWidget::importFeeds(const QDomDocument& doc)
{
    FeedList* feedList = new FeedList( Kernel::self()->storage() );
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

bool Akregator::MainWidget::loadFeeds(const QDomDocument& doc, Folder* parent)
{
    FeedList* feedList = new FeedList( Kernel::self()->storage() );
    bool parsed = feedList->readFromXML(doc);

    // parsing went wrong
    if (!parsed)
    {
        delete feedList;
        return false;
    }
    m_feedListView->setUpdatesEnabled(false);
    if (!parent)
    {
        Kernel::self()->setFeedList(feedList);
        ProgressManager::self()->setFeedList(feedList);
        disconnectFromFeedList(m_feedList);
        delete m_feedList;
        m_feedList = feedList;
        connectToFeedList(m_feedList);

        m_selectionController->setFeedList( m_feedList );
    }
    else
        m_feedList->append(feedList, parent);

    m_feedListView->setUpdatesEnabled(true);
    m_feedListView->triggerUpdate();
    return true;
}

void Akregator::MainWidget::slotDeleteExpiredArticles()
{
    TreeNode* rootNode = m_feedList->rootNode();
    Akregator::ArticleDeleteJob* job = new Akregator::ArticleDeleteJob;
    if (rootNode)
        rootNode->deleteExpiredArticles( job );
    job->start();
}

QDomDocument Akregator::MainWidget::feedListToOPML()
{
    return m_feedList->toXML();
}

void Akregator::MainWidget::addFeedToGroup(const QString& url, const QString& groupName)
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

void Akregator::MainWidget::slotNormalView()
{
    if (m_viewMode == NormalView)
    return;

    if (m_viewMode == CombinedView)
    {
        m_articleListView->show();

        const Article article =  m_selectionController->currentArticle();

        if (!article.isNull())
            m_articleViewer->showArticle(article);
        else
            m_articleViewer->slotShowSummary( m_selectionController->selectedSubscription() );
    }

    m_articleSplitter->setOrientation(Qt::Vertical);
    m_viewMode = NormalView;

    Settings::setViewMode( m_viewMode );
}

void Akregator::MainWidget::slotWidescreenView()
{
    if (m_viewMode == WidescreenView)
    return;

    if (m_viewMode == CombinedView)
    {
        m_articleListView->show();

        Article article =  m_selectionController->currentArticle();

        if (!article.isNull())
            m_articleViewer->showArticle(article);
        else
            m_articleViewer->slotShowSummary( m_selectionController->selectedSubscription() );
    }

    m_articleSplitter->setOrientation(Qt::Horizontal);
    m_viewMode = WidescreenView;

    Settings::setViewMode( m_viewMode );
}

void Akregator::MainWidget::slotCombinedView()
{
    if (m_viewMode == CombinedView)
        return;

    m_articleListView->slotClear();
    m_articleListView->hide();
    m_viewMode = CombinedView;

    Settings::setViewMode( m_viewMode );
}

void Akregator::MainWidget::slotFeedTreeContextMenu(K3ListView*, TreeNode* /*node*/, const QPoint& /*p*/)
{
    m_tabWidget->setCurrentWidget( m_mainFrame );
}

void Akregator::MainWidget::slotMoveCurrentNodeUp()
{
    TreeNode* current = m_selectionController->selectedSubscription();
    if (!current)
        return;
    TreeNode* prev = current->prevSibling();
    Folder* parent = current->parent();

    if (!prev || !parent)
        return;

    parent->removeChild(prev);
    parent->insertChild(prev, current);
    m_feedListView->ensureNodeVisible(current);
}

void Akregator::MainWidget::slotMoveCurrentNodeDown()
{
    TreeNode* current = m_selectionController->selectedSubscription();
    if (!current)
        return;
    TreeNode* next = current->nextSibling();
    Folder* parent = current->parent();

    if (!next || !parent)
        return;

    parent->removeChild(current);
    parent->insertChild(current, next);
    m_feedListView->ensureNodeVisible(current);
}

void Akregator::MainWidget::slotMoveCurrentNodeLeft()
{
    TreeNode* current = m_selectionController->selectedSubscription();
    if (!current || !current->parent() || !current->parent()->parent())
        return;

    Folder* parent = current->parent();
    Folder* grandparent = current->parent()->parent();

    parent->removeChild(current);
    grandparent->insertChild(current, parent);
    m_feedListView->ensureNodeVisible(current);
}

void Akregator::MainWidget::slotMoveCurrentNodeRight()
{
    TreeNode* current = m_selectionController->selectedSubscription();
    if (!current || !current->parent())
        return;
    TreeNode* prev = current->prevSibling();

    if ( prev && prev->isGroup() )
    {
        Folder* fg = static_cast<Folder*>(prev);
        current->parent()->removeChild(current);
        fg->appendChild(current);
        m_feedListView->ensureNodeVisible(current);
    }
}

void Akregator::MainWidget::slotNodeSelected(TreeNode* node)
{
    m_markReadTimer->stop();

    if (m_displayingAboutPage)
    {
        m_mainFrame->slotSetTitle(i18n("Articles"));
        if (m_viewMode != CombinedView)
            m_articleListView->show();
        if (Settings::showQuickFilter())
            m_searchBar->show();
        m_displayingAboutPage = false;
    }

    m_tabWidget->setCurrentWidget( m_mainFrame );

    m_searchBar->slotClearSearch();

    if (m_viewMode == CombinedView)
    {
        m_articleViewer->showNode(node);
    }
    else
    {
        m_articleViewer->slotShowSummary(node);
    }

    if (node)
       m_mainFrame->setWindowTitle(node->title());

    m_actionManager->slotNodeSelected(node);
}


void Akregator::MainWidget::slotFeedAdd()
{
    Folder* group = 0;
    if ( !m_selectionController->selectedSubscription() )
        group = m_feedList->rootNode(); // all feeds
    else
    {
        if ( m_selectionController->selectedSubscription()->isGroup())
            group = static_cast<Folder*>( m_selectionController->selectedSubscription() );
        else
            group= m_selectionController->selectedSubscription()->parent();

    }

    TreeNode* lastChild = group->children().last();

    addFeed(QString::null, lastChild, group, false);	//krazy:exclude=nullstrassign for old broken gcc
}

void Akregator::MainWidget::addFeed(const QString& url, TreeNode *after, Folder* parent, bool autoExec)
{

    QPointer<AddFeedDialog> afd = new AddFeedDialog( this, "add_feed" );

    afd->setUrl(KUrl::fromPercentEncoding( url.toLatin1() ));

    QPointer<QObject> thisPointer( this );

    if ( autoExec )
        afd->accept();
    else
        afd->exec();

    if ( !thisPointer ) // "this" might have been deleted while exec()!
        return;

    Feed* const feed = afd->feed();
    delete afd;

    if ( !feed )
        return;

    QPointer<FeedPropertiesDialog> dlg = new FeedPropertiesDialog( this, "edit_feed" );
    dlg->setFeed(feed);
    dlg->selectFeedName();

    if ( !autoExec && ( dlg->exec() != QDialog::Accepted || !thisPointer ) )
    {
        delete feed;
    }
    else
    {
        parent = parent ? parent : m_feedList->rootNode();
        parent->insertChild(feed, after);
        m_feedListView->ensureNodeVisible(feed);
    }

    delete dlg;
}

void Akregator::MainWidget::slotFeedAddGroup()
{
    TreeNode* node = m_selectionController->selectedSubscription();
    TreeNode* after = 0;

    if (!node)
        node = m_feedList->rootNode();

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

        m_feedListView->ensureNodeVisible(newGroup);
    }
}

void Akregator::MainWidget::slotFeedRemove()
{
    TreeNode* selectedNode = m_selectionController->selectedSubscription();

    // don't delete root element! (safety valve)
    if (!selectedNode || selectedNode == m_feedList->rootNode())
        return;

    m_deleteNodeVisitor->visit(selectedNode);
}

void Akregator::MainWidget::slotFeedModify()
{
    TreeNode* node = m_selectionController->selectedSubscription();
    if (node)
        m_editNodePropertiesVisitor->visit(node);

}

void Akregator::MainWidget::slotNextUnreadArticle()
{
    if (m_viewMode == CombinedView)
        m_feedListView->slotNextUnreadFeed();

    TreeNode* sel = m_selectionController->selectedSubscription();
    if (sel && sel->unread() > 0)
        m_articleListView->slotNextUnreadArticle();
    else
        m_feedListView->slotNextUnreadFeed();
}

void Akregator::MainWidget::slotPrevUnreadArticle()
{
    if (m_viewMode == CombinedView)
        m_feedListView->slotPrevUnreadFeed();

    TreeNode* sel = m_selectionController->selectedSubscription();
    if (sel && sel->unread() > 0)
        m_articleListView->slotPreviousUnreadArticle();
    else
        m_feedListView->slotPrevUnreadFeed();
}

void Akregator::MainWidget::slotMarkAllFeedsRead()
{
    m_feedList->rootNode()->slotMarkAllArticlesAsRead();
}

void Akregator::MainWidget::slotMarkAllRead()
{
    if(!m_selectionController->selectedSubscription())
        return;
    m_selectionController->selectedSubscription()->slotMarkAllArticlesAsRead();
}

void Akregator::MainWidget::slotSetTotalUnread()
{
    emit signalUnreadCountChanged( m_feedList->rootNode()->unread() );
}

void Akregator::MainWidget::slotDoIntervalFetches()
{
    if ( Solid::Networking::status() != Solid::Networking::Connected )
        return;
    m_feedList->rootNode()->slotAddToFetchQueue(Kernel::self()->fetchQueue(), true);
}

void Akregator::MainWidget::slotFetchCurrentFeed()
{
    if ( !m_selectionController->selectedSubscription() )
        return;
    m_selectionController->selectedSubscription()->slotAddToFetchQueue(Kernel::self()->fetchQueue());
}

void Akregator::MainWidget::slotFetchAllFeeds()
{
    m_feedList->rootNode()->slotAddToFetchQueue(Kernel::self()->fetchQueue());
}

void Akregator::MainWidget::slotFetchingStarted()
{
    m_mainFrame->slotSetState(Frame::Started);
    m_actionManager->action("feed_stop")->setEnabled(true);
    m_mainFrame->slotSetStatusText(i18n("Fetching Feeds..."));
}

void Akregator::MainWidget::slotFetchingStopped()
{
    m_mainFrame->slotSetState(Frame::Completed);
    m_actionManager->action("feed_stop")->setEnabled(false);
    m_mainFrame->slotSetStatusText(QString());
}

void Akregator::MainWidget::slotFeedFetched(Feed *feed)
{
    // iterate through the articles (once again) to do notifications properly
    if (feed->articles().count() > 0)
    {
        QList<Article> articles = feed->articles();
        QList<Article>::ConstIterator it;
        QList<Article>::ConstIterator end = articles.end();
        for (it = articles.begin(); it != end; ++it)
        {
            if ((*it).status()==Akregator::New && ((*it).feed()->useNotification() || Settings::useNotifications()))
            {
                NotificationManager::self()->slotNotifyArticle(*it);
            }
        }
    }
}



void Akregator::MainWidget::slotArticleSelected(const Akregator::Article& article)
{
    if (m_viewMode == CombinedView)
        return;

    m_markReadTimer->stop();

    const Feed *feed = article.feed();
    if (!feed)
        return;

    if (article.status() != Akregator::Read)
    {
        if ( Settings::useMarkReadDelay() )
        {
            const int delay = Settings::markReadDelay();

            if ( delay > 0 )
            {
                m_markReadTimer->start( delay * 1000 );
            }
            else
            {
                Akregator::ArticleModifyJob* job = new Akregator::ArticleModifyJob;
                const Akregator::ArticleId aid = { article.feed()->xmlUrl(), article.guid() };
                job->setStatus( aid, Akregator::Read );
                job->start();
            }
        }
    }

    KToggleAction* maai = static_cast<KToggleAction*>(m_actionManager->action("article_set_status_important"));
    maai->setChecked(article.keep());

    kDebug() <<"selected:" << article.guid();

    m_articleViewer->showArticle( article );
}

void Akregator::MainWidget::slotMouseButtonPressed(int button, const Article& article, const QPoint &, int)
{
    if (article.isNull() || button != Qt::MidButton)
        return;

    KUrl url = article.link();

    if (!url.isValid())
        return;

    OpenUrlRequest req(url);

    switch (Settings::mMBBehaviour())
    {
        case Settings::EnumMMBBehaviour::OpenInExternalBrowser:
            req.setOptions(OpenUrlRequest::ExternalBrowser);
            break;
        case Settings::EnumMMBBehaviour::OpenInBackground:
            req.setOptions(OpenUrlRequest::NewTab);
            req.setOpenInBackground(true);
            break;
        default:
            req.setOptions(OpenUrlRequest::NewTab);
            req.setOpenInBackground(false);
    }

    Kernel::self()->frameManager()->slotOpenUrlRequest(req);
}

void Akregator::MainWidget::slotOpenHomepage()
{
    Feed* feed = dynamic_cast<Feed *>( m_selectionController->selectedSubscription() );

    if (!feed)
        return;

    KUrl url(feed->htmlUrl());

    if (url.isValid())
    {
        OpenUrlRequest req(feed->htmlUrl());
        req.setOptions(OpenUrlRequest::ExternalBrowser);
        Kernel::self()->frameManager()->slotOpenUrlRequest(req);
    }
}

void Akregator::MainWidget::slotOpenCurrentArticleInBrowser()
{
    slotOpenArticleInBrowser( m_selectionController->currentArticle() );
}

void Akregator::MainWidget::slotOpenArticleInBrowser(const Akregator::Article& article)
{
    if (!article.isNull() && article.link().isValid())
    {
        OpenUrlRequest req(article.link());
        req.setOptions(OpenUrlRequest::ExternalBrowser);
        Kernel::self()->frameManager()->slotOpenUrlRequest(req);
    }
}


void Akregator::MainWidget::slotOpenCurrentArticle()
{
    Article article =  m_selectionController->currentArticle();

    if (article.isNull())
        return;

    KUrl url = article.link();

    if (url.isValid())
    {
        OpenUrlRequest req(url);
        // TODO: (re-)add a setting for foreground/background
        // and use it here
        Kernel::self()->frameManager()->slotOpenUrlRequest(req);
    }
}

void Akregator::MainWidget::slotCopyLinkAddress()
{
    const Article article =  m_selectionController->currentArticle();

    if(article.isNull())
       return;

    QString link;
    if (article.link().isValid())
    {
        link = article.link().url();
        QClipboard *cb = QApplication::clipboard();
        cb->setText(link, QClipboard::Clipboard);
        // don't set url to selection as it's a no-no according to a fd.o spec
        //cb->setText(link, QClipboard::Selection);
    }
}

void Akregator::MainWidget::slotFeedUrlDropped(KUrl::List &urls, TreeNode* after, Folder* parent)
{
    KUrl::List::iterator it;
    for ( it = urls.begin(); it != urls.end(); ++it )
    {
        addFeed((*it).prettyUrl(), after, parent, false);
    }
}

void Akregator::MainWidget::slotToggleShowQuickFilter()
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

void Akregator::MainWidget::slotArticleDelete()
{

    if ( m_viewMode == CombinedView )
        return;

    QList<Article> articles = m_selectionController->selectedArticles();

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

    if (KMessageBox::warningContinueCancel(0, msg, i18n("Delete Article"), KStandardGuiItem::del()) == KMessageBox::Continue)
    {
        if ( m_selectionController->selectedSubscription() )
            m_selectionController->selectedSubscription()->setNotificationMode(false);

        QSet<Feed*> feeds;
        Q_FOREACH( const Akregator::Article i, articles )
        {
            Feed* feed = i.feed();
            feeds.insert( feed );
            feed->setNotificationMode(false);
            Akregator::ArticleDeleteJob* job = new Akregator::ArticleDeleteJob;
            const Akregator::ArticleId aid = { feed->xmlUrl(), i.guid() };
            job->appendArticleId( aid );
            job->start();
        }

        Q_FOREACH( Akregator::Feed* i, feeds )
            i->setNotificationMode(true);

        if ( m_selectionController->selectedSubscription() )
            m_selectionController->selectedSubscription()->setNotificationMode(true);
    }
}


void Akregator::MainWidget::slotArticleToggleKeepFlag(bool /*enabled*/)
{
    const QList<Article> articles = m_selectionController->selectedArticles();

    if (articles.isEmpty())
        return;

    bool allFlagsSet = true;
    Q_FOREACH ( const Akregator::Article i, articles )
    {
        allFlagsSet = allFlagsSet && i.keep();
        if ( !allFlagsSet )
            break;
    }

    Akregator::ArticleModifyJob* job = new Akregator::ArticleModifyJob;
    Q_FOREACH ( const Akregator::Article i, articles )
    {
        const Akregator::ArticleId aid = { i.feed()->xmlUrl(), i.guid() };
        job->setKeep( aid, !allFlagsSet );
    }
    job->start();
}

namespace {

void setSelectedArticleStatus( const Akregator::AbstractSelectionController* controller, int status )
{
    const QList<Akregator::Article> articles = controller->selectedArticles();

    if (articles.isEmpty())
        return;

    Akregator::ArticleModifyJob* job = new Akregator::ArticleModifyJob;
    Q_FOREACH ( const Akregator::Article i, articles )
    {
        const Akregator::ArticleId aid = { i.feed()->xmlUrl(), i.guid() };
        job->setStatus( aid, status );
    }
    job->start();
}

}

void Akregator::MainWidget::slotSetSelectedArticleRead()
{
    ::setSelectedArticleStatus( m_selectionController, Akregator::Read );
}

void Akregator::MainWidget::slotTextToSpeechRequest()
{

    if (Kernel::self()->frameManager()->currentFrame() == m_mainFrame)
    {
        if (m_viewMode != CombinedView)
        {
            // in non-combined view, read selected articles
#ifdef __GNUC__
#warning "kde4:readd speechclient";
#endif
            //SpeechClient::self()->slotSpeak(m_selectionController->selectedArticles());
            // TODO: if article viewer has a selection, read only the selected text?
        }
        else
        {
            if (m_selectionController->selectedSubscription())
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

void Akregator::MainWidget::slotSetSelectedArticleUnread()
{
    ::setSelectedArticleStatus( m_selectionController, Akregator::Unread );
}

void Akregator::MainWidget::slotSetSelectedArticleNew()
{
    ::setSelectedArticleStatus( m_selectionController, Akregator::New );
}

void Akregator::MainWidget::slotSetCurrentArticleReadDelayed()
{
    const Article article =  m_selectionController->currentArticle();

    if (article.isNull())
        return;

    Akregator::ArticleModifyJob* const job = new Akregator::ArticleModifyJob;
    const Akregator::ArticleId aid = { article.feed()->xmlUrl(), article.guid() };
    job->setStatus( aid, Akregator::Read );
    job->start();
}

void Akregator::MainWidget::slotMouseOverInfo(const KFileItem& kifi)
{
    m_mainFrame->slotSetStatusText( kifi.isNull() ? QString() : kifi.url().prettyUrl() );
}

void Akregator::MainWidget::readProperties(const KConfigGroup &config)
{
    // read filter settings
    m_searchBar->slotSetText(config.readEntry("searchLine"));
    m_searchBar->slotSetStatus(config.readEntry("searchCombo").toInt());
}

void Akregator::MainWidget::saveProperties(KConfigGroup & config)
{
    // save filter settings
    config.writeEntry("searchLine", m_searchBar->text());
    config.writeEntry("searchCombo", m_searchBar->status());
}

void Akregator::MainWidget::connectToFeedList(FeedList* feedList)
{
    connect(feedList->rootNode(), SIGNAL(signalChanged(Akregator::TreeNode*)), this, SLOT(slotSetTotalUnread()));
    slotSetTotalUnread();
}

void Akregator::MainWidget::disconnectFromFeedList(FeedList* feedList)
{
    disconnect(feedList->rootNode(), SIGNAL(signalChanged(Akregator::TreeNode*)), this, SLOT(slotSetTotalUnread()));
}


#include "mainwidget.moc"
