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
#include "feed.h"
#include "fetchqueue.h"
#include "folder.h"
#include "framemanager.h"
#include "kernel.h"
#include "mainwidget.h"
//QT5 #include "speechclient.h"
#include "subscriptionlistview.h"
#include "tabwidget.h"
#include "trayicon.h"
#include "treenode.h"
#include "treenodevisitor.h"

#include <kactionmenu.h>
#include <ktoolbarpopupaction.h>
#include <QAction>
#include <KToggleAction>
#include <kactioncollection.h>
#include "akregator_debug.h"
#include <KLocalizedString>
#include <QMenu>
#include <QKeySequence>
#include <kstandardshortcut.h>
#include <kstandardaction.h>
#include <kxmlguifactory.h>
#include <KGuiItem>

#include <QApplication>
#include <QHash>
#include <QWidget>

namespace Akregator
{

class ActionManagerImpl::NodeSelectVisitor : public TreeNodeVisitor
{
public:
    NodeSelectVisitor(ActionManagerImpl *manager) : m_manager(manager) {}

    bool visitFeed(Feed *node) Q_DECL_OVERRIDE
    {
        QAction *remove = m_manager->action("feed_remove");
        if (remove) {
            remove->setEnabled(true);
        }
        QAction *hp = m_manager->action("feed_homepage");
        if (hp) {
            hp->setEnabled(!node->htmlUrl().isEmpty());
        }
        m_manager->action("feed_fetch")->setText(i18n("&Fetch Feed"));
        m_manager->action("feed_remove")->setText(i18n("&Delete Feed"));
        m_manager->action("feed_modify")->setText(i18n("&Edit Feed..."));
        m_manager->action("feed_mark_all_as_read")->setText(i18n("&Mark Feed as Read"));

        return true;
    }

    bool visitFolder(Folder *node) Q_DECL_OVERRIDE
    {
        QAction *remove = m_manager->action("feed_remove");
        if (remove) {
            remove->setEnabled(node->parent());    // root nodes must not be deleted
        }
        QAction *hp = m_manager->action("feed_homepage");
        if (hp) {
            hp->setEnabled(false);
        }

        m_manager->action("feed_fetch")->setText(i18n("&Fetch Feeds"));
        m_manager->action("feed_remove")->setText(i18n("&Delete Folder"));
        m_manager->action("feed_modify")->setText(i18n("&Rename Folder"));
        m_manager->action("feed_mark_all_as_read")->setText(i18n("&Mark Feeds as Read"));

        return true;
    }

private:
    ActionManagerImpl *m_manager;
};

class ActionManagerImpl::ActionManagerImplPrivate
{
public:

    NodeSelectVisitor *nodeSelectVisitor;
    ArticleListView *articleList;
    SubscriptionListView *subscriptionListView;
    MainWidget *mainWidget;
    ArticleViewer *articleViewer;
    Part *part;
    TrayIcon *trayIcon;
    KActionMenu *tagMenu;
    KActionCollection *actionCollection;
    TabWidget *tabWidget;
    QAction *speakSelectedArticlesAction;
    FrameManager *frameManager;
};

void ActionManagerImpl::slotNodeSelected(TreeNode *node)
{
    if (node != 0) {
        d->nodeSelectVisitor->visit(node);
    }
}

ActionManagerImpl::ActionManagerImpl(Part *part, QObject *parent) : ActionManager(parent), d(new ActionManagerImplPrivate)
{
    d->nodeSelectVisitor = new NodeSelectVisitor(this);
    d->part = part;
    d->subscriptionListView = 0;
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

void ActionManagerImpl::setTrayIcon(TrayIcon *trayIcon)
{
    if (trayIcon == 0) {
        d->trayIcon = 0;
        return;
    }
    if (d->trayIcon) {
        return;
    } else {
        d->trayIcon = trayIcon;
    }

    QMenu *traypop = trayIcon->contextMenu();

    if (actionCollection()->action("feed_fetch_all")) {
        traypop->addAction(actionCollection()->action("feed_fetch_all"));
    }
    if (actionCollection()->action("options_configure")) {
        traypop->addAction(actionCollection()->action("options_configure"));
    }
}

void ActionManagerImpl::initPart()
{
    QAction *action = d->actionCollection->addAction("file_import");
    action->setText(i18n("&Import Feeds..."));
    action->setIcon(QIcon::fromTheme("document-import"));
    connect(action, SIGNAL(triggered(bool)), d->part, SLOT(fileImport()));
    action = d->actionCollection->addAction("file_export");
    action->setText(i18n("&Export Feeds..."));
    action->setIcon(QIcon::fromTheme("document-export"));
    connect(action, SIGNAL(triggered(bool)), d->part, SLOT(fileExport()));

    QAction *configure = d->actionCollection->addAction("options_configure");
    configure->setText(i18n("&Configure Akregator..."));
    configure->setIcon(QIcon::fromTheme("configure"));
    connect(configure, SIGNAL(triggered()), d->part, SLOT(showOptions()));

    KStandardAction::configureNotifications(d->part, SLOT(showNotificationOptions()), d->actionCollection); // options_configure_notifications

    /*action = d->actionCollection->addAction("akregator_configure_akregator");
    action->setIcon(QIcon::fromTheme("configure"));
    action->setText(i18n("Configure &Akregator..."));
    connect(action, SIGNAL(triggered(bool)), d->part, SLOT(showOptions()));*/
}

void ActionManagerImpl::initMainWidget(MainWidget *mainWidget)
{
    if (d->mainWidget) {
        return;
    }

    d->mainWidget = mainWidget;

    KActionCollection *coll = actionCollection();

    // Feed/Feed Group popup menu
    QAction *action = coll->addAction("feed_homepage");
    action->setText(i18n("&Open Homepage"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotOpenHomepage);
    coll->setDefaultShortcut(action, QKeySequence("Ctrl+H"));

    action = coll->addAction("reload_all_tabs");
    action->setIcon(QIcon::fromTheme("view-refresh"));
    action->setText(i18n("Reload All Tabs"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotReloadAllTabs);
    coll->setDefaultShortcut(action, QKeySequence("Shift+F5"));

    action = coll->addAction("feed_add");
    action->setIcon(QIcon::fromTheme("feed-subscribe"));
    action->setText(i18n("&Add Feed..."));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotFeedAdd()));
    coll->setDefaultShortcut(action, QKeySequence("Insert"));

    action = coll->addAction("feed_add_group");
    action->setIcon(QIcon::fromTheme("folder-new"));
    action->setText(i18n("Ne&w Folder..."));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotFeedAddGroup);
    coll->setDefaultShortcut(action, QKeySequence("Shift+Insert"));

    action = coll->addAction("feed_remove");
    action->setIcon(QIcon::fromTheme("edit-delete"));
    action->setText(i18n("&Delete Feed"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotFeedRemove);
    coll->setDefaultShortcut(action, QKeySequence("Alt+Delete"));

    action = coll->addAction("feed_modify");
    action->setIcon(QIcon::fromTheme("document-properties"));
    action->setText(i18n("&Edit Feed..."));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotFeedModify);
    coll->setDefaultShortcut(action, QKeySequence("F2"));

    // toolbar / View
    action = coll->addAction("normal_view");
    action->setIcon(QIcon::fromTheme("view-split-top-bottom"));
    action->setText(i18n("&Normal View"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotNormalView);
    coll->setDefaultShortcut(action, QKeySequence("Ctrl+Shift+1"));

    action = coll->addAction("widescreen_view");
    action->setIcon(QIcon::fromTheme("view-split-left-right"));
    action->setText(i18n("&Widescreen View"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotWidescreenView()));
    coll->setDefaultShortcut(action, QKeySequence("Ctrl+Shift+2"));

    action = coll->addAction("combined_view");
    action->setIcon(QIcon::fromTheme("view-list-text"));
    action->setText(i18n("C&ombined View"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotCombinedView()));
    coll->setDefaultShortcut(action, QKeySequence("Ctrl+Shift+3"));

    // toolbar / feed menu
    action = coll->addAction("feed_fetch");
    action->setIcon(QIcon::fromTheme("go-down"));
    action->setText(i18n("&Fetch Feed"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotFetchCurrentFeed()));
    coll->setDefaultShortcuts(action, KStandardShortcut::shortcut(KStandardShortcut::Reload));

    action = coll->addAction("feed_fetch_all");
    action->setIcon(QIcon::fromTheme("go-bottom"));
    action->setText(i18n("Fe&tch All Feeds"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotFetchAllFeeds()));
    coll->setDefaultShortcut(action, QKeySequence("Ctrl+L"));

    QAction *stopAction = coll->addAction("feed_stop");
    stopAction->setIcon(QIcon::fromTheme("process-stop"));
    stopAction->setText(i18n("C&ancel Feed Fetches"));
    connect(stopAction, SIGNAL(triggered(bool)), Kernel::self()->fetchQueue(), SLOT(slotAbort()));
    coll->setDefaultShortcut(stopAction, QKeySequence(Qt::Key_Escape));
    stopAction->setEnabled(false);

    action = coll->addAction("feed_mark_all_as_read");
    action->setIcon(QIcon::fromTheme("mail-mark-read"));
    action->setText(i18n("&Mark Feed as Read"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotMarkAllRead()));
    coll->setDefaultShortcut(action, QKeySequence("Ctrl+R"));

    action = coll->addAction("feed_mark_all_feeds_as_read");
    action->setIcon(QIcon::fromTheme("mail-mark-read"));
    action->setText(i18n("Ma&rk All Feeds as Read"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotMarkAllFeedsRead()));
    coll->setDefaultShortcut(action, QKeySequence("Ctrl+Shift+R"));

    // Settings menu
    KToggleAction *sqf = coll->add<KToggleAction>("show_quick_filter");
    sqf->setText(i18n("Show Quick Filter"));
    connect(sqf, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotToggleShowQuickFilter()));
    sqf->setChecked(Settings::showQuickFilter());

    action = coll->addAction("article_open");
    action->setIcon(QIcon::fromTheme("tab-new"));
    action->setText(i18n("Open in Tab"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotOpenSelectedArticles()));
    coll->setDefaultShortcut(action, QKeySequence("Shift+Return"));

    action = coll->addAction("article_open_in_background");
    action->setIcon(QIcon::fromTheme("tab-new"));
    action->setText(i18n("Open in Background Tab"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotOpenSelectedArticlesInBackground()));
    coll->setDefaultShortcut(action, QKeySequence("Return"));

    action = coll->addAction("article_open_external");
    action->setIcon(QIcon::fromTheme("window-new"));
    action->setText(i18n("Open in External Browser"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotOpenSelectedArticlesInBrowser()));
    coll->setDefaultShortcut(action, QKeySequence("Ctrl+Shift+Return"));

    action = coll->addAction("article_copy_link_address");
    action->setText(i18n("Copy Link Address"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotCopyLinkAddress()));

    action = coll->addAction("go_prev_unread_article");
    action->setIcon(QIcon::fromTheme("go-previous"));
    action->setText(i18n("Pre&vious Unread Article"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotPrevUnreadArticle()));
    coll->setDefaultShortcut(action, QKeySequence(Qt::Key_Minus));

    action = coll->addAction("go_next_unread_article");
    action->setIcon(QIcon::fromTheme("go-next"));
    action->setText(i18n("Ne&xt Unread Article"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotNextUnreadArticle()));
    coll->setDefaultShortcut(action, QKeySequence(Qt::Key_Plus, Qt::Key_Equal));

    action = coll->addAction("article_delete");
    action->setIcon(QIcon::fromTheme("edit-delete"));
    action->setText(i18n("&Delete"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotArticleDelete()));
    coll->setDefaultShortcut(action, QKeySequence("Delete"));

    KActionMenu *statusMenu = coll->add<KActionMenu>("article_set_status");
    statusMenu->setText(i18n("&Mark As"));
    statusMenu->setEnabled(false);

    d->speakSelectedArticlesAction = coll->addAction("akr_texttospeech");
    d->speakSelectedArticlesAction->setIcon(QIcon::fromTheme("media-playback-start"));
    d->speakSelectedArticlesAction->setText(i18n("&Speak Selected Articles"));
    connect(d->speakSelectedArticlesAction, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotTextToSpeechRequest()));

