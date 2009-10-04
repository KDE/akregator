/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
                  2004 Sashmit Bhaduri <smt@vfemail.net>
                  2005 Frank Osterfeld <osterfeld@kde.org>

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
#include "akregatorconfig.h"
#include "akregator_part.h"
#include "browserframe.h"
#include "createfeedcommand.h"
#include "createtagcommand.h"
#include "deletesubscriptioncommand.h"
#include "editsubscriptioncommand.h"
#include "importfeedlistcommand.h"
#include "exportfeedlistcommand.h"
#include "framemanager.h"
#include "kernel.h"
#include "migratefeedscommand.h"
#include "notificationmanager.h"
#include "openurlrequest.h"
#include "progressmanager.h"
#include "searchbar.h"
#include "selectioncontroller.h"
#include "speechclient.h"
#include "tabwidget.h"
#include "types.h"

#include <krss/feedlist.h>
#include <krss/feedlistmodel.h>
#include <krss/item.h>
#include <krss/itemjobs.h>
#include <krss/netresource.h>
#include <krss/resourcemanager.h>
#include <krss/tagprovider.h>
#include <krss/resourcemanager.h>
#include <krss/treenode.h>
#include <krss/treenodevisitor.h>
#include <krss/netfeed.h>
#include <krss/tagjobs.h>
#include <krss/feedjobs.h>

#include <krss/ui/feedlistview.h>
#include <krss/ui/tagpropertiesdialog.h>
#include <krss/statusmodifyjob.h>

#include <solid/networking.h>

#include <kaction.h>
#include <kdialog.h>
#include <KDebug>
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
#include <QDomDocument>
#include <QTimer>

#include <algorithm>
#include <memory>
#include <cassert>
#include <boost/shared_ptr.hpp>

using namespace boost;
using namespace Akregator;
using namespace Solid;
using std::auto_ptr;
using boost::weak_ptr;

class MainWidget::Private {
    MainWidget* const q;
public:
    explicit Private( MainWidget* qq ) : q( qq ) {}
    void setUpAndStart( Command* cmd ) {
        cmd->setParentWidget( q );
        if ( cmd->isUserVisible() )
            ProgressManager::self()->addJob( cmd );
        cmd->start();
    }
};

Akregator::MainWidget::~MainWidget()
{
    // if m_shuttingDown is false, slotOnShutdown was not called. That
     // means that not the whole app is shutdown, only the part. So it
    // should be no risk to do the cleanups now
    if (!m_shuttingDown)
        slotOnShutdown();
    delete d;
}

