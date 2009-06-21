/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <osterfeld@kde.org>

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
#include "akregatorconfig.h"
#include "akregator_part.h"
#include "articlelistview.h"
#include "articleviewer.h"
#include "framemanager.h"
#include "kernel.h"
#include "mainwidget.h"
#include "speechclient.h"
#include "tabwidget.h"
#include "trayicon.h"

#include <krss/ui/feedlistview.h>
#include <krss/treenode.h>
#include <krss/treenodevisitor.h>

#include <kactionmenu.h>
#include <ktoolbarpopupaction.h>
#include <kaction.h>
#include <KToggleAction>
#include <kactioncollection.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmenu.h>
#include <kshortcut.h>
#include <kstandardshortcut.h>
#include <kstandardaction.h>
#include <kxmlguifactory.h>
#include <kicon.h>

#include <QHash>
#include <QWidget>

#include <boost/shared_ptr.hpp>

using boost::shared_ptr;

namespace Akregator
{

class ActionManagerImpl::NodeSelectVisitor : public KRss::TreeNodeVisitor
{
    public:
    NodeSelectVisitor(ActionManagerImpl* manager) : m_manager(manager) {}

    void visit( const shared_ptr<KRss::RootNode>& rootNode )
    {
    }

    void visit( const shared_ptr<KRss::TagNode>& tagNode )
    {
        QAction* remove = m_manager->action("feed_remove");
        if (remove)
            remove->setEnabled( !tagNode->tag().isNull() );
        QAction* hp = m_manager->action("feed_homepage");
        if (hp)
            hp->setEnabled(false);

        m_manager->action("feed_fetch")->setText(i18n("&Fetch Feeds"));
        m_manager->action("feed_remove")->setText(i18n("&Delete Tag"));
        m_manager->action("feed_modify")->setText(i18n("&Modify Tag"));
        m_manager->action("feed_mark_feed_as_read")->setText(i18n("&Mark Feeds as Read"));
    }

    void visit( const shared_ptr<KRss::FeedNode>& feedNode )
    {
        QAction* remove = m_manager->action("feed_remove");
        if (remove)
            remove->setEnabled(true);
        QAction* hp = m_manager->action("feed_homepage");
        if (hp) {
            //hp->setEnabled(!node->htmlUrl().isEmpty());
        }
        m_manager->action("feed_fetch")->setText(i18n("&Fetch Feed"));
        m_manager->action("feed_remove")->setText(i18n("&Delete Feed"));
        m_manager->action("feed_modify")->setText(i18n("&Edit Feed..."));
        m_manager->action("feed_mark_feed_as_read")->setText(i18n("&Mark Feed as Read"));
    }

    private:
    ActionManagerImpl* m_manager;
};

class ActionManagerImpl::ActionManagerImplPrivate
{
public:

