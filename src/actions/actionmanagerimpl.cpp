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
#include "feed.h"
#include "fetchqueue.h"
#include "folder.h"
#include "kernel.h"
#include "mainwidget.h"
#include "subscriptionlistview.h"
#include "tabwidget.h"
#include "trayicon.h"
#include "treenode.h"
#include "treenodevisitor.h"

#include <WebEngineViewer/ZoomActionMenu>

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

    bool visitFeed(Feed *node) Q_DECL_OVERRIDE {
        QAction *remove = m_manager->action(QStringLiteral("feed_remove"));
        if (remove)
        {
            remove->setEnabled(true);
        }
        QAction *hp = m_manager->action(QStringLiteral("feed_homepage"));
        if (hp)
        {
            hp->setEnabled(!node->htmlUrl().isEmpty());
        }
        m_manager->action(QStringLiteral("feed_fetch"))->setText(i18n("&Fetch Feed"));
        m_manager->action(QStringLiteral("feed_remove"))->setText(i18n("&Delete Feed"));
        m_manager->action(QStringLiteral("feed_modify"))->setText(i18n("&Edit Feed..."));
        m_manager->action(QStringLiteral("feed_mark_all_as_read"))->setText(i18n("&Mark Feed as Read"));

        return true;
    }

    bool visitFolder(Folder *node) Q_DECL_OVERRIDE {
        QAction *remove = m_manager->action(QStringLiteral("feed_remove"));
        if (remove)
        {
            remove->setEnabled(node->parent());    // root nodes must not be deleted
        }
        QAction *hp = m_manager->action(QStringLiteral("feed_homepage"));
        if (hp)
        {
            hp->setEnabled(false);
        }

        m_manager->action(QStringLiteral("feed_fetch"))->setText(i18n("&Fetch Feeds"));
        m_manager->action(QStringLiteral("feed_remove"))->setText(i18n("&Delete Folder"));
        m_manager->action(QStringLiteral("feed_modify"))->setText(i18n("&Rename Folder"));
        m_manager->action(QStringLiteral("feed_mark_all_as_read"))->setText(i18n("&Mark Feeds as Read"));

        return true;
    }

private:
    ActionManagerImpl *m_manager;
};

class ActionManagerImpl::ActionManagerImplPrivate
{
public:
    QString quickSearchLineText() const;
    NodeSelectVisitor *nodeSelectVisitor;
    ArticleListView *articleList;
    SubscriptionListView *subscriptionListView;
    MainWidget *mainWidget;
    Part *part;
    TrayIcon *trayIcon;
    KActionMenu *tagMenu;
    KActionCollection *actionCollection;
    TabWidget *tabWidget;
    PimCommon::ShareServiceUrlManager *shareServiceManager;
    WebEngineViewer::ZoomActionMenu *zoomActionMenu;
    QAction *mQuickSearchAction;
};

void ActionManagerImpl::slotNodeSelected(TreeNode *node)
{
    if (node != 0) {
        d->nodeSelectVisitor->visit(node);
    }
}

ActionManagerImpl::ActionManagerImpl(Part *part, QObject *parent)
    : ActionManager(parent), d(new ActionManagerImplPrivate)
{
    d->nodeSelectVisitor = new NodeSelectVisitor(this);
    d->part = part;
    d->subscriptionListView = 0;
    d->articleList = 0;
    d->trayIcon = 0;
    d->mainWidget = 0;
    d->tabWidget = 0;
    d->tagMenu = 0;
    d->actionCollection = part->actionCollection();
    d->shareServiceManager = new PimCommon::ShareServiceUrlManager(this);
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

    if (QAction *act = actionCollection()->action(QStringLiteral("feed_fetch_all"))) {
        traypop->addAction(act);
    }
    if (QAction *act = actionCollection()->action(QStringLiteral("options_configure"))) {
        traypop->addAction(act);
    }
}

void ActionManagerImpl::initPart()
{
    QAction *action = d->actionCollection->addAction(QStringLiteral("file_import"));
    action->setText(i18n("&Import Feeds..."));
    action->setIcon(QIcon::fromTheme(QStringLiteral("document-import")));
    connect(action, &QAction::triggered, d->part, &Part::fileImport);
    action = d->actionCollection->addAction(QStringLiteral("file_export"));
    action->setText(i18n("&Export Feeds..."));
    action->setIcon(QIcon::fromTheme(QStringLiteral("document-export")));
    connect(action, &QAction::triggered, d->part, &Part::fileExport);

    QAction *configure = d->actionCollection->addAction(QStringLiteral("options_configure"));
    configure->setText(i18n("&Configure Akregator..."));
    configure->setIcon(QIcon::fromTheme(QStringLiteral("configure")));
    connect(configure, &QAction::triggered, d->part, &Part::showOptions);

    KStandardAction::configureNotifications(d->part, SLOT(showNotificationOptions()), d->actionCollection); // options_configure_notifications
}

