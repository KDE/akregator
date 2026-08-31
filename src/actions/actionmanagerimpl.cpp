/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#include "actionmanagerimpl.h"
#include "akregator_part.h"
#include "akregatorconfig.h"
#include "articlelistview.h"
#include "config-akregator.h"
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

#include "akregator_debug.h"
#include <KActionCollection>
#include <KActionMenu>
#include <KColorSchemeMenu>
#include <KGuiItem>
#include <KLocalizedString>
#include <KStandardAction>
#include <KStandardShortcut>
#include <KToggleAction>
#include <QAction>
#include <QActionGroup>
#include <QKeySequence>
#include <QMenu>
#include <kxmlguifactory.h>

#include <KColorSchemeManager>
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
#include <TextAddonsWidgets/VerifyNewVersionWidget>
#endif
#include <QWidget>

using namespace Akregator;
using namespace Qt::Literals::StringLiterals;
class ActionManagerImpl::NodeSelectVisitor : public TreeNodeVisitor
{
public:
    explicit NodeSelectVisitor(ActionManagerImpl *manager)
        : m_manager(manager)
    {
    }

    bool visitFeed(Feed *node) override
    {
        QAction *remove = m_manager->action(u"feed_remove"_s);
        if (remove) {
            remove->setEnabled(true);
        }
        QAction *hp = m_manager->action(u"feed_homepage"_s);
        if (hp) {
            hp->setEnabled(!node->htmlUrl().isEmpty());
        }
        m_manager->action(u"feed_fetch"_s)->setText(i18n("&Fetch Feed"));
        m_manager->action(u"feed_remove"_s)->setText(i18n("&Delete Feed"));
        m_manager->action(u"feed_modify"_s)->setText(i18n("&Edit Feed…"));
        m_manager->action(u"feed_mark_all_as_read"_s)->setText(i18n("&Mark Feed as Read"));

        return true;
    }

    bool visitFolder(Folder *node) override
    {
        QAction *remove = m_manager->action(u"feed_remove"_s);
        if (remove) {
            remove->setEnabled(node->parent()); // root nodes must not be deleted
        }
        QAction *hp = m_manager->action(u"feed_homepage"_s);
        if (hp) {
            hp->setEnabled(false);
        }

        m_manager->action(u"feed_fetch"_s)->setText(i18n("&Fetch Feeds"));
        m_manager->action(u"feed_remove"_s)->setText(i18n("&Delete Folder"));
        m_manager->action(u"feed_modify"_s)->setText(i18n("&Rename Folder"));
        m_manager->action(u"feed_mark_all_as_read"_s)->setText(i18n("&Mark Feeds as Read"));

        return true;
    }

private:
    ActionManagerImpl *const m_manager;
};

class ActionManagerImpl::ActionManagerImplPrivate
{
public:
    [[nodiscard]] QString quickSearchLineText() const;
    NodeSelectVisitor *nodeSelectVisitor = nullptr;
    ArticleListView *articleList = nullptr;
    SubscriptionListView *subscriptionListView = nullptr;
    MainWidget *mainWidget = nullptr;
    Part *part = nullptr;
    TrayIcon *trayIcon = nullptr;
    KActionMenu *tagMenu = nullptr;
    KActionCollection *actionCollection = nullptr;
    TabWidget *tabWidget = nullptr;
    PimCommon::ShareServiceUrlManager *shareServiceManager = nullptr;
    WebEngineViewer::ZoomActionMenu *zoomActionMenu = nullptr;
    QAction *mQuickSearchAction = nullptr;
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    TextAddonsWidgets::VerifyNewVersionWidget *mVerifyNewVersionWidget = nullptr;
#endif
};

void ActionManagerImpl::slotSettingsChanged()
{
    QAction *a = action(u"feed_hide_read"_s);
    if (!a) {
        qCCritical(AKREGATOR_LOG) << "Action not found";
        return;
    }
    a->setChecked(Settings::hideReadFeeds());

    a = action(u"feed_lock_in_place"_s);
    if (!a) {
        qCCritical(AKREGATOR_LOG) << "Action not found";
        return;
    }
    a->setChecked(Settings::lockFeedsInPlace());

    a = action(u"auto_expand_folders"_s);
    if (!a) {
        qCCritical(AKREGATOR_LOG) << "Action not found";
        return;
    }
    a->setChecked(Settings::autoExpandFolders());
}