    NodeSelectVisitor* nodeSelectVisitor;
    ArticleListView* articleList;
    KRss::FeedListView* feedListView;
    MainWidget* mainWidget;
    ArticleViewer* articleViewer;
    Part* part;
    TrayIcon* trayIcon;
    KActionMenu* tagMenu;
    KActionCollection* actionCollection;
    TabWidget* tabWidget;
    KAction* speakSelectedArticlesAction;
    FrameManager* frameManager;
};


void ActionManagerImpl::slotNodeSelected( const shared_ptr<KRss::TreeNode>& treeNode )
{
    if ( treeNode )
        treeNode->accept( d->nodeSelectVisitor );
}

ActionManagerImpl::ActionManagerImpl(Part* part, QObject* parent ) : ActionManager(parent), d(new ActionManagerImplPrivate)
{
    d->nodeSelectVisitor = new NodeSelectVisitor(this);
    d->part = part;
    d->feedListView = 0;
    d->articleList = 0;
    d->trayIcon = 0;
    d->articleViewer = 0;
    d->mainWidget = 0;
    d->tabWidget = 0;
    d->tagMenu = 0;
    d->frameManager = 0;
    d->speakSelectedArticlesAction = 0;
    d->actionCollection = part->actionCollection();
    initPart();
}

ActionManagerImpl::~ActionManagerImpl()
{
    delete d->nodeSelectVisitor;
    delete d;
    d = 0;
}

void ActionManagerImpl::initTrayIcon(TrayIcon* trayIcon)
{
    if (d->trayIcon)
        return;
    else d->trayIcon = trayIcon;

    QMenu* traypop = trayIcon->contextMenu();

    if (actionCollection()->action("feed_fetch_all"))
        traypop->addAction(actionCollection()->action("feed_fetch_all"));
    if (actionCollection()->action("options_configure"))
        traypop->addAction(actionCollection()->action("options_configure"));
}

void ActionManagerImpl::initPart()
{
    KAction *configure = d->actionCollection->addAction("options_configure");
    configure->setText(i18n("&Configure Akregator..."));
    configure->setIcon(KIcon("configure"));
    connect(configure, SIGNAL(triggered()), d->part, SLOT(showOptions()));

    KStandardAction::configureNotifications(d->part, SLOT(showNotificationOptions()), d->actionCollection); // options_configure_notifications

    /*action = d->actionCollection->addAction("akregator_configure_akregator");
    action->setIcon(KIcon("configure"));
    action->setText(i18n("Configure &Akregator..."));
    connect(action, SIGNAL(triggered(bool)), d->part, SLOT(showOptions()));*/
}

void ActionManagerImpl::initMainWidget(MainWidget* mainWidget)
{
    if (d->mainWidget)
        return;

    d->mainWidget = mainWidget;

    KActionCollection* coll = actionCollection();

    // Feed/Feed Group popup menu
    KAction* action = coll->addAction("feed_homepage");
    action->setText(i18n("&Open Homepage"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotOpenHomepage()));
    action->setShortcuts(KShortcut( "Ctrl+H" ));

    action = coll->addAction("feed_add");
    action->setIcon(KIcon("feed-subscribe"));
    action->setText(i18n("&Add Feed..."));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotFeedAdd()));
    action->setShortcuts(KShortcut( "Insert" ));

    action = coll->addAction("tag_add");
    action->setText(i18n("Ne&w Tag..."));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotTagAdd()));
    action->setShortcuts(KShortcut( "Shift+Insert" ));

    action = coll->addAction("feed_remove");
    action->setIcon(KIcon("edit-delete"));
    action->setText(i18n("&Delete Feed"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotFeedRemove()));
    action->setShortcuts(KShortcut( "Alt+Delete" ));

    action = coll->addAction("feed_modify");
    action->setIcon(KIcon("document-properties"));
    action->setText(i18n("&Edit Feed..."));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotFeedModify()));
    action->setShortcuts(KShortcut( "F2" ));

    // toolbar / View
    action = coll->addAction("normal_view");
    action->setIcon(KIcon("view-split-top-bottom"));
    action->setText(i18n("&Normal View"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotNormalView()));
    action->setShortcuts(KShortcut( "Ctrl+Shift+1" ));

    action = coll->addAction("widescreen_view");
    action->setIcon(KIcon("view-split-left-right"));
    action->setText(i18n("&Widescreen View"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotWidescreenView()));
    action->setShortcuts(KShortcut( "Ctrl+Shift+2" ));

    action = coll->addAction("combined_view");
    action->setIcon(KIcon("view-list-text"));
    action->setText(i18n("C&ombined View"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotCombinedView()));
    action->setShortcuts(KShortcut( "Ctrl+Shift+3" ));

    // toolbar / feed menu
    action = coll->addAction("feed_fetch");
    action->setIcon(KIcon("go-down"));
    action->setText(i18n("&Fetch Feed"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotFetchCurrentFeed()));
    action->setShortcuts(KStandardShortcut::shortcut(KStandardShortcut::Reload));

    action = coll->addAction("feed_fetch_all");
    action->setIcon(KIcon("go-bottom"));
    action->setText(i18n("Fe&tch All Feeds"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotFetchAllFeeds()));
    action->setShortcuts(KShortcut( "Ctrl+L" ));

    KAction *stopAction = coll->addAction("feed_stop");
    stopAction->setIcon(KIcon("process-stop"));
    stopAction->setText(i18n("&Abort Fetches"));