    action = coll->addAction("akr_aborttexttospeech");
    action->setText(i18n("&Stop Speaking"));
    action->setIcon(QIcon::fromTheme("media-playback-stop"));
    //QT5 connect(action, SIGNAL(triggered(bool)),SpeechClient::self(), SLOT(slotAbortJobs()));
    //action->setShortcuts(Qt::Key_Escape);
    action->setEnabled(false);

    //QT5 connect(SpeechClient::self(), SIGNAL(signalActivated(bool)), action, SLOT(setEnabled(bool)));

    action = coll->addAction("article_set_status_read");
    action->setText(i18nc("as in: mark as read", "&Read"));
    action->setIcon(QIcon::fromTheme("mail-mark-read"));
    action->setToolTip(i18n("Mark selected article as read"));
    coll->setDefaultShortcut(action, QKeySequence("Ctrl+E"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotSetSelectedArticleRead()));
    statusMenu->addAction(action);

    action = coll->addAction("article_set_status_new");
    action->setText(i18nc("as in: mark as new", "&New"));
    action->setIcon(QIcon::fromTheme("mail-mark-unread-new"));
    coll->setDefaultShortcut(action, QKeySequence("Ctrl+N"));
    action->setToolTip(i18n("Mark selected article as new"));
    connect(action, SIGNAL(triggered(bool)),  d->mainWidget, SLOT(slotSetSelectedArticleNew()));
    statusMenu->addAction(action);