void ActionManagerImpl::slotNodeSelected(TreeNode *node)
{
    if (node) {
        d->nodeSelectVisitor->visit(node);
    }
}

ActionManagerImpl::ActionManagerImpl(Part *part, QObject *parent)
    : ActionManager(parent)
    , d(new ActionManagerImplPrivate)
{
    d->nodeSelectVisitor = new NodeSelectVisitor(this);
    d->part = part;
    d->subscriptionListView = nullptr;
    d->articleList = nullptr;
    d->trayIcon = nullptr;
    d->mainWidget = nullptr;
    d->tabWidget = nullptr;
    d->tagMenu = nullptr;
    d->actionCollection = part->actionCollection();
    d->shareServiceManager = new PimCommon::ShareServiceUrlManager(this);
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    d->mVerifyNewVersionWidget = new TextAddonsWidgets::VerifyNewVersionWidget(this);
#endif

    initPart();
}

ActionManagerImpl::~ActionManagerImpl()
{
    delete d->nodeSelectVisitor;
}

void ActionManagerImpl::setTrayIcon(TrayIcon *trayIcon)
{
    if (trayIcon == nullptr) {
        d->trayIcon = nullptr;
        return;
    }
    if (d->trayIcon) {
        return;
    } else {
        d->trayIcon = trayIcon;
    }

    QMenu *traypop = trayIcon->contextMenu();

    if (QAction *act = actionCollection()->action(u"feed_fetch_all"_s)) {
        traypop->addAction(act);
    }
    if (QAction *act = actionCollection()->action(u"options_configure"_s)) {
        traypop->addAction(act);
    }
}

void ActionManagerImpl::initPart()
{
    QAction *action = d->actionCollection->addAction(u"file_import"_s);
    action->setText(i18n("&Import Feeds…"));
    action->setIcon(QIcon::fromTheme(u"document-import"_s));
    connect(action, &QAction::triggered, d->part, &Part::fileImport);
    action = d->actionCollection->addAction(u"file_export"_s);
    action->setText(i18n("&Export Feeds…"));
    action->setIcon(QIcon::fromTheme(u"document-export"_s));
    connect(action, &QAction::triggered, d->part, &Part::fileExport);

    QAction *configure = d->actionCollection->addAction(u"options_configure"_s);
    configure->setText(i18n("&Configure Akregator…"));
    configure->setIcon(QIcon::fromTheme(u"configure"_s));
    connect(configure, &QAction::triggered, d->part, &Part::showOptions);

    KStandardActions::configureNotifications(d->part, &Part::showNotificationOptions, d->actionCollection); // options_configure_notifications
}