#ifdef KRSS_PORT_DISABLED
    connect(stopAction, SIGNAL(triggered(bool)), Kernel::self()->fetchQueue(), SLOT(slotAbort()));
#else
    kWarning() << "Code temporarily disabled (Akonadi port)";
#endif //KRSS_PORT_DISABLED    stopAction->setShortcut(QKeySequence(Qt::Key_Escape));
    stopAction->setEnabled(false);

    action = coll->addAction("feed_mark_feed_as_read");
    action->setIcon(KIcon("mail-mark-read"));
    action->setText(i18n("&Mark Feed as Read"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotMarkFeedRead()));
    action->setShortcuts(KShortcut( "Ctrl+R" ));

    action = coll->addAction("feed_mark_all_feeds_as_read");
    action->setIcon(KIcon("mail-mark-read"));
    action->setText(i18n("Ma&rk All Feeds as Read"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotMarkAllFeedsRead()));
    action->setShortcuts(KShortcut( "Ctrl+Shift+R" ));

    // Settings menu
    KToggleAction *sqf = coll->add<KToggleAction>("show_quick_filter");
    sqf->setText(i18n("Show Quick Filter"));
    connect(sqf, SIGNAL(triggered(bool) ), d->mainWidget, SLOT(slotToggleShowQuickFilter()));
    sqf->setChecked( Settings::showQuickFilter() );

    action = coll->addAction("article_open" );
    action->setIcon(KIcon("tab-new"));
    action->setText(i18n("Open in Tab"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotOpenSelectedArticles()));
    action->setShortcuts(KShortcut( "Shift+Return" ));

    action = coll->addAction("article_open_external" );
    action->setIcon(KIcon("window-new"));
    action->setText(i18n("Open in External Browser"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotOpenSelectedArticlesInBrowser()));
    action->setShortcuts(KShortcut( "Ctrl+Shift+Return" ));

    action = coll->addAction("article_copy_link_address" );
    action->setText(i18n("Copy Link Address"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotCopyLinkAddress()));

    action = coll->addAction("go_prev_unread_article");
    action->setIcon(KIcon("go-previous"));
    action->setText(i18n("Pre&vious Unread Article"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotPrevUnreadArticle()));
    action->setShortcut(QKeySequence(Qt::Key_Minus));

    action = coll->addAction("go_next_unread_article");
    action->setIcon(KIcon("go-next"));
    action->setText(i18n("Ne&xt Unread Article"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotNextUnreadArticle()));
    action->setShortcuts(KShortcut(Qt::Key_Plus, Qt::Key_Equal));

    action = coll->addAction("article_delete");
    action->setIcon(KIcon("edit-delete"));
    action->setText(i18n("&Delete"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotArticleDelete()));
    action->setShortcuts(KShortcut( "Delete" ));

    KActionMenu* statusMenu = coll->add<KActionMenu>("article_set_status");
    statusMenu->setText(i18n("&Mark As"));
    statusMenu->setEnabled( false );

    d->speakSelectedArticlesAction = coll->addAction("akr_texttospeech");
    d->speakSelectedArticlesAction->setIcon(KIcon("media-playback-start"));
    d->speakSelectedArticlesAction->setText(i18n("&Speak Selected Articles"));
    connect(d->speakSelectedArticlesAction, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotTextToSpeechRequest()));


    action = coll->addAction("akr_aborttexttospeech");
    action->setText(i18n( "&Stop Speaking" ));
    action->setIcon(KIcon("media-playback-stop"));
    connect(action, SIGNAL(triggered(bool)),SpeechClient::self(), SLOT(slotAbortJobs()));
    //action->setShortcuts(Qt::Key_Escape);
    action->setEnabled(false);

    connect(SpeechClient::self(), SIGNAL(signalActivated(bool)), action, SLOT(setEnabled(bool)));

    action = coll->addAction("article_set_status_read");
    action->setText(i18nc("as in: mark as read","&Read"));
    action->setIcon(KIcon("mail-mark-read"));
    action->setToolTip(i18n("Mark selected article as read"));
    action->setShortcuts(KShortcut( "Ctrl+E" ));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotSetSelectedArticleRead()));
    statusMenu->addAction(action);

    action = coll->addAction("article_set_status_new");
    action->setText(i18nc("as in: mark as new", "&New"));
    action->setIcon(KIcon("mail-mark-unread-new"));
    action->setShortcuts(KShortcut( "Ctrl+N" ));
    action->setToolTip(i18n("Mark selected article as new"));
    connect(action, SIGNAL(triggered(bool)),  d->mainWidget, SLOT(slotSetSelectedArticleNew()));
    statusMenu->addAction(action);

    action = coll->addAction("article_set_status_unread");
    action->setText(i18nc("as in: mark as unread", "&Unread"));
    action->setIcon(KIcon("mail-mark-unread"));
    action->setToolTip(i18n("Mark selected article as unread"));
    action->setShortcuts(KShortcut( "Ctrl+U" ));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotSetSelectedArticleUnread()));
    statusMenu->addAction(action);

    KToggleAction* importantAction = coll->add<KToggleAction>("article_set_status_important");
    importantAction->setText(i18n("&Mark as Important"));
    importantAction->setIcon(KIcon("mail-mark-important"));
    importantAction->setShortcuts(KShortcut("Ctrl+I"));
    importantAction->setCheckedState(KGuiItem(i18n("Remove &Important Mark")));
    connect(importantAction, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotArticleToggleKeepFlag(bool)));

    action = coll->addAction("feedstree_move_up");
    action->setText(i18n("Move Node Up"));
    connect(action, SIGNAL(triggered(bool)), mainWidget, SLOT(slotMoveCurrentNodeUp()));
    action->setShortcuts(KShortcut( "Shift+Alt+Up" ));

    action = coll->addAction("feedstree_move_down");
    action->setText(i18n("Move Node Down"));
    connect(action, SIGNAL(triggered(bool)), mainWidget, SLOT(slotMoveCurrentNodeDown()));
    action->setShortcuts(KShortcut( "Shift+Alt+Down" ));

    action = coll->addAction("file_sendlink");
    action->setIcon(KIcon("mail-message-new"));
    action->setText(i18n("Send &Link Address..."));
    connect(action, SIGNAL(triggered(bool)), mainWidget, SLOT(slotSendLink()));

    action = coll->addAction("file_sendfile");
    action->setIcon(KIcon("mail-message-new"));
    action->setText(i18n("Send &File..."));
    connect(action, SIGNAL(triggered(bool)), mainWidget, SLOT(slotSendFile()));

    action = coll->addAction("file_import");
    action->setText(i18n("&Import Feeds..."));
    action->setIcon(KIcon("document-import"));
    connect(action, SIGNAL(triggered(bool)), mainWidget, SLOT(slotImportFeedList()));

    action = coll->addAction("file_export");
    action->setText(i18n("&Export Feeds..." ));
    action->setIcon(KIcon("document-export"));
    connect(action, SIGNAL(triggered(bool)), mainWidget, SLOT(slotExportFeedList()));

    action = coll->addAction("file_data_migration");
    action->setText(i18n("&Import from Previous Version..."));
    action->setIcon(KIcon("document-import"));
    connect(action, SIGNAL(triggered(bool)), mainWidget, SLOT(slotMetakitImport()));

    setArticleActionsEnabled( false );
}

