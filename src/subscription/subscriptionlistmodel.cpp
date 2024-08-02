/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2007 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "subscriptionlistmodel.h"
#include "akregatorconfig.h"
#include "config-akregator.h"
#include "feed.h"
#include "feedlist.h"
#include "folder.h"
#include "kernel.h"
#include "subscriptionlistjobs.h"
#include "treenode.h"

#include "akregator_debug.h"
#include <KLocalizedString>

#include <KColorScheme>
#include <QApplication>
#include <QByteArray>
#include <QDataStream>
#include <QIODevice>
#include <QIcon>
#include <QItemSelection>
#include <QList>
#include <QMimeData>
#include <QPalette>
#include <QStyle>
#include <QUrl>
#include <QVariant>
#if HAVE_ACTIVITY_SUPPORT
#include "activities/activitiesmanager.h"
#endif
using namespace Akregator;
using namespace Syndication;

#define AKREGATOR_TREENODE_MIMETYPE QStringLiteral("akregator/treenode-id")

namespace
{
static uint nodeIdForIndex(const QModelIndex &idx)
{
    return idx.isValid() ? idx.internalId() : 0;
}

static QString errorCodeToString(Syndication::ErrorCode err)
{
    switch (err) {
    case Timeout:
        return i18n("Timeout on remote server");
    case UnknownHost:
        return i18n("Unknown host");
    case FileNotFound:
        return i18n("Feed file not found on remote server");
    case InvalidXml:
        return i18n("Could not read feed (invalid XML)");
    case XmlNotAccepted:
        return i18n("Could not read feed (unknown format)");
    case InvalidFormat:
        return i18n("Could not read feed (invalid feed)");
    case Success:
    case Aborted:
    default:
        return {};
    }
}

static const TreeNode *nodeForIndex(const QModelIndex &index, const FeedList *feedList)
{
    return (!index.isValid() || !feedList) ? nullptr : feedList->findByID(index.internalId());
}
}
// TODO add activities support here.
FilterSubscriptionProxyModel::FilterSubscriptionProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_selectedHierarchy()
{
}

bool FilterSubscriptionProxyModel::doFilter() const
{
    return m_doFilter;
}

void FilterSubscriptionProxyModel::activitiesChanged()
{
    invalidateFilter();
}

void FilterSubscriptionProxyModel::setDoFilter(bool v)
{
    m_doFilter = v;
    invalidateFilter();
}

void FilterSubscriptionProxyModel::setSourceModel(QAbstractItemModel *src)
{
    clearCache();
    QSortFilterProxyModel::setSourceModel(src);
}

bool FilterSubscriptionProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
#if HAVE_ACTIVITY_SUPPORT
    if (Kernel::self()->activitiesManager()->enabled()) {
        if (idx.data(SubscriptionListModel::ActivityEnabled).toBool()) {
            if (!Kernel::self()->activitiesManager()->isInCurrentActivity(idx.data(SubscriptionListModel::Activities).toStringList())) {
                return false;
            }
        }
    }
#endif

    if (!m_doFilter) {
        return true;
    }

    if (m_selectedHierarchy.contains(idx)) {
        return true;
    }

    const QVariant v = idx.data(SubscriptionListModel::HasUnreadRole);
    if (v.isNull()) {
        return true;
    }

    return v.toBool();
}

/**
 * This caches the hierarchy of the selected node. Its purpose is to allow
 * feeds/folders with no unread content not to be filtered out immediately,
 * which would occur otherwise (we'd select the last article to read, it would
 * become unread, and disappear from the list without letting us view it).
 **/
void FilterSubscriptionProxyModel::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QModelIndexList desel = mapSelectionToSource(deselected).indexes();
    // calling invalidateFilter causes refiltering at the call point, so we should
    // call it ONLY after we recreate our node cache
    bool doInvalidate = false;

    // if we're deselecting an empty feed/folder, we need to hide it
    if (!desel.isEmpty()) {
        if (m_selectedHierarchy.contains(desel.at(0))) {
            doInvalidate = true;
        }
    }

    clearCache();

    const QModelIndexList sel = mapSelectionToSource(selected).indexes();
    if (!sel.isEmpty()) {
        // XXX add support for multiple selections? this doesn't generally make sense in this case honestly
        QModelIndex current = sel.constFirst();
        while (current.isValid()) {
            m_selectedHierarchy.insert(current);
            current = current.parent();
        }
    }

    if (doInvalidate && doFilter()) {
        invalidateFilter();
    }
}

