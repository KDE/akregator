/*
    This file is part of Akregator.

    Copyright (C) 2007 Frank Osterfeld <osterfeld@kde.org>

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
#ifndef AKREGATOR_SUBSCRIPTIONLISTMODEL_H
#define AKREGATOR_SUBSCRIPTIONLISTMODEL_H

#include "akregatorpart_export.h"

#include <QAbstractItemModel>
#include <QSet>
#include <QSortFilterProxyModel>

#include <QSharedPointer>

namespace Akregator {
class Feed;
class FeedList;
class Folder;
class TreeNode;

/**
 * Filters feeds with unread counts.
 */
class FilterUnreadProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit FilterUnreadProxyModel(QObject *parent = nullptr);

    bool doFilter() const;

    void setDoFilter(bool v);

    void setSourceModel(QAbstractItemModel *src) override;

public slots:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void clearCache();

private:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    typedef QSet<QModelIndex> SelectionHierarchy;

    bool m_doFilter;
    SelectionHierarchy m_selectedHierarchy;
};

class AKREGATORPART_EXPORT SubscriptionListModel : public QAbstractItemModel
{
    Q_OBJECT
public:

    enum Role {
        SubscriptionIdRole = Qt::UserRole,
        IsFetchableRole,
        IsGroupRole,
        IsAggregationRole,
        LinkRole,
        IdRole,
        IsOpenRole,
        HasUnreadRole
    };

    enum Column {
        TitleColumn = 0,
        UnreadCountColumn = 1,
        TotalCountColumn = 2,
        ColumnCount = 3
    };

    explicit SubscriptionListModel(const QSharedPointer<const FeedList> &feedList, QObject *parent = nullptr);

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &index) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QStringList mimeTypes() const override;

    QMimeData *mimeData(const QModelIndexList &indexes) const override;

    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

    bool setData(const QModelIndex &idx, const QVariant &value, int role = Qt::EditRole) override;

private:
    QModelIndex indexForNode(const TreeNode *node) const;

private Q_SLOTS:

    void subscriptionAdded(Akregator::TreeNode *);

    void aboutToRemoveSubscription(Akregator::TreeNode *);

    void subscriptionRemoved(Akregator::TreeNode *);

    void subscriptionChanged(Akregator::TreeNode *);

    void fetchStarted(Akregator::Feed *);

    void fetched(Akregator::Feed *);

    void fetchError(Akregator::Feed *);

    void fetchAborted(Akregator::Feed *);

private:

    QSharedPointer<const FeedList> m_feedList;
    bool m_beganRemoval;
};
}

namespace Akregator {
class AKREGATORPART_EXPORT FolderExpansionHandler : public QObject
{
    Q_OBJECT

public:
    explicit FolderExpansionHandler(QObject *parent = nullptr);

    void setFeedList(const QSharedPointer<FeedList> &feedList);
    void setModel(QAbstractItemModel *model);

public Q_SLOTS:
    void itemExpanded(const QModelIndex &index);
    void itemCollapsed(const QModelIndex &index);

private:
    void setExpanded(const QModelIndex &index, bool expanded);

private:
    QSharedPointer<FeedList> m_feedList;
    QAbstractItemModel *m_model = nullptr;
};
} // namespace Akregator

#endif // AKREGATOR_SUBSCRIPTIONLISTMODEL_H