void ActionManagerImpl::initMainWidget(MainWidget *mainWidget)
{
    if (d->mainWidget) {
        return;
    }

    d->mainWidget = mainWidget;

    KActionCollection *coll = actionCollection();

    // Feed/Feed Group popup menu
    QAction *action = coll->addAction(QStringLiteral("feed_homepage"));
    action->setText(i18n("&Open Homepage"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotOpenHomepage);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::Key_H));

    action = coll->addAction(QStringLiteral("reload_all_tabs"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));
    action->setText(i18n("Reload All Tabs"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotReloadAllTabs);
    coll->setDefaultShortcut(action, QKeySequence(Qt::SHIFT + Qt::Key_F5));

    action = coll->addAction(QStringLiteral("feed_add"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("feed-subscribe")));
    action->setText(i18n("&Add Feed..."));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotFeedAdd);
    coll->setDefaultShortcut(action, QKeySequence(Qt::Key_Insert));

    action = coll->addAction(QStringLiteral("feed_add_group"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("folder-new")));
    action->setText(i18n("Ne&w Folder..."));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotFeedAddGroup);
    coll->setDefaultShortcut(action, QKeySequence(Qt::SHIFT + Qt::Key_Insert));

    action = coll->addAction(QStringLiteral("feed_remove"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
    action->setText(i18n("&Delete Feed"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotFeedRemove);
    coll->setDefaultShortcut(action, QKeySequence(Qt::ALT + Qt::Key_Delete));

    action = coll->addAction(QStringLiteral("feed_modify"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("document-properties")));
    action->setText(i18n("&Edit Feed..."));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotFeedModify);
    coll->setDefaultShortcut(action, QKeySequence(Qt::Key_F2));

    // toolbar / View

    const MainWidget::ViewMode viewMode = static_cast<MainWidget::ViewMode>(Settings::viewMode());
    QActionGroup *group = new QActionGroup(this);
    action = coll->addAction(QStringLiteral("normal_view"));
    action->setCheckable(true);
    action->setChecked(viewMode == MainWidget::NormalView);
    group->addAction(action);
    action->setIcon(QIcon::fromTheme(QStringLiteral("view-split-top-bottom")));
    action->setText(i18n("&Normal View"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotNormalView);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_1));

    action = coll->addAction(QStringLiteral("widescreen_view"));
    action->setCheckable(true);
    action->setChecked(viewMode == MainWidget::WidescreenView);
    group->addAction(action);
    action->setIcon(QIcon::fromTheme(QStringLiteral("view-split-left-right")));
    action->setText(i18n("&Widescreen View"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotWidescreenView);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_2));

    action = coll->addAction(QStringLiteral("combined_view"));
    action->setCheckable(true);
    action->setChecked(viewMode == MainWidget::CombinedView);
    group->addAction(action);
    action->setIcon(QIcon::fromTheme(QStringLiteral("view-list-text")));
    action->setText(i18n("C&ombined View"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotCombinedView);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_3));

    // toolbar / feed menu
    action = coll->addAction(QStringLiteral("feed_fetch"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("go-down")));
    action->setText(i18n("&Fetch Feed"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotFetchCurrentFeed);
    coll->setDefaultShortcuts(action, KStandardShortcut::shortcut(KStandardShortcut::Reload));

    action = coll->addAction(QStringLiteral("feed_fetch_all"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("go-bottom")));
    action->setText(i18n("Fe&tch All Feeds"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotFetchAllFeeds);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::Key_L));

    QAction *stopAction = coll->addAction(QStringLiteral("feed_stop"));
    stopAction->setIcon(QIcon::fromTheme(QStringLiteral("process-stop")));
    stopAction->setText(i18n("C&ancel Feed Fetches"));
    connect(stopAction, &QAction::triggered, Kernel::self()->fetchQueue(), &FetchQueue::slotAbort);
    coll->setDefaultShortcut(stopAction, QKeySequence(Qt::Key_Escape));
    stopAction->setEnabled(false);

    action = coll->addAction(QStringLiteral("feed_mark_all_as_read"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("mail-mark-read")));
    action->setText(i18n("&Mark Feed as Read"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotMarkAllRead);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::Key_R));

    action = coll->addAction(QStringLiteral("feed_mark_all_feeds_as_read"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("mail-mark-read")));
    action->setText(i18n("Ma&rk All Feeds as Read"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotMarkAllFeedsRead);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_R));

    // Settings menu
    KToggleAction *sqf = coll->add<KToggleAction>(QStringLiteral("show_quick_filter"));
    sqf->setText(i18n("Show Quick Filter"));
    connect(sqf, &QAction::triggered, d->mainWidget, &MainWidget::slotToggleShowQuickFilter);
    sqf->setChecked(Settings::showQuickFilter());

    action = coll->addAction(QStringLiteral("article_open"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("tab-new")));
    action->setText(i18n("Open in Tab"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotOpenSelectedArticles);
    coll->setDefaultShortcut(action, QKeySequence(Qt::SHIFT + Qt::Key_Return));

    action = coll->addAction(QStringLiteral("article_open_in_background"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("tab-new")));
    action->setText(i18n("Open in Background Tab"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotOpenSelectedArticlesInBackground);
    coll->setDefaultShortcut(action, QKeySequence(Qt::Key_Return));

    action = coll->addAction(QStringLiteral("article_open_external"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("window-new")));
    action->setText(i18n("Open in External Browser"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotOpenSelectedArticlesInBrowser);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Return));

    action = coll->addAction(QStringLiteral("article_copy_link_address"));
    action->setText(i18n("Copy Link Address"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotCopyLinkAddress);

    action = coll->addAction(QStringLiteral("go_prev_unread_article"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("go-previous")));
    action->setText(i18n("Pre&vious Unread Article"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotPrevUnreadArticle);
    coll->setDefaultShortcuts(action, QList<QKeySequence>() << QKeySequence(Qt::Key_Minus) << QKeySequence(Qt::Key_Minus + Qt::KeypadModifier));

    action = coll->addAction(QStringLiteral("go_next_unread_article"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("go-next")));
    action->setText(i18n("Ne&xt Unread Article"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotNextUnreadArticle);
    coll->setDefaultShortcuts(action, QList<QKeySequence>() << QKeySequence(Qt::Key_Plus) << QKeySequence(Qt::Key_Plus + Qt::KeypadModifier)
                              << QKeySequence(Qt::Key_Equal) << QKeySequence(Qt::Key_Equal + Qt::KeypadModifier));

    action = coll->addAction(QStringLiteral("article_delete"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
    action->setText(i18n("&Delete"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotArticleDelete);
    coll->setDefaultShortcut(action, QKeySequence(Qt::Key_Delete));

    KActionMenu *statusMenu = coll->add<KActionMenu>(QStringLiteral("article_set_status"));
    statusMenu->setText(i18n("&Mark As"));
    statusMenu->setEnabled(false);

    action = coll->addAction(QStringLiteral("article_set_status_read"));
    action->setText(i18nc("as in: mark as read", "&Read"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("mail-mark-read")));
    action->setToolTip(i18n("Mark selected article as read"));
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::Key_E));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotSetSelectedArticleRead);
    statusMenu->addAction(action);

    action = coll->addAction(QStringLiteral("article_set_status_new"));
    action->setText(i18nc("as in: mark as new", "&New"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("mail-mark-unread-new")));
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::Key_N));
    action->setToolTip(i18n("Mark selected article as new"));
    connect(action, &QAction::triggered,  d->mainWidget, &MainWidget::slotSetSelectedArticleNew);
    statusMenu->addAction(action);

    action = coll->addAction(QStringLiteral("article_set_status_unread"));
    action->setText(i18nc("as in: mark as unread", "&Unread"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("mail-mark-unread")));
    action->setToolTip(i18n("Mark selected article as unread"));
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::Key_U));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotSetSelectedArticleUnread);
    statusMenu->addAction(action);

    KToggleAction *importantAction = coll->add<KToggleAction>(QStringLiteral("article_set_status_important"));
    importantAction->setText(i18n("&Mark as Important"));
    importantAction->setIcon(QIcon::fromTheme(QStringLiteral("mail-mark-important")));
    const QList<QKeySequence> importantSC = {QKeySequence(Qt::CTRL + Qt::Key_I), QKeySequence(Qt::Key_I)};
    coll->setDefaultShortcuts(importantAction, importantSC);
    importantAction->setCheckedState(KGuiItem(i18n("Remove &Important Mark")));
    connect(importantAction, &QAction::triggered, d->mainWidget, &MainWidget::slotArticleToggleKeepFlag);

    action = coll->addAction(QStringLiteral("feedstree_move_up"));
    action->setText(i18n("Move Node Up"));
    connect(action, &QAction::triggered, mainWidget, &MainWidget::slotMoveCurrentNodeUp);
    coll->setDefaultShortcut(action, QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_Up));

    action = coll->addAction(QStringLiteral("feedstree_move_down"));
    action->setText(i18n("Move Node Down"));
    connect(action, &QAction::triggered, mainWidget, &MainWidget::slotMoveCurrentNodeDown);
    coll->setDefaultShortcut(action, QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_Down));

    action = coll->addAction(QStringLiteral("move_node_left"));
    action->setText(i18n("Move Node Left"));
    connect(action, &QAction::triggered, mainWidget, &MainWidget::slotMoveCurrentNodeLeft);
    coll->setDefaultShortcut(action, QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_Left));

    action = coll->addAction(QStringLiteral("feedstree_move_right"));
    action->setText(i18n("Move Node Right"));
    connect(action, &QAction::triggered, mainWidget, &MainWidget::slotMoveCurrentNodeRight);
    coll->setDefaultShortcut(action, QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_Right));

    action = coll->addAction(QStringLiteral("file_sendlink"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("mail-message-new")));
    action->setText(i18n("Send &Link Address..."));
    connect(action, &QAction::triggered, mainWidget, &MainWidget::slotSendLink);

    action = coll->addAction(QStringLiteral("file_sendfile"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("mail-message-new")));
    action->setText(i18n("Send &File..."));
    connect(action, &QAction::triggered, mainWidget, &MainWidget::slotSendFile);

    coll->addAction(QStringLiteral("share_serviceurl"), d->shareServiceManager->menu());
    connect(d->shareServiceManager, &PimCommon::ShareServiceUrlManager::serviceUrlSelected, this, &ActionManagerImpl::slotServiceUrlSelected);

    d->mQuickSearchAction = new QAction(i18n("Set Focus to Quick Search"), this);
    //If change shortcut change Panel::setQuickSearchClickMessage(...) message
    actionCollection()->setDefaultShortcut(d->mQuickSearchAction, QKeySequence(Qt::ALT + Qt::Key_Q));
    actionCollection()->addAction(QStringLiteral("focus_to_quickseach"), d->mQuickSearchAction);
    connect(d->mQuickSearchAction, &QAction::triggered, mainWidget, &MainWidget::slotFocusQuickSearch);

    setArticleActionsEnabled(false);
}