Akregator::MainWidget::MainWidget( Part *part, QWidget *parent, ActionManagerImpl* actionManager)
     : QWidget(parent),
     d( new Private( this ) ),
     m_feedList(),
     m_viewMode(NormalView),
     m_actionManager(actionManager)
{
    m_actionManager->initMainWidget(this);
    m_actionManager->initFrameManager(Kernel::self()->frameManager());
    m_part = part;
    m_shuttingDown = false;
    m_displayingAboutPage = false;
    setFocusPolicy(Qt::StrongFocus);

    QVBoxLayout *lt = new QVBoxLayout( this );
    lt->setMargin(0);

    m_horizontalSplitter = new QSplitter(Qt::Horizontal, this);

    m_horizontalSplitter->setOpaqueResize(true);
    lt->addWidget(m_horizontalSplitter);

    const QString defaultResourceId = Settings::activeAkonadiResource();
    const shared_ptr<const KRss::NetResource> resource = KRss::ResourceManager::self()->resource( defaultResourceId );
    connect( resource.get(), SIGNAL( fetchQueueStarted() ),
             this, SLOT( slotFetchQueueStarted() ) );
    connect( resource.get(), SIGNAL( fetchQueueFinished() ),
             this, SLOT( slotFetchQueueFinished() ) );

    m_feedListView = new KRss::FeedListView( m_horizontalSplitter );
    const KConfigGroup group( Settings::self()->config(), "General" );
    m_feedListView->setConfigGroup( group );
    m_actionManager->initFeedListView( m_feedListView );

    connect(m_feedListView, SIGNAL(signalDropped (KUrl::List &, Akregator::TreeNode*,
            Akregator::Folder*)),
            this, SLOT(slotFeedUrlDropped (KUrl::List &,
            Akregator::TreeNode*, Akregator::Folder*)));

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
    m_mainTab->setWhatsThis( i18n("Articles list."));

    QVBoxLayout *mainTabLayout = new QVBoxLayout( m_mainTab);
    mainTabLayout->setMargin(0);

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

    connect(m_searchBar, SIGNAL( signalSearch( std::vector<boost::shared_ptr<const Akregator::Filters::AbstractMatcher> > ) ),
            m_selectionController, SLOT( setFilters( std::vector<boost::shared_ptr<const Akregator::Filters::AbstractMatcher> > ) ) );

#ifdef KRSS_PORT_DISABLED
    FolderExpansionHandler* expansionHandler = new FolderExpansionHandler( this );
    connect( m_feedListView, SIGNAL( expanded( QModelIndex ) ), expansionHandler, SLOT( itemExpanded( QModelIndex ) ) );
    connect( m_feedListView, SIGNAL( collapsed( QModelIndex ) ), expansionHandler, SLOT( itemCollapsed( QModelIndex ) ) );

    m_selectionController->setFolderExpansionHandler( expansionHandler );
#else
    kWarning() << "Code temporarily disabled (Akonadi port)";
#endif //KRSS_PORT_DISABLED

    connect( m_selectionController, SIGNAL( currentSubscriptionChanged(boost::shared_ptr<KRss::TreeNode>) ),
             this, SLOT( slotNodeSelected(boost::shared_ptr<KRss::TreeNode>) ) );

    connect( m_selectionController, SIGNAL(currentItemChanged(KRss::Item)),
             this, SLOT(slotItemSelected(KRss::Item)) );

    connect( m_selectionController, SIGNAL(itemDoubleClicked(KRss::Item)),
             this, SLOT(slotOpenItemInBrowser(KRss::Item)) );

    m_actionManager->initArticleListView(m_articleListView);

    connect( m_articleListView, SIGNAL(signalMouseButtonPressed(int, KUrl )),
             this, SLOT(slotMouseButtonPressed(int, KUrl )));

/*
    connect( m_part, SIGNAL(signalSettingsChanged()),
             m_articleListView, SLOT(slotPaletteOrFontChanged()));
*/

    m_articleViewer = new ArticleViewer(m_articleSplitter);
    m_actionManager->initArticleViewer(m_articleViewer);
    m_articleListView->setFocusProxy(m_articleViewer);

    connect( m_articleViewer, SIGNAL(signalOpenUrlRequest(Akregator::OpenUrlRequest& )),
             Kernel::self()->frameManager(), SLOT(slotOpenUrlRequest( Akregator::OpenUrlRequest& )) );
    connect( m_articleViewer->part()->browserExtension(), SIGNAL(mouseOverInfo( KFileItem )),
             this, SLOT(slotMouseOverInfo( KFileItem )) );
    connect( m_part, SIGNAL(signalSettingsChanged()),
             m_articleViewer, SLOT(slotPaletteOrFontChanged()));
    connect(m_searchBar, SIGNAL( signalSearch( std::vector<boost::shared_ptr<const Akregator::Filters::AbstractMatcher> > ) ),
            m_articleViewer, SLOT( setFilters( std::vector<boost::shared_ptr<const Akregator::Filters::AbstractMatcher> > ) ) );

    m_articleViewer->part()->widget()->setWhatsThis( i18n("Browsing area."));

    mainTabLayout->addWidget( m_articleSplitter );

    m_mainFrame = new MainFrame( this, m_part, m_mainTab );
    m_mainFrame->slotSetTitle( i18n("Articles") );
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

    if ( !Settings::resetQuickFilterOnNodeChange() )
    {
        m_searchBar->slotSetStatus( Settings::statusFilter() );
        m_searchBar->slotSetText( Settings::textFilter() );
    }
}

