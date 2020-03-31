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

#ifndef AKREGATOR_ACTIONMANAGERIMPL_H
#define AKREGATOR_ACTIONMANAGERIMPL_H

#include "actionmanager.h"
#include <PimCommon/ShareServiceUrlManager>
class QAction;
class KActionCollection;

class QWidget;

namespace WebEngineViewer {
class ZoomActionMenu;
}

namespace Akregator {
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

    QAction *action(const QString &name) override;
    QWidget *container(const QString &name) override;

    void initMainWidget(MainWidget *mainWidget);
    void initArticleListView(ArticleListView *articleList);
    void initSubscriptionListView(SubscriptionListView *subscriptionListView);
    void initTabWidget(TabWidget *tabWidget);

    void setArticleActionsEnabled(bool enabled) override;
    void setTrayIcon(TrayIcon *trayIcon);
    KActionCollection *actionCollection() const;
    WebEngineViewer::ZoomActionMenu *zoomActionMenu() const override;
    QString quickSearchLineText() const;

public Q_SLOTS:

    void slotNodeSelected(Akregator::TreeNode *node);
    void slotSettingsChanged();

private Q_SLOTS:
    void slotServiceUrlSelected(PimCommon::ShareServiceUrlManager::ServiceType type);

private:

    void initPart();

    friend class NodeSelectVisitor;
    class NodeSelectVisitor;

    class ActionManagerImplPrivate;
    ActionManagerImplPrivate *d;
};
} // namespace Akregator

#endif // AKREGATOR_ACTIONMANAGERIMPL_H