void ActionManagerImpl::slotServiceUrlSelected(PimCommon::ShareServiceUrlManager::ServiceType type)
{
    if (d->mainWidget) {
        QString title;
        QString link;
        d->mainWidget->currentArticleInfo(link, title);
        const QUrl url = d->shareServiceManager->generateServiceUrl(link, title, type);
        d->shareServiceManager->openUrl(url);
    }
}

void ActionManagerImpl::initArticleListView(ArticleListView *articleList)
{
    if (d->articleList) {
        return;
    } else {
        d->articleList = articleList;
    }

    QAction *action = actionCollection()->addAction(QStringLiteral("go_previous_article"));
    action->setText(i18n("&Previous Article"));
    connect(action, &QAction::triggered, articleList, &ArticleListView::slotPreviousArticle);
    actionCollection()->setDefaultShortcut(action, QKeySequence(Qt::Key_Left));
    action = actionCollection()->addAction(QStringLiteral("go_next_article"));
    action->setText(i18n("&Next Article"));
    connect(action, &QAction::triggered, articleList, &ArticleListView::slotNextArticle);
    actionCollection()->setDefaultShortcut(action, QKeySequence(Qt::Key_Right));
}

void ActionManagerImpl::initSubscriptionListView(SubscriptionListView *subscriptionListView)
{
    if (d->subscriptionListView) {
        return;
    } else {
        d->subscriptionListView = subscriptionListView;
    }

    KActionCollection *coll = actionCollection();

    QAction *action = coll->addAction(QStringLiteral("go_prev_feed"));
    action->setText(i18n("&Previous Feed"));
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotPrevFeed);
    coll->setDefaultShortcut(action, QKeySequence(Qt::Key_P));

    action = coll->addAction(QStringLiteral("go_next_feed"));
    action->setText(i18n("&Next Feed"));
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotNextFeed);
    coll->setDefaultShortcut(action, QKeySequence(Qt::Key_N));

    action = coll->addAction(QStringLiteral("go_next_unread_feed"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("go-down")));
    action->setText(i18n("N&ext Unread Feed"));
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotNextUnreadFeed);
    coll->setDefaultShortcut(action, QKeySequence(Qt::ALT + Qt::Key_Plus));

    action = coll->addAction(QStringLiteral("go_prev_unread_feed"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("go-up")));
    action->setText(i18n("Prev&ious Unread Feed"));
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotPrevUnreadFeed);
    coll->setDefaultShortcut(action, QKeySequence(Qt::ALT + Qt::Key_Minus));

    action = coll->addAction(QStringLiteral("feedstree_home"));
    action->setText(i18n("Go to Top of Tree"));
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotItemBegin);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::Key_Home));

    action = coll->addAction(QStringLiteral("feedstree_end"));
    action->setText(i18n("Go to Bottom of Tree"));
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotItemEnd);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::Key_End));

    action = coll->addAction(QStringLiteral("feedstree_left"));
    action->setText(i18n("Go Left in Tree"));
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotItemLeft);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::Key_Left));

    action = coll->addAction(QStringLiteral("feedstree_right"));
    action->setText(i18n("Go Right in Tree"));
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotItemRight);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::Key_Right));

    action = coll->addAction(QStringLiteral("feedstree_up"));
    action->setText(i18n("Go Up in Tree"));
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotItemUp);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::Key_Up));

    action = coll->addAction(QStringLiteral("feedstree_down"));
    action->setText(i18n("Go Down in Tree"));
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotItemDown);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::Key_Down));
}