void Akregator::MainWidget::slotOnShutdown()
{
    m_shuttingDown = true;

    // close all pageviewers in a controlled way
    // fixes bug 91660, at least when no part loading data
    while ( m_tabWidget->count() > 1 ) { // remove frames until only the main frame remains
        m_tabWidget->setCurrentIndex( m_tabWidget->count() - 1 ); // select last page
        m_tabWidget->slotRemoveCurrentFrame();
    }

    delete m_feedListView; // call delete here, so that the header settings will get saved
    delete m_articleListView; // same for this one

    delete m_mainTab;
    delete m_mainFrame;

    Settings::self()->writeConfig();
}


void Akregator::MainWidget::saveSettings()
{
    const QList<int> spl1 = m_horizontalSplitter->sizes();
    if ( std::count( spl1.begin(), spl1.end(), 0 ) == 0 )
        Settings::setSplitter1Sizes( spl1 );
    const QList<int> spl2 = m_articleSplitter->sizes();
    if ( std::count( spl2.begin(), spl2.end(), 0 ) == 0 )
        Settings::setSplitter2Sizes( spl2 );
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

void Akregator::MainWidget::sendArticle(bool attach)
{
    QByteArray text;
    QString title;

    Frame* frame = Kernel::self()->frameManager()->currentFrame();

    if (frame && frame->id() > 0) { // are we in some other tab than the articlelist?
        text = frame->url().prettyUrl().toLatin1();
        title = frame->title();
    }
    else { // nah, we're in articlelist..
         const KRss::Item item = m_selectionController->currentItem();
         if(!item.isNull()) {
             text = KUrl( item.link() ).prettyUrl().toLatin1();
             title = item.title();
         }
    }

    if(text.isEmpty())
        return;

    if(attach)
    {
        KToolInvocation::invokeMailer(QString(),
                           QString(),
                           QString(),
                           title,
                           QString(),
                           QString(),
                           QStringList(text),
                           text);
    }
    else
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
}

void MainWidget::slotImportFeedList()
{
    std::auto_ptr<ImportFeedListCommand> cmd( new ImportFeedListCommand );
    cmd->setResourceIdentifier( Settings::activeAkonadiResource() );
    d->setUpAndStart( cmd.release() );
}

void MainWidget::slotExportFeedList()
{
    std::auto_ptr<ExportFeedListCommand> cmd( new ExportFeedListCommand );
    cmd->setResourceIdentifier( Settings::activeAkonadiResource() );
    d->setUpAndStart( cmd.release() );
}

void MainWidget::slotMetakitImport()
{
    std::auto_ptr<MigrateFeedsCommand> cmd( new MigrateFeedsCommand );
    cmd->setOpmlFile( KGlobal::dirs()->saveLocation("data", "akregator/data") + "/feeds.opml" );
    cmd->setResource( Settings::activeAkonadiResource() );
    d->setUpAndStart( cmd.release() );
}

void Akregator::MainWidget::setFeedList( const shared_ptr<KRss::FeedList>& list )
{
    if ( list == m_feedList )
        return;
    const shared_ptr<KRss::FeedList> oldList = m_feedList;

    m_feedList = list;
    if ( m_feedList ) {
        connect( m_feedList.get(), SIGNAL(unreadCountChanged(int) ),
                 this, SLOT(slotSetTotalUnread()) );
    }

    slotSetTotalUnread();

    m_articleViewer->setFeedList( m_feedList );
    Kernel::self()->setFeedList( m_feedList );
    NotificationManager::self()->setFeedList( m_feedList );
    ProgressManager::self()->setFeedList( m_feedList );
    m_selectionController->setFeedList( m_feedList );

    if ( oldList )
        oldList->disconnect( this );
}

void Akregator::MainWidget::setTagProvider( const boost::shared_ptr<const KRss::TagProvider>& tagProvider )
{
    if ( tagProvider == m_tagProvider )
        return;

    m_tagProvider = tagProvider;
    m_selectionController->setTagProvider( m_tagProvider );
}

void Akregator::MainWidget::addFeedToGroup(const QString& url, const QString& groupName)
{
#ifdef KRSS_PORT_DISABLED
    // Locate the group.
    QList<TreeNode *> namedGroups = m_feedList->findByTitle( groupName );
    Folder* group = 0;
    foreach( TreeNode* const candidate, namedGroups ) {
        if ( candidate->isGroup() ) {
            group =  static_cast<Folder*>( candidate );
            break;
        }
    }

    if (!group)
    {
        Folder* g = new Folder( groupName );
        m_feedList->allFeedsFolder()->appendChild(g);
        group = g;
    }

#endif

    // Invoke the Add Feed dialog with url filled in.
    addFeed(url, true);
}

void Akregator::MainWidget::slotNormalView()
{
    if (m_viewMode == NormalView)
        return;

    if (m_viewMode == CombinedView)
    {
        m_articleListView->show();

        const KRss::Item item = m_selectionController->currentItem();

        if ( !item.isNull() )
            m_articleViewer->showItem( item );
        else
            m_articleViewer->slotShowSummary( m_feedList, m_selectionController->selectedSubscription() );
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

        const KRss::Item item = m_selectionController->currentItem();

        if ( !item.isNull() )
            m_articleViewer->showItem( item );
        else
            m_articleViewer->slotShowSummary( m_feedList, m_selectionController->selectedSubscription() );
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


void Akregator::MainWidget::slotNodeSelected(const boost::shared_ptr<KRss::TreeNode>& node)
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
    if ( Settings::resetQuickFilterOnNodeChange() )
        m_searchBar->slotClearSearch();

    if (m_viewMode == CombinedView)
    {
        m_articleViewer->showNode( m_feedList, node );
    }
    else
    {
        m_articleViewer->slotShowSummary( m_feedList, node );
    }

    if (node)
       m_mainFrame->setWindowTitle( node->title( m_feedList ) );

    m_actionManager->slotNodeSelected(node);
}


void Akregator::MainWidget::slotFeedAdd()
{
    addFeed(QString(), false);
}

void Akregator::MainWidget::addFeed(const QString& url, bool autoExec)
{
    std::auto_ptr<CreateFeedCommand> cmd( new CreateFeedCommand( this ) );
    cmd->setAutoExecute( autoExec );
    cmd->setUrl( url );
    // FIXME: keep a shared pointer to the default resource in MainWidget
    const shared_ptr<KRss::NetResource> resource = KRss::ResourceManager::self()->resource(
                                                                Settings::activeAkonadiResource() );
    cmd->setResource( weak_ptr<KRss::NetResource>( resource ) );
    cmd->setFeedListView( m_feedListView );
    cmd->setFeedList( weak_ptr<KRss::FeedList>( m_feedList ) );
    d->setUpAndStart( cmd.release() );
}

void Akregator::MainWidget::slotTagAdd()
{
    std::auto_ptr<CreateTagCommand> cmd( new CreateTagCommand( m_tagProvider, this ) );
    cmd->setFeedListView( m_feedListView );
    d->setUpAndStart( cmd.release() );
}

void Akregator::MainWidget::slotFeedRemove()
{
    const shared_ptr<const KRss::TreeNode> treeNode = m_selectionController->selectedSubscription();
    if ( !treeNode )
        return;

    if ( treeNode->tier() == KRss::TreeNode::TagTier ) {
        const shared_ptr<const KRss::TagNode> tagNode = dynamic_pointer_cast<const KRss::TagNode,
                                                                             const KRss::TreeNode>( treeNode );
        assert( tagNode );
        KRss::TagDeleteJob * const job = m_tagProvider->tagDeleteJob();
        job->setTag( tagNode->tag() );
        connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotJobFinished( KJob* ) ) );
        job->start();
    }
    else { // FeedTier
        const shared_ptr<const KRss::FeedNode> feedNode = dynamic_pointer_cast<const KRss::FeedNode,
                                                                             const KRss::TreeNode>( treeNode );
        assert( feedNode );
        const shared_ptr<const KRss::Feed> feed = m_feedList->constFeedById( feedNode->feedId() );
        KRss::FeedDeleteJob * const job = new KRss::FeedDeleteJob( feed , this );
        connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotJobFinished( KJob* ) ) );
        job->start();
    }
}