    action = coll->addAction("article_set_status_unread");
    action->setText(i18nc("as in: mark as unread", "&Unread"));
    action->setIcon(QIcon::fromTheme("mail-mark-unread"));
    action->setToolTip(i18n("Mark selected article as unread"));
    coll->setDefaultShortcut(action, QKeySequence("Ctrl+U"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotSetSelectedArticleUnread()));
    statusMenu->addAction(action);

    KToggleAction *importantAction = coll->add<KToggleAction>("article_set_status_important");
    importantAction->setText(i18n("&Mark as Important"));
    importantAction->setIcon(QIcon::fromTheme("mail-mark-important"));
    QList<QKeySequence> importantSC;
    importantSC << QKeySequence("Ctrl+I");
    importantSC << QKeySequence(Qt::Key_I);
    coll->setDefaultShortcuts(importantAction, importantSC);
    importantAction->setCheckedState(KGuiItem(i18n("Remove &Important Mark")));
    connect(importantAction, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotArticleToggleKeepFlag(bool)));

    action = coll->addAction("feedstree_move_up");
    action->setText(i18n("Move Node Up"));
    connect(action, &QAction::triggered, mainWidget, &MainWidget::slotMoveCurrentNodeUp);
    coll->setDefaultShortcut(action, QKeySequence("Shift+Alt+Up"));

    action = coll->addAction("feedstree_move_down");
    action->setText(i18n("Move Node Down"));
    connect(action, &QAction::triggered, mainWidget, &MainWidget::slotMoveCurrentNodeDown);
    coll->setDefaultShortcut(action, QKeySequence("Shift+Alt+Down"));

    action = coll->addAction(i18n("Move Node Left"));
    action->setText(i18n("Move Node Left"));
    connect(action, &QAction::triggered, mainWidget, &MainWidget::slotMoveCurrentNodeLeft);
    coll->setDefaultShortcut(action, QKeySequence("Shift+Alt+Left"));

    action = coll->addAction("feedstree_move_right");
    action->setText(i18n("Move Node Right"));
    connect(action, &QAction::triggered, mainWidget, &MainWidget::slotMoveCurrentNodeRight);
    coll->setDefaultShortcut(action, QKeySequence("Shift+Alt+Right"));

    action = coll->addAction("file_sendlink");
    action->setIcon(QIcon::fromTheme("mail-message-new"));
    action->setText(i18n("Send &Link Address..."));
    connect(action, &QAction::triggered, mainWidget, &MainWidget::slotSendLink);

    action = coll->addAction("file_sendfile");
    action->setIcon(QIcon::fromTheme("mail-message-new"));
    action->setText(i18n("Send &File..."));
    connect(action, &QAction::triggered, mainWidget, &MainWidget::slotSendFile);

    setArticleActionsEnabled(false);
}

void ActionManagerImpl::initArticleViewer(ArticleViewer *articleViewer)
{
    if (d->articleViewer) {
        return;
    } else {
        d->articleViewer = articleViewer;
    }

    KActionCollection *coll = actionCollection();
    QAction *action = 0;

    action = KStandardAction::print(articleViewer, SLOT(slotPrint()), actionCollection());
    coll->addAction("viewer_print", action);

    action = KStandardAction::copy(articleViewer, SLOT(slotCopy()), coll);
    coll->addAction("viewer_copy", action);

    connect(d->tabWidget, SIGNAL(signalZoomInFrame(int)), d->articleViewer, SLOT(slotZoomIn(int)));
    connect(d->tabWidget, SIGNAL(signalZoomOutFrame(int)), d->articleViewer, SLOT(slotZoomOut(int)));
}