void ActionManagerImpl::initArticleViewer(ArticleViewer* articleViewer)
{
    if (d->articleViewer)
        return;
    else
        d->articleViewer = articleViewer;
}

void ActionManagerImpl::initArticleListView(ArticleListView* articleList)
{
    if (d->articleList)
        return;
    else
        d->articleList = articleList;

    KAction *action = actionCollection()->addAction("go_previous_article");
    action->setText(i18n("&Previous Article"));
    connect(action, SIGNAL(triggered(bool)), articleList, SLOT(slotPreviousArticle()));
    action->setShortcuts(KShortcut( "Left" ));
    action = actionCollection()->addAction("go_next_article");
    action->setText(i18n("&Next Article"));
    connect(action, SIGNAL(triggered(bool)), articleList, SLOT(slotNextArticle()));
    action->setShortcuts(KShortcut( "Right" ));
}

void ActionManagerImpl::initFeedListView( KRss::FeedListView* feedListView )
{
    if ( d->feedListView )
        return;
    else
        d->feedListView = feedListView;

    KActionCollection *coll = actionCollection();

    KAction *action = coll->addAction("go_prev_feed");
    action->setText(i18n("&Previous Feed"));
    connect(action, SIGNAL(triggered(bool)), feedListView, SLOT(slotPrevFeed()));
    action->setShortcuts(KShortcut( "P" ));

    action = coll->addAction("go_next_feed");
    action->setText(i18n("&Next Feed"));
    connect(action, SIGNAL(triggered(bool)), feedListView, SLOT(slotNextFeed()));
    action->setShortcuts(KShortcut( "N" ));

    action = coll->addAction("go_next_unread_feed");
    action->setIcon(KIcon("go-down"));
    action->setText(i18n("N&ext Unread Feed"));
    connect(action, SIGNAL(triggered(bool)), feedListView, SLOT(slotNextUnreadFeed()));
    action->setShortcut(  QKeySequence(Qt::ALT+Qt::Key_Plus) );

    action = coll->addAction("go_prev_unread_feed");
    action->setIcon(KIcon("go-up"));
    action->setText(i18n("Prev&ious Unread Feed"));
    connect(action, SIGNAL(triggered(bool)), feedListView, SLOT(slotPrevUnreadFeed()));
    action->setShortcut( QKeySequence(Qt::ALT+Qt::Key_Minus) );

#ifdef KRSS_PORT_DISABLED // those slots have never been implemented in Akregator
    action = coll->addAction("feedstree_home");
    action->setText(i18n("Go to Top of Tree"));
    connect(action, SIGNAL(triggered(bool)), feedListView, SLOT(slotItemBegin()));
    action->setShortcuts(KShortcut( "Ctrl+Home" ));

    action = coll->addAction("feedstree_end");
    action->setText(i18n("Go to Bottom of Tree"));
    connect(action, SIGNAL(triggered(bool)), feedListView, SLOT(slotItemEnd()));
    action->setShortcuts(KShortcut( "Ctrl+End" ));

    action = coll->addAction("feedstree_left");
    action->setText(i18n("Go Left in Tree"));
    connect(action, SIGNAL(triggered(bool)), feedListView, SLOT(slotItemLeft()));
    action->setShortcuts(KShortcut( "Ctrl+Left" ));

    action = coll->addAction("feedstree_right");
    action->setText(i18n("Go Right in Tree"));
    connect(action, SIGNAL(triggered(bool)), feedListView, SLOT(slotItemRight()));
    action->setShortcuts(KShortcut( "Ctrl+Right" ));

    action = coll->addAction("feedstree_up");
    action->setText(i18n("Go Up in Tree"));
    connect(action, SIGNAL(triggered(bool)), feedListView, SLOT(slotItemUp()));
    action->setShortcuts(KShortcut( "Ctrl+Up" ));

    action = coll->addAction("feedstree_down" );
    action->setText(i18n("Go Down in Tree"));
    connect(action, SIGNAL(triggered(bool)), feedListView, SLOT(slotItemDown()));
    action->setShortcuts(KShortcut( "Ctrl+Down" ));
#endif
}