void ActionManagerImpl::initTabWidget(TabWidget *tabWidget)
{
    if (d->tabWidget) {
        return;
    } else {
        d->tabWidget = tabWidget;
    }

    KActionCollection *coll = actionCollection();

    QAction *action = coll->addAction(QStringLiteral("select_next_tab"));
    action->setText(i18n("Select Next Tab"));
    connect(action, &QAction::triggered, d->tabWidget, &TabWidget::slotNextTab);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::Key_Period));

    action = coll->addAction(QStringLiteral("select_previous_tab"));
    action->setText(i18n("Select Previous Tab"));
    connect(action, &QAction::triggered, d->tabWidget, &TabWidget::slotPreviousTab);
    coll->setDefaultShortcut(action, QKeySequence(Qt::Key_Comma + Qt::CTRL));

    action = coll->addAction(QStringLiteral("tab_detach"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("tab-detach")));
    action->setText(i18n("Detach Tab"));
    connect(action, &QAction::triggered, d->tabWidget, &TabWidget::slotDetachTab);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_B));

    action = KStandardAction::copy(d->tabWidget, SLOT(slotCopy()), coll);
    coll->addAction(QStringLiteral("viewer_copy"), action);

    action = KStandardAction::print(d->tabWidget, SLOT(slotPrint()), coll);
    coll->addAction(QStringLiteral("viewer_print"), action);

    action = KStandardAction::printPreview(d->tabWidget, SLOT(slotPrintPreview()), coll);
    coll->addAction(QStringLiteral("viewer_printpreview"), action);


    action = coll->addAction(QStringLiteral("tab_mute"));
    action->setText(i18n("Mute"));
    connect(action, &QAction::triggered, d->tabWidget, &TabWidget::slotMute);

    action = coll->addAction(QStringLiteral("tab_unmute"));
    action->setText(i18n("Unmute"));
    connect(action, &QAction::triggered, d->tabWidget, &TabWidget::slotUnMute);

    action = new QAction(i18n("Speak Text"), this);
    action->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-text-to-speech")));
    coll->addAction(QStringLiteral("speak_text"), action);
    connect(action, &QAction::triggered, d->tabWidget, &TabWidget::slotTextToSpeech);

    action = new QAction(QIcon::fromTheme(QStringLiteral("edit-find")), i18n("&Find in Message..."), this);
    coll->addAction(QStringLiteral("find_in_messages"), action);
    connect(action, &QAction::triggered, d->tabWidget, &TabWidget::slotFindTextInHtml);
    coll->setDefaultShortcut(action, KStandardShortcut::find().first());

    action = coll->addAction(QStringLiteral("tab_copylinkaddress"));
    action->setText(i18n("Copy Link Address"));
    connect(action, &QAction::triggered, d->tabWidget, &TabWidget::slotCopyLinkAddress);

    action = coll->addAction(QStringLiteral("tab_remove"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("tab-close")));
    action->setText(i18n("Close Tab"));
    connect(action, &QAction::triggered, d->tabWidget, &TabWidget::slotCloseTab);
    coll->setDefaultShortcuts(action, KStandardShortcut::close());

    d->zoomActionMenu = new WebEngineViewer::ZoomActionMenu(this);
    connect(d->zoomActionMenu, &WebEngineViewer::ZoomActionMenu::zoomChanged, d->tabWidget, &TabWidget::slotZoomChanged);

    d->zoomActionMenu->setActionCollection(coll);
    d->zoomActionMenu->createZoomActions();

    QString actionname;
    for (int i = 1; i < 10; ++i) {
        actionname.sprintf("activate_tab_%02d", i);
        action = new QAction(i18n("Activate Tab %1", i), this);
        coll->addAction(actionname, action);
        coll->setDefaultShortcut(action, QKeySequence(QStringLiteral("Alt+%1").arg(i)));
        connect(action, &QAction::triggered, d->tabWidget, &TabWidget::slotActivateTab);
    }
    action = coll->addAction(QStringLiteral("savelinkas"));
    action->setText(i18n("&Save Link As..."));
    connect(action, &QAction::triggered, d->tabWidget, &TabWidget::slotSaveLinkAs);

    action = coll->addAction(QStringLiteral("copylinkaddress"));
    action->setText(i18n("Copy &Link Address"));
    connect(action, &QAction::triggered, d->tabWidget, &TabWidget::slotCopyLinkAddress);

    action = new QAction(i18n("Copy Image Location"), this);
    action->setIcon(QIcon::fromTheme(QStringLiteral("view-media-visualization")));
    coll->addAction(QStringLiteral("copy_image_location"), action);
    coll->setShortcutsConfigurable(action, false);
    connect(action, &QAction::triggered, d->tabWidget, &TabWidget::slotCopyImageLocation);

    // save Image On Disk
    action = new QAction(i18n("Save Image On Disk..."), this);
    coll->addAction(QStringLiteral("saveas_imageurl"), action);
    coll->setShortcutsConfigurable(action, false);
    connect(action, &QAction::triggered, d->tabWidget, &TabWidget::slotSaveImageOnDisk);
}