void ActionManagerImpl::initArticleListView(ArticleListView *articleList)
{
    if (d->articleList) {
        return;
    } else {
        d->articleList = articleList;
    }

    QAction *action = actionCollection()->addAction("go_previous_article");
    action->setText(i18n("&Previous Article"));
    connect(action, &QAction::triggered, articleList, &ArticleListView::slotPreviousArticle);
    actionCollection()->setDefaultShortcut(action, QKeySequence("Left"));
    action = actionCollection()->addAction("go_next_article");
    action->setText(i18n("&Next Article"));
    connect(action, &QAction::triggered, articleList, &ArticleListView::slotNextArticle);
    actionCollection()->setDefaultShortcut(action, QKeySequence("Right"));
}

void ActionManagerImpl::initSubscriptionListView(SubscriptionListView *subscriptionListView)
{
    if (d->subscriptionListView) {
        return;
    } else {
        d->subscriptionListView = subscriptionListView;
    }

    KActionCollection *coll = actionCollection();

    QAction *action = coll->addAction("go_prev_feed");
    action->setText(i18n("&Previous Feed"));
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotPrevFeed);
    coll->setDefaultShortcut(action, QKeySequence("P"));

    action = coll->addAction("go_next_feed");
    action->setText(i18n("&Next Feed"));
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotNextFeed);
    coll->setDefaultShortcut(action, QKeySequence("N"));

    action = coll->addAction("go_next_unread_feed");
    action->setIcon(QIcon::fromTheme("go-down"));
    action->setText(i18n("N&ext Unread Feed"));
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotNextUnreadFeed);
    coll->setDefaultShortcut(action, QKeySequence(Qt::ALT + Qt::Key_Plus));

    action = coll->addAction("go_prev_unread_feed");
    action->setIcon(QIcon::fromTheme("go-up"));
    action->setText(i18n("Prev&ious Unread Feed"));
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotPrevUnreadFeed);
    coll->setDefaultShortcut(action, QKeySequence(Qt::ALT + Qt::Key_Minus));

    action = coll->addAction("feedstree_home");
    action->setText(i18n("Go to Top of Tree"));
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotItemBegin);
    coll->setDefaultShortcut(action, QKeySequence("Ctrl+Home"));

    action = coll->addAction("feedstree_end");
    action->setText(i18n("Go to Bottom of Tree"));
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotItemEnd);
    coll->setDefaultShortcut(action, QKeySequence("Ctrl+End"));

    action = coll->addAction("feedstree_left");
    action->setText(i18n("Go Left in Tree"));
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotItemLeft);
    coll->setDefaultShortcut(action, QKeySequence("Ctrl+Left"));

    action = coll->addAction("feedstree_right");
    action->setText(i18n("Go Right in Tree"));
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotItemRight);
    coll->setDefaultShortcut(action, QKeySequence("Ctrl+Right"));

    action = coll->addAction("feedstree_up");
    action->setText(i18n("Go Up in Tree"));
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotItemUp);
    coll->setDefaultShortcut(action, QKeySequence("Ctrl+Up"));

    action = coll->addAction("feedstree_down");
    action->setText(i18n("Go Down in Tree"));
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotItemDown);
    coll->setDefaultShortcut(action, QKeySequence("Ctrl+Down"));
}

void ActionManagerImpl::initTabWidget(TabWidget *tabWidget)
{
    if (d->tabWidget) {
        return;
    } else {
        d->tabWidget = tabWidget;
    }

    KActionCollection *coll = actionCollection();

    QAction *action = coll->addAction("select_next_tab");
    action->setText(i18n("Select Next Tab"));
    connect(action, SIGNAL(triggered(bool)), d->tabWidget, SLOT(slotNextTab()));
    coll->setDefaultShortcut(action, QKeySequence("Ctrl+Period"));

    action = coll->addAction("select_previous_tab");
    action->setText(i18n("Select Previous Tab"));
    connect(action, SIGNAL(triggered(bool)), d->tabWidget, SLOT(slotPreviousTab()));
    coll->setDefaultShortcut(action, QKeySequence("Ctrl+Comma"));

    action = coll->addAction("tab_detach");
    action->setIcon(QIcon::fromTheme("tab-detach"));
    action->setText(i18n("Detach Tab"));
    connect(action, SIGNAL(triggered(bool)), d->tabWidget, SLOT(slotDetachTab()));
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_B));

    action = coll->addAction("tab_copylinkaddress");
    action->setText(i18n("Copy Link Address"));
    connect(action, SIGNAL(triggered(bool)), d->tabWidget, SLOT(slotCopyLinkAddress()));

    action = coll->addAction("tab_remove");
    action->setIcon(QIcon::fromTheme("tab-close"));
    action->setText(i18n("Close Tab"));
    connect(action, SIGNAL(triggered(bool)), d->tabWidget, SLOT(slotCloseTab()));
    coll->setDefaultShortcuts(action, KStandardShortcut::close());

    action = coll->addAction("inc_font_sizes");
    action->setIcon(QIcon::fromTheme("format-font-size-more"));
    action->setText(i18n("Enlarge Font"));
    connect(action, SIGNAL(triggered(bool)), d->tabWidget, SLOT(slotFrameZoomIn()));
    coll->setDefaultShortcut(action, QKeySequence::ZoomIn);

    action = coll->addAction("dec_font_sizes");
    action->setIcon(QIcon::fromTheme("format-font-size-less"));
    action->setText(i18n("Shrink Font"));
    connect(action, SIGNAL(triggered(bool)), d->tabWidget, SLOT(slotFrameZoomOut()));
    coll->setDefaultShortcut(action, QKeySequence::ZoomOut);

    QString actionname;
    for (int i = 1; i < 10; ++i) {
        actionname.sprintf("activate_tab_%02d", i);
        action = new QAction(i18n("Activate Tab %1", i), this);
        coll->addAction(actionname, action);
        coll->setDefaultShortcut(action, QKeySequence(QStringLiteral("Alt+%1").arg(i)));
        connect(action, SIGNAL(triggered(bool)), d->tabWidget, SLOT(slotActivateTab()));
    }

}