void ActionManagerImpl::initMainWidget(MainWidget *mainWidget)
{
    if (d->mainWidget) {
        return;
    }

    d->mainWidget = mainWidget;

    KActionCollection *coll = actionCollection();

    // Feed/Feed Group popup menu
    QAction *action = coll->addAction(u"feed_homepage"_s);
    action->setText(i18n("&Open Homepage"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotOpenHomepage);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::Key_H));

    action = coll->addAction(u"reload_all_tabs"_s);
    action->setIcon(QIcon::fromTheme(u"view-refresh"_s));
    action->setText(i18n("Reload All Tabs"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotReloadAllTabs);
    coll->setDefaultShortcut(action, QKeySequence(Qt::SHIFT | Qt::Key_F5));

    action = coll->addAction(u"feed_add"_s);
    action->setIcon(QIcon::fromTheme(u"feed-subscribe"_s));
    action->setText(i18n("&Add Feed…"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotFeedAdd);
    coll->setDefaultShortcut(action, QKeySequence(Qt::Key_Insert));

    action = coll->addAction(u"feed_add_group"_s);
    action->setIcon(QIcon::fromTheme(u"folder-new"_s));
    action->setText(i18n("Ne&w Folder…"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotFeedAddGroup);
    coll->setDefaultShortcut(action, QKeySequence(Qt::SHIFT | Qt::Key_Insert));

    action = coll->addAction(u"feed_remove"_s);
    action->setIcon(QIcon::fromTheme(u"edit-delete"_s));
    action->setText(i18n("&Delete Feed"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotFeedRemove);
    coll->setDefaultShortcut(action, QKeySequence(Qt::ALT | Qt::Key_Delete));

    action = coll->addAction(u"feed_modify"_s);
    action->setIcon(QIcon::fromTheme(u"document-properties"_s));
    action->setText(i18n("&Edit Feed…"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotFeedModify);
    coll->setDefaultShortcut(action, QKeySequence(Qt::Key_F2));

    // toolbar / View

    const auto viewMode = static_cast<MainWidget::ViewMode>(Settings::viewMode());
    auto group = new QActionGroup(this);
    action = coll->addAction(u"normal_view"_s);
    action->setCheckable(true);
    action->setChecked(viewMode == MainWidget::NormalView);
    group->addAction(action);
    action->setIcon(QIcon::fromTheme(u"view-split-top-bottom"_s));
    action->setText(i18n("&Normal View"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotNormalView);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_1));

    action = coll->addAction(u"widescreen_view"_s);
    action->setCheckable(true);
    action->setChecked(viewMode == MainWidget::WidescreenView);
    group->addAction(action);
    action->setIcon(QIcon::fromTheme(u"view-split-left-right"_s));
    action->setText(i18n("&Widescreen View"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotWidescreenView);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_2));

    action = coll->addAction(u"combined_view"_s);
    action->setCheckable(true);
    action->setChecked(viewMode == MainWidget::CombinedView);
    group->addAction(action);
    action->setIcon(QIcon::fromTheme(u"view-list-text"_s));
    action->setText(i18n("C&ombined View"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotCombinedView);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_3));

    // toolbar / feed menu
    action = coll->addAction(u"feed_fetch"_s);
    action->setIcon(QIcon::fromTheme(u"go-down"_s));
    action->setText(i18n("&Fetch Feed"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotFetchCurrentFeed);
    coll->setDefaultShortcuts(action, KStandardShortcut::shortcut(KStandardShortcut::Reload));

    action = coll->addAction(u"feed_fetch_all"_s);
    action->setIcon(QIcon::fromTheme(u"go-bottom"_s));
    action->setText(i18n("Fe&tch All Feeds"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotFetchAllFeeds);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::Key_L));

    QAction *stopAction = coll->addAction(u"feed_stop"_s);
    stopAction->setIcon(QIcon::fromTheme(u"process-stop"_s));
    stopAction->setText(i18n("C&ancel Feed Fetches"));
    connect(stopAction, &QAction::triggered, Kernel::self()->fetchQueue(), &FetchQueue::slotAbort);
    coll->setDefaultShortcut(stopAction, QKeySequence(Qt::Key_Escape));
    stopAction->setEnabled(false);

    action = coll->addAction(u"feed_mark_all_as_read"_s);
    action->setIcon(QIcon::fromTheme(u"mail-mark-read"_s));
    action->setText(i18n("&Mark Feed as Read"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotMarkAllRead);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::Key_R));

    action = coll->addAction(u"feed_mark_all_feeds_as_read"_s);
    action->setIcon(QIcon::fromTheme(u"mail-mark-read"_s));
    action->setText(i18n("Ma&rk All Feeds as Read"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotMarkAllFeedsRead);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R));

    // Settings menu
    auto sqf = coll->add<KToggleAction>(u"show_quick_filter"_s);
    sqf->setText(i18n("Show Quick Filter"));
    connect(sqf, &QAction::triggered, d->mainWidget, &MainWidget::slotToggleShowQuickFilter);
    sqf->setChecked(Settings::showQuickFilter());

    action = coll->addAction(u"article_open"_s);
    action->setIcon(QIcon::fromTheme(u"tab-new"_s));
    action->setText(i18n("Open in Tab"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotOpenSelectedArticles);
    coll->setDefaultShortcut(action, QKeySequence(Qt::SHIFT | Qt::Key_Return));

    action = coll->addAction(u"article_open_in_background"_s);
    action->setIcon(QIcon::fromTheme(u"tab-new"_s));
    action->setText(i18n("Open in Background Tab"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotOpenSelectedArticlesInBackground);
    coll->setDefaultShortcut(action, QKeySequence(Qt::Key_Return));

    action = coll->addAction(u"article_open_external"_s);
    action->setIcon(QIcon::fromTheme(u"window-new"_s));
    action->setText(i18n("Open in External Browser"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotOpenSelectedArticlesInBrowser);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Return));

    action = coll->addAction(u"article_copy_link_address"_s);
    action->setText(i18nc("@action", "Copy Article Link Address"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotCopyLinkAddress);

    action = coll->addAction(u"go_prev_unread_article"_s);
    action->setIcon(QIcon::fromTheme(u"go-previous"_s));
    action->setText(i18n("Pre&vious Unread Article"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotPrevUnreadArticle);
    coll->setDefaultShortcuts(action, QList<QKeySequence>() << QKeySequence(Qt::Key_Minus) << QKeySequence(Qt::Key_Minus | Qt::KeypadModifier));

    action = coll->addAction(u"go_next_unread_article"_s);
    action->setIcon(QIcon::fromTheme(u"go-next"_s));
    action->setText(i18n("Ne&xt Unread Article"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotNextUnreadArticle);
    coll->setDefaultShortcuts(action,
                              QList<QKeySequence>() << QKeySequence(Qt::Key_Plus) << QKeySequence(Qt::Key_Plus | Qt::KeypadModifier)
                                                    << QKeySequence(Qt::Key_Equal) << QKeySequence(Qt::Key_Equal | Qt::KeypadModifier));

    action = coll->addAction(u"article_delete"_s);
    action->setIcon(QIcon::fromTheme(u"edit-delete"_s));
    action->setText(i18n("&Delete"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotArticleDelete);
    coll->setDefaultShortcut(action, QKeySequence(Qt::Key_Delete));

    auto statusMenu = coll->add<KActionMenu>(u"article_set_status"_s);
    statusMenu->setText(i18n("&Mark As"));
    statusMenu->setEnabled(false);

    action = coll->addAction(u"article_set_status_read"_s);
    action->setText(i18nc("as in: mark as read", "&Read"));
    action->setIcon(QIcon::fromTheme(u"mail-mark-read"_s));
    action->setToolTip(i18nc("@info:tooltip", "Mark selected article as read"));
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::Key_E));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotSetSelectedArticleRead);
    statusMenu->addAction(action);

    action = coll->addAction(u"article_set_status_new"_s);
    action->setText(i18nc("as in: mark as new", "&New"));
    action->setIcon(QIcon::fromTheme(u"mail-mark-unread-new"_s));
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::Key_N));
    action->setToolTip(i18nc("@info:tooltip", "Mark selected article as new"));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotSetSelectedArticleNew);
    statusMenu->addAction(action);

    action = coll->addAction(u"article_set_status_unread"_s);
    action->setText(i18nc("as in: mark as unread", "&Unread"));
    action->setIcon(QIcon::fromTheme(u"mail-mark-unread"_s));
    action->setToolTip(i18nc("@info:tooltip", "Mark selected article as unread"));
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::Key_U));
    connect(action, &QAction::triggered, d->mainWidget, &MainWidget::slotSetSelectedArticleUnread);
    statusMenu->addAction(action);

    auto importantAction = coll->add<KToggleAction>(u"article_set_status_important"_s);
    importantAction->setText(i18n("&Mark as Important"));
    importantAction->setIcon(QIcon::fromTheme(u"mail-mark-important"_s));
    const QList<QKeySequence> importantSC = {QKeySequence(Qt::CTRL | Qt::Key_I), QKeySequence(Qt::Key_I)};
    coll->setDefaultShortcuts(importantAction, importantSC);
    importantAction->setCheckedState(KGuiItem(i18nc("@action:button", "Remove &Important Mark")));
    connect(importantAction, &QAction::triggered, d->mainWidget, &MainWidget::slotArticleToggleKeepFlag);

    action = coll->addAction(u"feedstree_move_up"_s);
    action->setText(i18n("Move Node Up"));
    connect(action, &QAction::triggered, mainWidget, &MainWidget::slotMoveCurrentNodeUp);
    coll->setDefaultShortcut(action, QKeySequence(Qt::SHIFT | Qt::ALT | Qt::Key_Up));

    action = coll->addAction(u"feedstree_move_down"_s);
    action->setText(i18n("Move Node Down"));
    connect(action, &QAction::triggered, mainWidget, &MainWidget::slotMoveCurrentNodeDown);
    coll->setDefaultShortcut(action, QKeySequence(Qt::SHIFT | Qt::ALT | Qt::Key_Down));

    action = coll->addAction(u"move_node_left"_s);
    action->setText(i18n("Move Node Left"));
    connect(action, &QAction::triggered, mainWidget, &MainWidget::slotMoveCurrentNodeLeft);
    coll->setDefaultShortcut(action, QKeySequence(Qt::SHIFT | Qt::ALT | Qt::Key_Left));

    action = coll->addAction(u"feedstree_move_right"_s);
    action->setText(i18n("Move Node Right"));
    connect(action, &QAction::triggered, mainWidget, &MainWidget::slotMoveCurrentNodeRight);
    coll->setDefaultShortcut(action, QKeySequence(Qt::SHIFT | Qt::ALT | Qt::Key_Right));

    action = coll->addAction(u"file_sendlink"_s);
    action->setIcon(QIcon::fromTheme(u"mail-message-new"_s));
    action->setText(i18n("Send &Link Address…"));
    connect(action, &QAction::triggered, mainWidget, &MainWidget::slotSendLink);

    action = coll->addAction(u"file_sendfile"_s);
    action->setIcon(QIcon::fromTheme(u"mail-message-new"_s));
    action->setText(i18n("Send &File…"));
    connect(action, &QAction::triggered, mainWidget, &MainWidget::slotSendFile);

    coll->addAction(u"share_serviceurl"_s, d->shareServiceManager->menu());
    connect(d->shareServiceManager, &PimCommon::ShareServiceUrlManager::serviceUrlSelected, this, &ActionManagerImpl::slotServiceUrlSelected);

    d->mQuickSearchAction = new QAction(i18nc("@action", "Set Focus to Quick Search"), this);
    // If change shortcut change Panel::setQuickSearchClickMessage(…) message
    coll->setDefaultShortcut(d->mQuickSearchAction, QKeySequence(Qt::ALT | Qt::Key_Q));
    coll->addAction(u"focus_to_quickseach"_s, d->mQuickSearchAction);
    connect(d->mQuickSearchAction, &QAction::triggered, mainWidget, &MainWidget::slotFocusQuickSearch);

    auto showWhatsNewAction = new QAction(QIcon::fromTheme(u"akregator"_s), i18n("What's new"), this);
    coll->addAction(u"whatsnew"_s, showWhatsNewAction);
    connect(showWhatsNewAction, &QAction::triggered, mainWidget, &MainWidget::slotWhatsNew);

    auto manager = KColorSchemeManager::instance();
    coll->addAction(u"colorscheme_menu"_s, KColorSchemeMenu::createMenu(manager, this));