QWidget *ActionManagerImpl::container(const QString &name)
{
    if (d->part->factory()) {
        return d->part->factory()->container(name, d->part);
    } else {
        return 0;
    }
}

KActionCollection *ActionManagerImpl::actionCollection() const
{
    return d->actionCollection;
}

QAction *ActionManagerImpl::action(const QString &name)
{
    return d->actionCollection != Q_NULLPTR ? d->actionCollection->action(name) : Q_NULLPTR;
}

void ActionManagerImpl::setArticleActionsEnabled(bool enabled)
{
#undef setActionEnabled
#define setActionEnabled(name) { QAction* const a = action( name ); if ( a ) a->setEnabled( enabled ); }
    setActionEnabled(QStringLiteral("article_open"))
    setActionEnabled(QStringLiteral("article_open_external"))
    setActionEnabled(QStringLiteral("article_set_status_important"))
    setActionEnabled(QStringLiteral("article_set_status"))
    setActionEnabled(QStringLiteral("article_delete"))
    setActionEnabled(QStringLiteral("file_sendlink"))
    setActionEnabled(QStringLiteral("file_sendfile"))
    setActionEnabled(QStringLiteral("article_open_in_background"))
    setActionEnabled(QStringLiteral("share_serviceurl"))
#undef setActionEnabled
}

WebEngineViewer::ZoomActionMenu *ActionManagerImpl::zoomActionMenu() const
{
    return d->zoomActionMenu;
}

QString ActionManagerImpl::quickSearchLineText() const
{
    return d->quickSearchLineText();
}

QString ActionManagerImpl::ActionManagerImplPrivate::quickSearchLineText() const
{
    return mQuickSearchAction->shortcut().toString();
}

} // namespace Akregator