void FilterSubscriptionProxyModel::clearCache()
{
    m_selectedHierarchy.clear();
}

SubscriptionListModel::SubscriptionListModel(const QSharedPointer<const FeedList> &feedList, QObject *parent)
    : QAbstractItemModel(parent)
    , m_feedList(feedList)
{
    if (!m_feedList) {
        return;
    }
    connect(m_feedList.data(), &FeedList::signalNodeAdded, this, &SubscriptionListModel::subscriptionAdded);
    connect(m_feedList.data(), &FeedList::signalAboutToRemoveNode, this, &SubscriptionListModel::aboutToRemoveSubscription);
    connect(m_feedList.data(), &FeedList::signalNodeRemoved, this, &SubscriptionListModel::subscriptionRemoved);
    connect(m_feedList.data(), &FeedList::signalNodeChanged, this, &SubscriptionListModel::subscriptionChanged);
    connect(m_feedList.data(), &FeedList::fetchStarted, this, &SubscriptionListModel::fetchStarted);
    connect(m_feedList.data(), &FeedList::fetched, this, &SubscriptionListModel::fetched);
    connect(m_feedList.data(), &FeedList::fetchAborted, this, &SubscriptionListModel::fetchAborted);

    m_errorColor = KColorScheme(QPalette::Normal, KColorScheme::View).foreground(KColorScheme::NegativeText).color();
}

int SubscriptionListModel::columnCount(const QModelIndex &) const
{
    return 3;
}

int SubscriptionListModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return 1;
    }

    const TreeNode *const node = nodeForIndex(parent, m_feedList.data());
    return node ? node->children().count() : 0;
}

QVariant SubscriptionListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    const TreeNode *const node = nodeForIndex(index, m_feedList.data());

    if (!node) {
        return {};
    }

    const Feed *const feed = qobject_cast<const Feed *const>(node);

    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        switch (index.column()) {
        case TitleColumn:
            return node->title();
        case UnreadCountColumn:
            return node->unread();
        case TotalCountColumn:
            return node->totalCount();
        }
        break;
    case Qt::ForegroundRole:
        return feed && feed->fetchErrorCode() ? m_errorColor : QApplication::palette().color(QPalette::Text);
    case Qt::ToolTipRole: {
        if (node->isGroup() || node->isAggregation()) {
            return node->title();
        }
        if (!feed) {
            return QString();
        }
        if (feed->fetchErrorOccurred()) {
            return i18n("Could not fetch feed: %1", errorCodeToString(feed->fetchErrorCode()));
        }
        return feed->title();
    }
    case Qt::DecorationRole: {
        if (index.column() != TitleColumn) {
            return {};
        }
        const auto iconSize = QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize);
        return feed && feed->isFetching() ? node->icon().pixmap(iconSize, QIcon::Active) : node->icon();
    }
    case SubscriptionIdRole:
        return node->id();
    case IsGroupRole:
        return node->isGroup();
    case IsFetchableRole:
        return !node->isGroup() && !node->isAggregation();
    case IsAggregationRole:
        return node->isAggregation();
    case LinkRole: {
        return feed ? feed->xmlUrl() : QVariant();
    }
    case IsOpenRole: {
        if (!node->isGroup()) {
            return false;
        }
        const auto *const folder = qobject_cast<const Folder *const>(node);
        Q_ASSERT(folder);
        return folder->isOpen();
    }
    case HasUnreadRole:
        return node->unread() > 0;
    case Activities:
        return feed ? feed->activities() : QVariant();
    case ActivityEnabled:
        return feed ? feed->activityEnabled() : QVariant();
    }

    return {};
}