#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    const QString defaultUrlPath = u"https://origin.cdn.kde.org/ci-builds/pim/akregator/"_s;
    const QString stableBranch = u"26.04"_s;
    bool stableVersion = false;
#if AKREGATOR_STABLE_VERSION
    stableVersion = true;
#endif
    d->mVerifyNewVersionWidget->generateUrlInfo(stableBranch, defaultUrlPath, stableVersion);
    auto verifyNewVersionAction = d->mVerifyNewVersionWidget->verifyNewVersionAction();
    coll->addAction(u"verify_check_version"_s, verifyNewVersionAction);
#endif

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
    }
    d->articleList = articleList;

    QAction *action = actionCollection()->addAction(u"go_previous_article"_s);
    action->setText(i18n("&Previous Article"));
    connect(action, &QAction::triggered, articleList, &ArticleListView::slotPreviousArticle);
    actionCollection()->setDefaultShortcut(action, QKeySequence(Qt::Key_Left));
    action = actionCollection()->addAction(u"go_next_article"_s);
    action->setText(i18n("&Next Article"));
    connect(action, &QAction::triggered, articleList, &ArticleListView::slotNextArticle);
    actionCollection()->setDefaultShortcut(action, QKeySequence(Qt::Key_Right));
}

void ActionManagerImpl::initSubscriptionListView(SubscriptionListView *subscriptionListView)
{
    if (d->subscriptionListView) {
        return;
    }
    d->subscriptionListView = subscriptionListView;

    KActionCollection *coll = actionCollection();

    QAction *action = coll->addAction(u"go_prev_feed"_s);
    action->setText(i18n("&Previous Feed"));
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotPrevFeed);
    coll->setDefaultShortcut(action, QKeySequence(Qt::Key_P));

    action = coll->addAction(u"go_next_feed"_s);
    action->setText(i18n("&Next Feed"));
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotNextFeed);
    coll->setDefaultShortcut(action, QKeySequence(Qt::Key_N));

    action = coll->addAction(u"go_next_unread_feed"_s);
    action->setIcon(QIcon::fromTheme(u"go-down"_s));
    action->setText(i18n("N&ext Unread Feed"));
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotNextUnreadFeed);
    coll->setDefaultShortcut(action, QKeySequence(Qt::ALT | Qt::Key_Plus));

    action = coll->addAction(u"go_prev_unread_feed"_s);
    action->setIcon(QIcon::fromTheme(u"go-up"_s));
    action->setText(i18n("Prev&ious Unread Feed"));
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotPrevUnreadFeed);
    coll->setDefaultShortcut(action, QKeySequence(Qt::ALT | Qt::Key_Minus));

    action = coll->addAction(u"feedstree_home"_s);
    action->setText(i18n("Go to Top of Tree"));
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotItemBegin);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::Key_Home));

    action = coll->addAction(u"feedstree_end"_s);
    action->setText(i18n("Go to Bottom of Tree"));
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotItemEnd);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::Key_End));

    action = coll->addAction(u"feedstree_left"_s);
    action->setText(i18n("Go Left in Tree"));
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotItemLeft);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::Key_Left));

    action = coll->addAction(u"feedstree_right"_s);
    action->setText(i18n("Go Right in Tree"));
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotItemRight);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::Key_Right));

    action = coll->addAction(u"feedstree_up"_s);
    action->setText(i18n("Go Up in Tree"));
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotItemUp);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::Key_Up));

    action = coll->addAction(u"feedstree_down"_s);
    action->setText(i18n("Go Down in Tree"));
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotItemDown);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::Key_Down));

    action = coll->addAction(u"feed_hide_read"_s);
    action->setCheckable(true);
    action->setText(i18n("Hide Read Feeds"));
    action->setChecked(Settings::hideReadFeeds());
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotSetHideReadFeeds);

    action = coll->addAction(u"feed_lock_in_place"_s);
    action->setCheckable(true);
    action->setText(i18n("Lock Feeds In Place"));
    action->setChecked(Settings::lockFeedsInPlace());
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotSetLockFeedsInPlace);

    action = coll->addAction(u"auto_expand_folders"_s);
    action->setCheckable(true);
    action->setText(i18n("Auto-expand folders with unread articles"));
    action->setChecked(Settings::autoExpandFolders());
    connect(action, &QAction::triggered, subscriptionListView, &SubscriptionListView::slotSetAutoExpandFolders);
}