void ActionManagerImpl::initTabWidget(TabWidget* tabWidget)
{
    if (d->tabWidget)
        return;
    else
        d->tabWidget = tabWidget;

    KActionCollection *coll = actionCollection();

    KAction *action = coll->addAction("select_next_tab");
    action->setText(i18n("Select Next Tab"));
    connect(action, SIGNAL(triggered(bool)), d->tabWidget, SLOT(slotNextTab()));
    action->setShortcuts(KShortcut( "Ctrl+Period" ));

    action = coll->addAction("select_previous_tab");
    action->setText(i18n("Select Previous Tab"));
    connect(action, SIGNAL(triggered(bool)), d->tabWidget, SLOT(slotPreviousTab()));
    action->setShortcuts(KShortcut( "Ctrl+Comma" ));

    action = coll->addAction("tab_detach");
    action->setIcon(KIcon("tab-detach"));
    action->setText(i18n("Detach Tab"));
    connect(action, SIGNAL(triggered(bool)), d->tabWidget, SLOT(slotDetachTab()));
    action->setShortcut(QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_B));

    action = coll->addAction("tab_copylinkaddress");
    action->setText(i18n("Copy Link Address"));
    connect(action, SIGNAL(triggered(bool)), d->tabWidget, SLOT(slotCopyLinkAddress()));

    action = coll->addAction("tab_remove");
    action->setIcon(KIcon("tab-close"));
    action->setText(i18n("&Close Tab"));
    connect(action, SIGNAL(triggered(bool)), d->tabWidget, SLOT(slotCloseTab()));
    action->setShortcuts(KStandardShortcut::close());
}