void Akregator::MainWidget::slotFeedModify()
{
    const shared_ptr<KRss::TreeNode> treeNode = m_selectionController->selectedSubscription();
    if ( !treeNode )
        return;

    std::auto_ptr<EditSubscriptionCommand> cmd( new EditSubscriptionCommand );
    cmd->setTagProvider( m_tagProvider );
    cmd->setFeedList( m_feedList );
    cmd->setFeedListView( m_feedListView );
    cmd->setNode( treeNode );
    d->setUpAndStart( cmd.release() );
}

namespace {
    class RemoveTagFromFeedVisitor : public KRss::TreeNodeVisitor {
    public:
        explicit RemoveTagFromFeedVisitor( const shared_ptr<KRss::FeedList>& fl ) : feedList( fl ) {}

        void visit( const shared_ptr<KRss::FeedNode>& fn ) {
            if ( !feedList )
                return;
            const shared_ptr<KRss::Feed> f = feedList->feedById( fn->feedId() );
            if ( !f )
                return;
            const KRss::TagId id = fn->parent()->tag().id();
            f->removeTag( id );
            KRss::FeedModifyJob* job = new KRss::FeedModifyJob( f );
            job->start();
        }
        void visit( const shared_ptr<KRss::RootNode>& ) {}
        void visit( const shared_ptr<KRss::TagNode>& ) {}