void ActionManagerImpl::initFrameManager(FrameManager *frameManager)
{
    if (d->frameManager) {
        return;
    }

    d->frameManager = frameManager;

    bool isRTL = QApplication::isRightToLeft();

    KToolBarPopupAction *forward = new KToolBarPopupAction(QIcon::fromTheme(isRTL ? "go-previous" : "go-next"), i18nc("Go forward in browser history", "Forward"), this);
    d->actionCollection->addAction("browser_forward", forward);
    d->actionCollection->setDefaultShortcut(forward, QKeySequence(isRTL ? "Alt+Left" : "Alt+Right"));
    connect(forward, &KToolBarPopupAction::triggered, frameManager, &FrameManager::slotBrowserForward);

    connect(forward->menu(), SIGNAL(aboutToShow()), frameManager, SLOT(slotBrowserForwardAboutToShow()));

    KToolBarPopupAction *back = new KToolBarPopupAction(QIcon::fromTheme(isRTL ? "go-next" : "go-previous"), i18nc("Go back in browser history", "Back"), this);
    d->actionCollection->addAction("browser_back", back);
    d->actionCollection->setDefaultShortcut(back, QKeySequence(isRTL ?  "Alt+Right" : "Alt+Left"));
    connect(back, &KToolBarPopupAction::triggered, frameManager, &FrameManager::slotBrowserBack);

    connect(back->menu(), SIGNAL(aboutToShow()), frameManager, SLOT(slotBrowserBackAboutToShow()));

    QAction *action = d->actionCollection->addAction("browser_reload");
    action->setIcon(QIcon::fromTheme("view-refresh"));
    action->setText(i18nc("Reload current page", "Reload"));
    connect(action, &QAction::triggered, frameManager, &FrameManager::slotBrowserReload);

    action = d->actionCollection->addAction("browser_stop");
    action->setIcon(QIcon::fromTheme("process-stop"));
    action->setText(i18n("Stop"));
    connect(action, &QAction::triggered, frameManager, &FrameManager::slotBrowserStop);
}

QWidget *ActionManagerImpl::container(const char *name)
{
    if (d->part->factory()) {
        return d->part->factory()->container(name, d->part);
    } else {
        return 0;
    }
}

KActionCollection *ActionManagerImpl::actionCollection()
{
    return d->actionCollection;
}

QAction *ActionManagerImpl::action(const char *name)
{
    return d->actionCollection != 0 ? d->actionCollection->action(name) : 0;
}

void ActionManagerImpl::setArticleActionsEnabled(bool enabled)
{
#undef setActionEnabled
#define setActionEnabled(name) { QAction* const a = action( name ); if ( a ) a->setEnabled( enabled ); }
    setActionEnabled("article_open")
    setActionEnabled("article_open_external")
    setActionEnabled("article_set_status_important")
    setActionEnabled("article_set_status")
    setActionEnabled("article_delete")
    setActionEnabled("file_sendlink")
    setActionEnabled("file_sendfile")
    setActionEnabled("article_open_in_background")
#undef setActionEnabled
}

} // namespace Akregator

