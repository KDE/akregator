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

class QAction;
class KActionCollection;

class QWidget;

namespace Akregator
{

class ArticleListView;
class ArticleViewer;
class FrameManager;
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

    explicit ActionManagerImpl(Part *part, QObject *parent = 0);
    virtual ~ActionManagerImpl();

    virtual QAction *action(const char *name);
    virtual QWidget *container(const char *name);

    void initMainWidget(MainWidget *mainWidget);
    void initArticleViewer(ArticleViewer *articleViewer);
    void initArticleListView(ArticleListView *articleList);
    void initSubscriptionListView(SubscriptionListView *subscriptionListView);
    void initTabWidget(TabWidget *tabWidget);
    void initFrameManager(FrameManager *frameManager);

    void setArticleActionsEnabled(bool enabled);
    void setTrayIcon(TrayIcon *trayIcon);

public slots:

    void slotNodeSelected(Akregator::TreeNode *node);

protected:

    KActionCollection *actionCollection();

private:

    void initPart();

    friend class NodeSelectVisitor;
    class NodeSelectVisitor;

    class ActionManagerImplPrivate;
    ActionManagerImplPrivate *d;
};

} // namespace Akregator

#endif // AKREGATOR_ACTIONMANAGERIMPL_H