void ActionManagerImpl::initTabWidget(TabWidget *tabWidget)
{
    if (d->tabWidget) {
        return;
    }
    d->tabWidget = tabWidget;

    KActionCollection *coll = actionCollection();

    QAction *action = coll->addAction(u"select_next_tab"_s);
    action->setText(i18n("Select Next Tab"));
    connect(action, &QAction::triggered, d->tabWidget, &TabWidget::slotNextTab);
    coll->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::Key_Period));

    action = coll->addAction(u"select_previous_tab"_s);
    action->setText(i18n("Select Previous Tab"));
    connect(action, &QAction::triggered, d->tabWidget, &TabWidget::slotPreviousTab);
    coll->setDefaultShortcut(action, QKeySequence(Qt::Key_Comma | Qt::CTRL));

    action = KStandardActions::copy(d->tabWidget, &TabWidget::slotCopy, coll);
    coll->addAction(u"viewer_copy"_s, action);

    action = KStandardActions::print(d->tabWidget, &TabWidget::slotPrint, coll);
    coll->addAction(u"viewer_print"_s, action);

    action = KStandardActions::printPreview(d->tabWidget, &TabWidget::slotPrintPreview, coll);
    coll->addAction(u"viewer_printpreview"_s, action);

    action = coll->addAction(u"tab_mute"_s);
    action->setText(i18n("Mute"));
    connect(action, &QAction::triggered, d->tabWidget, &TabWidget::slotMute);

    action = coll->addAction(u"tab_unmute"_s);
    action->setText(i18n("Unmute"));
    connect(action, &QAction::triggered, d->tabWidget, &TabWidget::slotUnMute);

    action = new QAction(i18nc("@action", "Speak Text"), this);
    action->setIcon(QIcon::fromTheme(u"preferences-desktop-text-to-speech"_s));
    coll->addAction(u"speak_text"_s, action);
    connect(action, &QAction::triggered, d->tabWidget, &TabWidget::slotTextToSpeech);

    action = new QAction(QIcon::fromTheme(u"edit-find"_s), i18n("&Find in Message…"), this);
    coll->addAction(u"find_in_messages"_s, action);
    connect(action, &QAction::triggered, d->tabWidget, &TabWidget::slotFindTextInHtml);
    coll->setDefaultShortcut(action, KStandardShortcut::find().first());

    action = coll->addAction(u"tab_copyurl"_s);
    action->setIcon(QIcon::fromTheme(u"edit-copy"_s));
    action->setText(i18nc("@action", "Copy Tab Address"));
    connect(action, &QAction::triggered, d->tabWidget, &TabWidget::slotCopyTabAddress);

    action = coll->addAction(u"tab_remove"_s);
    action->setIcon(QIcon::fromTheme(u"tab-close"_s));
    action->setText(i18n("Close Tab"));
    connect(action, &QAction::triggered, d->tabWidget, &TabWidget::slotCloseTab);
    coll->setDefaultShortcuts(action, KStandardShortcut::close());

    d->zoomActionMenu = new WebEngineViewer::ZoomActionMenu(this);
    connect(d->zoomActionMenu, &WebEngineViewer::ZoomActionMenu::zoomChanged, d->tabWidget, &TabWidget::slotZoomChanged);

    d->zoomActionMenu->setActionCollection(coll);
    d->zoomActionMenu->createZoomActions();

    QString actionname;
    for (int i = 1; i < 10; ++i) {
        actionname = QString::asprintf("activate_tab_%02d", i);
        action = new QAction(i18nc("@action", "Activate Tab %1", i), this);
        coll->addAction(actionname, action);
        coll->setDefaultShortcut(action, QKeySequence(u"Alt+%1"_s.arg(i)));
        connect(action, &QAction::triggered, d->tabWidget, &TabWidget::slotActivateTab);
    }
}