QVariant SubscriptionListModel::headerData(int section, Qt::Orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return {};
    }

    switch (section) {
    case TitleColumn:
        return i18nc("Feedlist's column header", "Feeds");
    case UnreadCountColumn:
        return i18nc("Feedlist's column header", "Unread");
    case TotalCountColumn:
        return i18nc("Feedlist's column header", "Total");
    }

    return {};
}

QModelIndex SubscriptionListModel::parent(const QModelIndex &index) const
{
    const TreeNode *const node = nodeForIndex(index, m_feedList.data());

    if (!node || !node->parent()) {
        return {};
    }

    const Folder *parent = node->parent();

    if (!parent->parent()) {
        return createIndex(0, 0, parent->id());
    }

    const Folder *const grandparent = parent->parent();

    const int row = grandparent->indexOf(parent);

    Q_ASSERT(row != -1);

    return createIndex(row, 0, parent->id());
}

QModelIndex SubscriptionListModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return (row == 0 && m_feedList) ? createIndex(row, column, m_feedList->allFeedsFolder()->id()) : QModelIndex();
    }

    const TreeNode *const parentNode = nodeForIndex(parent, m_feedList.data());

    if (!parentNode) {
        return {};
    }

    const TreeNode *const childNode = parentNode->childAt(row);
    return childNode ? createIndex(row, column, childNode->id()) : QModelIndex();
}

QModelIndex SubscriptionListModel::indexForNode(const TreeNode *node) const
{
    if (!node || !m_feedList) {
        return {};
    }
    const Folder *const parent = node->parent();
    if (!parent) {
        return index(0, 0);
    }
    const int row = parent->indexOf(node);
    Q_ASSERT(row >= 0);
    const QModelIndex idx = index(row, 0, indexForNode(parent));
    Q_ASSERT(idx.internalId() == node->id());
    return idx;
}

void SubscriptionListModel::subscriptionAdded(TreeNode *subscription)
{
    const Folder *const parent = subscription->parent();
    const int row = parent ? parent->indexOf(subscription) : 0;
    Q_ASSERT(row >= 0);
    beginInsertRows(indexForNode(parent), row, row);
    endInsertRows();
}

void SubscriptionListModel::aboutToRemoveSubscription(TreeNode *subscription)
{
    qCDebug(AKREGATOR_LOG) << subscription->id();
    const Folder *const parent = subscription->parent();
    const int row = parent ? parent->indexOf(subscription) : -1;
    if (row < 0) {
        return;
    }
    beginRemoveRows(indexForNode(parent), row, row);
    m_beganRemoval = true;
}

void SubscriptionListModel::subscriptionRemoved(TreeNode *subscription)
{
    qCDebug(AKREGATOR_LOG) << subscription->id();
    if (m_beganRemoval) {
        m_beganRemoval = false;
        endRemoveRows();
    }
}

void SubscriptionListModel::subscriptionChanged(TreeNode *node)
{
    const QModelIndex idx = indexForNode(node);
    if (!idx.isValid()) {
        return;
    }
    Q_EMIT dataChanged(index(idx.row(), 0, idx.parent()), index(idx.row(), ColumnCount - 1, idx.parent()));
}

void SubscriptionListModel::fetchStarted(Feed *node)
{
    subscriptionChanged(node);
}

void SubscriptionListModel::fetched(Feed *node)
{
    subscriptionChanged(node);
}

void SubscriptionListModel::fetchError(Feed *node)
{
    subscriptionChanged(node);
}

void SubscriptionListModel::fetchAborted(Feed *node)
{
    subscriptionChanged(node);
}

void FolderExpansionHandler::itemExpanded(const QModelIndex &idx)
{
    setExpanded(idx, true);
}

void FolderExpansionHandler::itemCollapsed(const QModelIndex &idx)
{
    setExpanded(idx, false);
}

void FolderExpansionHandler::setExpanded(const QModelIndex &idx, bool expanded)
{
    if (!m_feedList || !m_model) {
        return;
    }
    TreeNode *const node = m_feedList->findByID(nodeIdForIndex(idx));
    if (!node || !node->isGroup()) {
        return;
    }

    auto const folder = qobject_cast<Folder *>(node);
    Q_ASSERT(folder);
    folder->setOpen(expanded);
}