        const shared_ptr<KRss::FeedList> feedList;
    };
}

void Akregator::MainWidget::slotFeedRemoveTag()
{
    const shared_ptr<KRss::TreeNode> treeNode = m_selectionController->selectedSubscription();
    RemoveTagFromFeedVisitor v( m_feedList );
    treeNode->accept( &v );
}

void Akregator::MainWidget::slotNextUnreadArticle()
{
    if (m_viewMode == CombinedView)
    {
        m_feedListView->slotNextUnreadFeed();
        return;
    }
    const shared_ptr<KRss::TreeNode> sel = m_selectionController->selectedSubscription();
    if (sel && sel->unreadCount( m_feedList ) > 0)
        m_articleListView->slotNextUnreadArticle();
    else
        m_feedListView->slotNextUnreadFeed();
}

void Akregator::MainWidget::slotPrevUnreadArticle()
{
    if (m_viewMode == CombinedView)
    {
        m_feedListView->slotPrevUnreadFeed();
        return;
    }
    const shared_ptr<KRss::TreeNode> sel = m_selectionController->selectedSubscription();

    if (sel && sel->unreadCount( m_feedList ) > 0)
        m_articleListView->slotPreviousUnreadArticle();
    else
        m_feedListView->slotPrevUnreadFeed();
}

void Akregator::MainWidget::slotMarkAllFeedsRead()
{
    KRss::CompositeStatusModifyJob * const job = new KRss::CompositeStatusModifyJob( this );
    Q_FOREACH( const shared_ptr<KRss::Feed>& feed, m_feedList->feeds() ) {
        job->addSubJob( feed->statusModifyJob() );
    }

    job->clearFlags( QList<KRss::Item::StatusFlag>() << KRss::Item::Unread << KRss::Item::New );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotJobFinished( KJob* ) ) );
    ProgressManager::self()->addJob( job );
    job->start();
}

