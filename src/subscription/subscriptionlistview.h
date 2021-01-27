/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2007 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef AKREGATOR_SUBSCRIPTIONLISTVIEW_H
#define AKREGATOR_SUBSCRIPTIONLISTVIEW_H

#include <QAction>
#include <QByteArray>
#include <QTreeView>

namespace Akregator
{
class TreeNode;

class SubscriptionListView : public QTreeView
{
    Q_OBJECT
public:
    explicit SubscriptionListView(QWidget *parent = nullptr);
    ~SubscriptionListView() override;
    // the following is all transitional, for easier porting from the item-based views

    void startNodeRenaming(TreeNode *node);

    void ensureNodeVisible(TreeNode *node);

    // override
    void setModel(QAbstractItemModel *model) override;

    void triggerUpdate()
    {
    }

    enum Column { TitleColumn = 0, UnreadColumn = 1, TotalColumn = 2 };

public Q_SLOTS:

    void slotPrevFeed();
    void slotNextFeed();

    void slotPrevUnreadFeed();
    void slotNextUnreadFeed();

    void slotItemBegin();
    void slotItemEnd();
    void slotItemLeft();
    void slotItemRight();
    void slotItemUp();
    void slotItemDown();

    void slotSetHideReadFeeds(bool setting);
    void slotSetAutoExpandFolders(bool setting);

Q_SIGNALS:
    void userActionTakingPlace();

private:
    void saveHeaderSettings();
    void loadHeaderSettings();
    void restoreHeaderState();

private:
    void showHeaderMenu(const QPoint &pos);
    void headerMenuItemTriggered(QAction *action);

    QByteArray m_headerState;
};
}

#endif // AKREGATOR_SUBSCRIPTIONLISTVIEW_H
