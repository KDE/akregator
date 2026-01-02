/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#pragma once

#include "actionmanager.h"
#include <PimCommon/ShareServiceUrlManager>

#include <memory>

class QAction;
class KActionCollection;

class QWidget;

namespace WebEngineViewer
{
class ZoomActionMenu;
}

namespace Akregator
{
class ArticleListView;
class MainWidget;
class Part;
class SubscriptionListView;
class TabWidget;
class TreeNode;
class TrayIcon;

/**
 * Akregator-specific implementation of the ActionManager interface
 */
class ActionManagerImpl : public ActionManager
{
    Q_OBJECT

public:
    explicit ActionManagerImpl(Part *part, QObject *parent = nullptr);
    ~ActionManagerImpl() override;

    [[nodiscard]] QAction *action(const QString &name) override;
    [[nodiscard]] QWidget *container(const QString &name) override;

    void initMainWidget(MainWidget *mainWidget);
    void initArticleListView(ArticleListView *articleList);
    void initSubscriptionListView(SubscriptionListView *subscriptionListView);
    void initTabWidget(TabWidget *tabWidget);

    void setArticleActionsEnabled(bool enabled) override;
    void setTrayIcon(TrayIcon *trayIcon);
    [[nodiscard]] KActionCollection *actionCollection() const;
    [[nodiscard]] WebEngineViewer::ZoomActionMenu *zoomActionMenu() const override;
    [[nodiscard]] QString quickSearchLineText() const;

public Q_SLOTS:
    void slotNodeSelected(Akregator::TreeNode *node);
    void slotSettingsChanged();

private:
    void slotServiceUrlSelected(PimCommon::ShareServiceUrlManager::ServiceType type);
    void initPart();

    friend class NodeSelectVisitor;
    class NodeSelectVisitor;

    class ActionManagerImplPrivate;
    std::unique_ptr<ActionManagerImplPrivate> const d;
};
} // namespace Akregator