void Akregator::MainWidget::slotMarkFeedRead()
{
    const shared_ptr<KRss::TreeNode> treeNode = m_selectionController->selectedSubscription();
    if ( !treeNode )
        return;

    KRss::CreateStatusModifyJobVisitor visitor( m_feedList );
    treeNode->accept( &visitor );
    KRss::StatusModifyJob * const job = visitor.statusModifyJob();
    job->clearFlags( QList<KRss::Item::StatusFlag>() << KRss::Item::Unread << KRss::Item::New );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotJobFinished( KJob* ) ) );
    ProgressManager::self()->addJob( job );
    job->start();
}

void Akregator::MainWidget::slotSetTotalUnread()
{
    emit signalUnreadCountChanged( m_feedList ? m_feedList->unreadCount() : 0 );
}

void Akregator::MainWidget::slotFetchCurrentFeed()
{
    const shared_ptr<KRss::TreeNode> treeNode = m_selectionController->selectedSubscription();
    if ( !treeNode )
        return;

    KRss::FetchVisitor visitor( m_feedList );
    treeNode->accept( &visitor );
}

void Akregator::MainWidget::slotFetchAllFeeds()
{
    if ( !m_feedList )
        return;
    Q_FOREACH( const shared_ptr<const KRss::Feed>& feed, m_feedList->constFeeds() ) {
        feed->fetch();
    }
}

void Akregator::MainWidget::slotAbortFetches() {
    if ( !m_feedList )
        return;
    Q_FOREACH( const shared_ptr<const KRss::Feed>& feed, m_feedList->constFeeds() )
        feed->abortFetch();
}

void Akregator::MainWidget::slotFetchQueueStarted()
{
    m_mainFrame->slotSetState(Frame::Started);
    m_actionManager->action("feed_stop")->setEnabled(true);
    m_mainFrame->slotSetStatusText(i18n("Fetching Feeds..."));
    m_actionManager->action( "feed_fetch_all" )->setEnabled( false );
}

void Akregator::MainWidget::slotFetchQueueFinished()
{
    m_mainFrame->slotSetState(Frame::Completed);
    m_actionManager->action("feed_stop")->setEnabled(false);
    m_mainFrame->slotSetStatusText(QString());
    m_actionManager->action( "feed_fetch_all" )->setEnabled( true );
}

void Akregator::MainWidget::slotItemSelected( const KRss::Item& item )
{
    if (m_viewMode == CombinedView)
        return;

    m_markReadTimer->stop();

    KToggleAction* const maai = qobject_cast<KToggleAction*>( m_actionManager->action( "article_set_status_important" ) );
    assert( maai );
    maai->setChecked( item.isImportant() );

    m_articleViewer->showItem( item );

    if ( item.isNull() || item.isRead() )
        return;

    if ( !Settings::useMarkReadDelay() )
        return;

    const int delay = Settings::markReadDelay();

    if ( delay > 0 )
    {
        m_markReadTimer->start( delay * 1000 );
    }
    else
    {
        KRss::Item modifiedItem = item;
        modifiedItem.setStatus( item.status() & ~( KRss::Item::New | KRss::Item::Unread ) );
        KRss::ItemModifyJob * const job = new KRss::ItemModifyJob();
        job->setItem( modifiedItem );
        job->setIgnorePayload( true );
        //PENDING(frank) connect to finished signal and report errors

        job->start();
    }
}