QWidget *ActionManagerImpl::container(const QString &name)
{
    if (d->part->factory()) {
        return d->part->factory()->container(name, d->part);
    } else {
        return nullptr;
    }
}

KActionCollection *ActionManagerImpl::actionCollection() const
{
    return d->actionCollection;
}

QAction *ActionManagerImpl::action(const QString &name)
{
    return d->actionCollection ? d->actionCollection->action(name) : nullptr;
}

void ActionManagerImpl::setArticleActionsEnabled(bool enabled){
#undef setActionEnabled
#define setActionEnabled(name)                                                                                                                                 \
    {                                                                                                                                                          \
        QAction *const a = action(name);                                                                                                                       \
        if (a) {                                                                                                                                               \
            a->setEnabled(enabled);                                                                                                                            \
        }                                                                                                                                                      \
    }
    setActionEnabled(u"article_open"_s) setActionEnabled(u"article_open_external"_s) setActionEnabled(u"article_set_status_important"_s)
        setActionEnabled(u"article_set_status"_s) setActionEnabled(u"article_delete"_s) setActionEnabled(u"file_sendlink"_s)
            setActionEnabled(u"file_sendfile"_s) setActionEnabled(u"article_open_in_background"_s) setActionEnabled(u"share_serviceurl"_s)
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
    return mQuickSearchAction->shortcut().toString(QKeySequence::NativeText);
}

#include "moc_actionmanagerimpl.cpp"
