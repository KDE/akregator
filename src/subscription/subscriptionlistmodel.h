/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2007 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#pragma once

#include "akregatorpart_export.h"

#include <QAbstractItemModel>
#include <QSet>
#include <QSortFilterProxyModel>

#include <QColor>
#include <QSharedPointer>

namespace Akregator
{
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

    Q_REQUIRED_RESULT bool doFilter() const;

    void setDoFilter(bool v);

    void setSourceModel(QAbstractItemModel *src) override;

public Q_SLOTS:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void clearCache();

private:
    Q_REQUIRED_RESULT bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    using SelectionHierarchy = QSet<QModelIndex>;

    bool m_doFilter = false;
    SelectionHierarchy m_selectedHierarchy;
};

class AKREGATORPART_EXPORT SubscriptionListModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Role { SubscriptionIdRole = Qt::UserRole, IsFetchableRole, IsGroupRole, IsAggregationRole, LinkRole, IdRole, IsOpenRole, HasUnreadRole };

    enum Column { TitleColumn = 0, UnreadCountColumn = 1, TotalCountColumn = 2, ColumnCount = 3 };

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

    QColor m_errorColor;
};
}

namespace Akregator
{
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