void Akregator::MainWidget::slotMouseButtonPressed(int button, const KUrl& url)
{
    if (button != Qt::MidButton)
        return;

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
    const shared_ptr<const KRss::TreeNode> treeNode = m_selectionController->selectedSubscription();
    if ( !treeNode )
        return;
    else if ( treeNode->tier() == KRss::TreeNode::TagTier )
        return;

    const shared_ptr<const KRss::FeedNode> feedNode = dynamic_pointer_cast<const KRss::FeedNode,
                                                                           const KRss::TreeNode>( treeNode );
    assert( feedNode );
    const shared_ptr<const KRss::Feed> feed = m_feedList->constFeedById( feedNode->feedId() );
    assert( feed );

    // check whether it's a virtual search feed
    if ( feed->isVirtual() )
        return;

    const shared_ptr<const KRss::NetFeed> netFeed = dynamic_pointer_cast<const KRss::NetFeed,
                                                                         const KRss::Feed>( feed );
    assert( netFeed );

    KUrl url( netFeed->htmlUrl() );
    if (url.isValid()) {
        OpenUrlRequest req( url );
        req.setOptions(OpenUrlRequest::ExternalBrowser);
        Kernel::self()->frameManager()->slotOpenUrlRequest(req);
    }
}

void Akregator::MainWidget::slotOpenSelectedArticlesInBrowser()
{
    const QList<KRss::Item> items = m_selectionController->selectedItems();

    Q_FOREACH( const KRss::Item& i, items )
        slotOpenItemInBrowser( i );
}

void Akregator::MainWidget::slotOpenItemInBrowser( const KRss::Item& item )
{
    const KUrl link( item.link() );
    if ( !link.isValid() )
        return;

    OpenUrlRequest req( link );
    req.setOptions( OpenUrlRequest::ExternalBrowser );
    Kernel::self()->frameManager()->slotOpenUrlRequest( req );
}


void Akregator::MainWidget::slotOpenSelectedArticles()
{
    const QList<KRss::Item> items = m_selectionController->selectedItems();

    Q_FOREACH( const KRss::Item& item, items )
    {
        const KUrl url( item.link() );
        if ( !url.isValid() )
          continue;

        OpenUrlRequest req( url );
        req.setOptions( OpenUrlRequest::NewTab );
        // TODO: (re-)add a setting for foreground/background
        // and use it here
        //req.setOpenInBackground( true );
        Kernel::self()->frameManager()->slotOpenUrlRequest( req );
    }

}

void Akregator::MainWidget::slotCopyLinkAddress()
{
    const KRss::Item item =  m_selectionController->currentItem();

    if(item.isNull())
       return;

    if ( KUrl( item.link() ).isValid() )
    {
        const QString link = KUrl( item.link() ).url();
        QClipboard *cb = QApplication::clipboard();
        cb->setText(link, QClipboard::Clipboard);
        // don't set url to selection as it's a no-no according to a fd.o spec
        //cb->setText(link, QClipboard::Selection);
    }
}

void Akregator::MainWidget::slotFeedUrlDropped(KUrl::List &urls, TreeNode* after, Folder* parent)
{
    Q_FOREACH ( const KUrl& i, urls )
        addFeed( i.prettyUrl(), false );
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

    const QList<KRss::Item> items = m_selectionController->selectedItems();

    QString msg;
    switch (items.count())
    {
        case 0:
            return;
        case 1:
            msg = i18n("<qt>Are you sure you want to delete article <b>%1</b>?</qt>", Qt::escape(items.first().title()));
            break;
        default:
            msg = i18np("<qt>Are you sure you want to delete the selected article?</qt>", "<qt>Are you sure you want to delete the %1 selected articles?</qt>", items.count());
    }

    if ( KMessageBox::warningContinueCancel( this,
                                             msg, i18n( "Delete Article" ),
                                             KStandardGuiItem::del(),
                                             KStandardGuiItem::cancel(),
                                             "Disable delete article confirmation" ) != KMessageBox::Continue )
        return;

    Q_FOREACH( const KRss::Item& i, items )
    {
        KRss::Item modifiedItem = i;
        modifiedItem.setStatus( i.status() | KRss::Item::Deleted );
        KRss::ItemModifyJob * const job = new KRss::ItemModifyJob();
        job->setItem( modifiedItem );
        job->setIgnorePayload( true );

        job->start();
    }

}