void ActionManagerImpl::initFrameManager(FrameManager* frameManager)
{
    if (d->frameManager)
        return;

    d->frameManager = frameManager;

    KToolBarPopupAction* forward = new KToolBarPopupAction(KIcon("go-next"), i18nc("Go forward in browser history", "Forward"), this);
    d->actionCollection->addAction("browser_forward", forward);
    forward->setShortcuts(KShortcut("Alt+Right"));
    connect(forward, SIGNAL(triggered()), frameManager, SLOT(slotBrowserForward()));

    connect(forward->menu(), SIGNAL(aboutToShow()), frameManager, SLOT(slotBrowserForwardAboutToShow()));

    KToolBarPopupAction* back = new KToolBarPopupAction(KIcon("go-previous"), i18nc("Go back in browser history", "Back"), this);
    d->actionCollection->addAction("browser_back", back);
    back->setShortcuts(KShortcut("Alt+Left"));
    connect(back, SIGNAL(triggered()), frameManager, SLOT(slotBrowserBack()));

    connect(back->menu(), SIGNAL(aboutToShow()), frameManager, SLOT(slotBrowserBackAboutToShow()));

    KAction *action = d->actionCollection->addAction("browser_reload");
    action->setIcon(KIcon("view-refresh"));
    action->setText(i18nc("Reload current page", "Reload"));
    connect(action, SIGNAL(triggered(bool) ), frameManager, SLOT(slotBrowserReload()));

    action = d->actionCollection->addAction("browser_stop");
    action->setIcon(KIcon("process-stop"));
    action->setText(i18n("Stop"));
    connect(action, SIGNAL(triggered(bool) ), frameManager, SLOT(slotBrowserStop()));
}

QWidget* ActionManagerImpl::container(const char* name)
{
    return d->part->factory()->container(name, d->part);
}


KActionCollection* ActionManagerImpl::actionCollection()
{
    return d->actionCollection;
}

QAction* ActionManagerImpl::action(const char* name)
{
    return d->actionCollection != 0 ? d->actionCollection->action(name) : 0;
}

void ActionManagerImpl::setArticleActionsEnabled( bool enabled ) {
#undef setActionEnabled
#define setActionEnabled(name) { QAction* const a = action( name ); if ( a ) a->setEnabled( enabled ); }
    setActionEnabled("article_open")
    setActionEnabled("article_open_external")
    setActionEnabled("article_set_status_important")
    setActionEnabled("article_set_status")
    setActionEnabled("article_delete")
    setActionEnabled("file_sendlink")
    setActionEnabled("file_sendfile")
#undef setActionEnabled
}

} // namespace Akregator

#include "actionmanagerimpl.moc"