FolderExpansionHandler::FolderExpansionHandler(QObject *parent)
    : QObject(parent)
    , m_feedList()
{
}

void FolderExpansionHandler::setModel(QAbstractItemModel *model)
{
    m_model = model;
}

void FolderExpansionHandler::setFeedList(const QSharedPointer<FeedList> &feedList)
{
    m_feedList = feedList;
}

Qt::ItemFlags SubscriptionListModel::flags(const QModelIndex &idx) const
{
    const Qt::ItemFlags flags = QAbstractItemModel::flags(idx);
    if (!idx.isValid() || (idx.column() != TitleColumn)) {
        return flags;
    }
    if (!idx.parent().isValid()) { // the root folder is neither draggable nor editable
        return flags | Qt::ItemIsDropEnabled;
    }
    return flags | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable;
}

QStringList SubscriptionListModel::mimeTypes() const
{
    return {QStringLiteral("text/uri-list"), AKREGATOR_TREENODE_MIMETYPE};
}

QMimeData *SubscriptionListModel::mimeData(const QModelIndexList &indexes) const
{
    auto mimeData = new QMimeData;

    QList<QUrl> urls;
    for (const QModelIndex &i : indexes) {
        const QUrl url(i.data(LinkRole).toString());
        if (!url.isEmpty()) {
            urls << url;
        }
    }

    mimeData->setUrls(urls);

    QByteArray idList;
    QDataStream idStream(&idList, QIODevice::WriteOnly);
    for (const QModelIndex &i : indexes) {
        if (i.isValid()) {
            idStream << i.data(SubscriptionIdRole).toInt();
        }
    }

    mimeData->setData(AKREGATOR_TREENODE_MIMETYPE, idList);

    return mimeData;
}

bool SubscriptionListModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if (!idx.isValid() || idx.column() != TitleColumn || role != Qt::EditRole) {
        return false;
    }
    const TreeNode *const node = nodeForIndex(idx, m_feedList.data());
    if (!node) {
        return false;
    }
    auto job = new RenameSubscriptionJob(this);
    job->setSubscriptionId(node->id());
    job->setName(value.toString());
    job->start();
    return true;
}

bool SubscriptionListModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(column)

    if (action == Qt::IgnoreAction) {
        return true;
    }

    // if ( column != TitleColumn )
    //    return false;

    if (!data->hasFormat(AKREGATOR_TREENODE_MIMETYPE)) {
        return false;
    }

    const auto *const droppedOnNode = qobject_cast<const TreeNode *>(nodeForIndex(parent, m_feedList.data()));

    if (!droppedOnNode) {
        return false;
    }

    const Folder *const destFolder = droppedOnNode->isGroup() ? qobject_cast<const Folder *>(droppedOnNode) : droppedOnNode->parent();
    if (!destFolder) {
        return false;
    }

    QByteArray idData = data->data(AKREGATOR_TREENODE_MIMETYPE);
    QList<int> ids;
    QDataStream stream(&idData, QIODevice::ReadOnly);
    while (!stream.atEnd()) {
        int id;
        stream >> id;
        ids << id;
    }

    // don't drop nodes into their own subtree
    for (const int id : std::as_const(ids)) {
        const auto *const asFolder = qobject_cast<const Folder *>(m_feedList->findByID(id));
        if (asFolder && (asFolder == destFolder || asFolder->subtreeContains(destFolder))) {
            return false;
        }
    }

    const TreeNode *const after = droppedOnNode->isGroup() ? destFolder->childAt(row) : droppedOnNode;

    for (const int id : std::as_const(ids)) {
        const TreeNode *const node = m_feedList->findByID(id);
        if (!node) {
            continue;
        }
        auto job = new MoveSubscriptionJob(this);
        job->setSubscriptionId(node->id());
        job->setDestination(destFolder->id(), after ? after->id() : 0);
        job->start();
    }

    return true;
}

#include "moc_subscriptionlistmodel.cpp"