void Akregator::MainWidget::slotArticleToggleKeepFlag( bool )
{
    const QList<KRss::Item> items = m_selectionController->selectedItems();

    if (items.isEmpty())
        return;

    bool allFlagsSet = true;
    Q_FOREACH ( const KRss::Item& i, items )
    {
        allFlagsSet = allFlagsSet && i.isImportant();
        if ( !allFlagsSet )
            break;
    }

    Q_FOREACH ( const KRss::Item& i, items )
    {
        KRss::Item modifiedItem = i;
        if ( allFlagsSet )
            modifiedItem.setStatus( i.status() & ~KRss::Item::Important );
        else
            modifiedItem.setStatus( i.status() | KRss::Item::Important );

        KRss::ItemModifyJob * const job = new KRss::ItemModifyJob();
        job->setItem( modifiedItem );
        job->setIgnorePayload( true );
        //PENDING(frank) connect to finished signal and report errors
        job->start();
    }
}

namespace {

static void setSelectedArticleStatus( const Akregator::AbstractSelectionController* controller, Akregator::ArticleStatus status )
{
    const QList<KRss::Item> items = controller->selectedItems();

    if (items.isEmpty())
        return;

    Q_FOREACH ( const KRss::Item& i, items )
    {
        KRss::Item modifiedItem = i;
        switch ( status ) {
        case Akregator::Read:
            modifiedItem.setStatus( i.status() & ~( KRss::Item::New | KRss::Item::Unread ) );
            break;
        case Akregator::Unread:
            modifiedItem.setStatus( ( i.status() | KRss::Item::Unread ) & ~KRss::Item::New );
            break;
        case Akregator::New:
            modifiedItem.setStatus( i.status() | KRss::Item::New | KRss::Item::Unread );
            break;
        }
        KRss::ItemModifyJob * const job = new KRss::ItemModifyJob();
        job->setItem( modifiedItem );
        job->setIgnorePayload( true );
        //PENDING(frank) connect to finished signal and report errors
        job->start();
    }
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
            SpeechClient::self()->slotSpeak(m_selectionController->selectedItems());
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
    KRss::Item item =  m_selectionController->currentItem();

    if ( item.isNull() )
        return;

    item.setStatus( item.status() & ~( KRss::Item::New | KRss::Item::Unread ) );

    KRss::ItemModifyJob * const job = new KRss::ItemModifyJob();
    job->setItem( item );
    job->setIgnorePayload( true );
    job->start();
}

void Akregator::MainWidget::slotMouseOverInfo(const KFileItem& kifi)
{
    m_mainFrame->slotSetStatusText( kifi.isNull() ? QString() : kifi.url().prettyUrl() );
}

void Akregator::MainWidget::readProperties(const KConfigGroup &config)
{
    if ( !Settings::resetQuickFilterOnNodeChange() )
    {
        // read filter settings
        m_searchBar->slotSetText(config.readEntry("searchLine"));
        m_searchBar->slotSetStatus(config.readEntry("searchCombo").toInt());
    }
    // Reopen tabs
    QStringList childList = config.readEntry( QString::fromLatin1( "Children" ),
        QStringList() );
    Q_FOREACH( const QString& framePrefix, childList )
    {
        BrowserFrame* const frame = new BrowserFrame(m_tabWidget);
        frame->loadConfig( config, framePrefix + QLatin1Char( '_' ) );

        connect( m_part, SIGNAL(signalSettingsChanged()), frame, SLOT(slotPaletteOrFontChanged()));

        Kernel::self()->frameManager()->slotAddFrame(frame);

    }
}

void Akregator::MainWidget::saveProperties(KConfigGroup & config)
{
    // save filter settings
    config.writeEntry("searchLine", m_searchBar->text());
    config.writeEntry("searchCombo", m_searchBar->status());

    Kernel::self()->frameManager()->saveProperties(config);
}

void Akregator::MainWidget::slotJobFinished( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        KMessageBox::error( this, job->errorString() );
    }
}

#include "mainwidget.moc"
