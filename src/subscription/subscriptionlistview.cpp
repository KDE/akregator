/*
    This file is part of Akregator.

    Copyright (C) 2007 Frank Osterfeld <frank.osterfeld@kdemail.net>

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

#include "subscriptionlistview.h"
#include "subscriptionlistmodel.h"
#include "subscriptionlistdelegate.h"
#include "akregatorconfig.h"
#include "akregator_debug.h"

#include <QHeaderView>
#include <QStack>
#include <QPointer>
#include <QQueue>

#include <QMenu>
#include <KLocalizedString>
#include <KConfigGroup>

using namespace Akregator;

static QModelIndex prevIndex(const QModelIndex &idx)
{
    if (!idx.isValid()) {
        return QModelIndex();
    }
    const QAbstractItemModel *const model = idx.model();
    Q_ASSERT(model);

    if (idx.row() > 0) {
        QModelIndex i = idx.sibling(idx.row() - 1, idx.column());
        while (model->hasChildren(i)) {
            i = model->index(model->rowCount(i) - 1, i.column(), i);
        }
        return i;
    } else {
        return idx.parent();
    }
}

static QModelIndex prevFeedIndex(const QModelIndex &idx, bool allowPassed = false)
{
    QModelIndex prev = allowPassed ? idx : prevIndex(idx);
    while (prev.isValid() && prev.data(SubscriptionListModel::IsAggregationRole).toBool()) {
        prev = prevIndex(prev);
    }
    return prev;
}

static QModelIndex prevUnreadFeedIndex(const QModelIndex &idx, bool allowPassed = false)
{
    QModelIndex prev = allowPassed ? idx : prevIndex(idx);
    while (prev.isValid() && (prev.data(SubscriptionListModel::IsAggregationRole).toBool() || prev.sibling(prev.row(), SubscriptionListModel::UnreadCountColumn).data().toInt() == 0)) {
        prev = prevIndex(prev);
    }
    return prev;
}

static QModelIndex lastLeaveChild(const QAbstractItemModel *const model)
{
    Q_ASSERT(model);
    if (model->rowCount() == 0) {
        return QModelIndex();
    }
    QModelIndex idx = model->index(model->rowCount() - 1, 0);
    while (model->hasChildren(idx)) {
        idx = model->index(model->rowCount(idx) - 1, idx.column(), idx);
    }
    return idx;
}

static QModelIndex nextIndex(const QModelIndex &idx)
{
    if (!idx.isValid()) {
        return QModelIndex();
    }
    const QAbstractItemModel *const model = idx.model();
    Q_ASSERT(model);
    if (model->hasChildren(idx)) {
        return model->index(0, idx.column(), idx);
    }
    QModelIndex i = idx;
    while (true) {
        if (!i.isValid()) {
            return i;
        }
        const int siblings = model->rowCount(i.parent());
        if (i.row() + 1 < siblings) {
            return i.sibling(i.row() + 1, i.column());
        }
        i = i.parent();
    }
}

static QModelIndex nextFeedIndex(const QModelIndex &idx)
{
    QModelIndex next = nextIndex(idx);
    while (next.isValid() && next.data(SubscriptionListModel::IsAggregationRole).toBool()) {
        next = nextIndex(next);
    }
    return next;
}

static QModelIndex nextUnreadFeedIndex(const QModelIndex &idx)
{
    QModelIndex next = nextIndex(idx);
    while (next.isValid() && (next.data(SubscriptionListModel::IsAggregationRole).toBool() || next.sibling(next.row(), SubscriptionListModel::UnreadCountColumn).data().toInt() == 0)) {
        next = nextIndex(next);
    }
    return next;
}

Akregator::SubscriptionListView::SubscriptionListView(QWidget *parent) : QTreeView(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setRootIsDecorated(false);
    setAlternatingRowColors(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDropIndicatorShown(true);
    setAcceptDrops(true);
    setUniformRowHeights(true);
    setItemDelegate(new SubscriptionListDelegate(this));
    connect(header(), &QWidget::customContextMenuRequested, this, &SubscriptionListView::showHeaderMenu);

    loadHeaderSettings();
}

Akregator::SubscriptionListView::~SubscriptionListView()
{
    saveHeaderSettings();
}

void Akregator::SubscriptionListView::setModel(QAbstractItemModel *m)
{
    Q_ASSERT(m);

    if (model()) {
        m_headerState = header()->saveState();
    }

    QTreeView::setModel(m);

    restoreHeaderState();

    QStack<QModelIndex> stack;
    stack.push(rootIndex());
    while (!stack.isEmpty()) {
        const QModelIndex i = stack.pop();
        const int childCount = m->rowCount(i);
        for (int j = 0; j < childCount; ++j) {
            const QModelIndex child = m->index(j, 0, i);
            if (child.isValid()) {
                stack.push(child);
            }
        }
        setExpanded(i, i.data(Akregator::SubscriptionListModel::IsOpenRole).toBool());
    }

    header()->setContextMenuPolicy(Qt::CustomContextMenu);
}

void Akregator::SubscriptionListView::showHeaderMenu(const QPoint &pos)
{
    if (!model()) {
        return;
    }

    QPointer<QMenu> menu = new QMenu(this);
    menu->setTitle(i18n("Columns"));
    menu->setAttribute(Qt::WA_DeleteOnClose);
    connect(menu.data(), &QMenu::triggered, this, &SubscriptionListView::headerMenuItemTriggered);

    for (int i = 0; i < model()->columnCount(); ++i) {
        if (SubscriptionListModel::TitleColumn == i) {
            continue;
        }
        QString col = model()->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();
        QAction *act = menu->addAction(col);
        act->setCheckable(true);
        act->setChecked(!header()->isSectionHidden(i));
        act->setData(i);
    }

    menu->popup(header()->mapToGlobal(pos));
}

void Akregator::SubscriptionListView::headerMenuItemTriggered(QAction *act)
{
    Q_ASSERT(act);
    const int col = act->data().toInt();
    if (act->isChecked()) {
        header()->showSection(col);
    } else {
        header()->hideSection(col);
    }
}

void Akregator::SubscriptionListView::saveHeaderSettings()
{
    if (model()) {
        m_headerState = header()->saveState();
    }
    KConfigGroup conf(Settings::self()->config(), "General");
    conf.writeEntry("SubscriptionListHeaders", m_headerState.toBase64());
}

void Akregator::SubscriptionListView::loadHeaderSettings()
{
    const KConfigGroup conf(Settings::self()->config(), "General");
    m_headerState = QByteArray::fromBase64(conf.readEntry("SubscriptionListHeaders").toLatin1());
    restoreHeaderState();
}

void Akregator::SubscriptionListView::restoreHeaderState()
{
    header()->restoreState(m_headerState);                  // needed, even with Qt 4.5
    // Always shows the title column
    header()->showSection(SubscriptionListModel::TitleColumn);
    if (m_headerState.isEmpty()) {
        // Default configuration: only show the title column
        header()->hideSection(SubscriptionListModel::UnreadCountColumn);
        header()->hideSection(SubscriptionListModel::TotalCountColumn);
    }
}

void Akregator::SubscriptionListView::slotPrevFeed()
{
    if (!model()) {
        return;
    }
    const QModelIndex current = currentIndex();
    QModelIndex prev = prevFeedIndex(current);
    if (!prev.isValid()) {
        prev = prevFeedIndex(lastLeaveChild(model()), true);
    }
    if (prev.isValid()) {
        setCurrentIndex(prev);
    }
}

void Akregator::SubscriptionListView::slotNextFeed()
{
    if (!model()) {
        return;
    }
    Q_EMIT userActionTakingPlace();
    const QModelIndex current = currentIndex();
    QModelIndex next = nextFeedIndex(current);
    if (!next.isValid()) {
        next = nextFeedIndex(model()->index(0, 0));
    }
    if (next.isValid()) {
        setCurrentIndex(next);
    }
}

void Akregator::SubscriptionListView::slotPrevUnreadFeed()
{
    if (!model()) {
        return;
    }
    Q_EMIT userActionTakingPlace();
    const QModelIndex current = currentIndex();
    QModelIndex prev = prevUnreadFeedIndex(current);
    if (!prev.isValid()) {
        prev = prevUnreadFeedIndex(lastLeaveChild(model()), true);
    }
    if (prev.isValid()) {
        setCurrentIndex(prev);
    }
}

void Akregator::SubscriptionListView::slotNextUnreadFeed()
{
    if (!model()) {
        return;
    }
    Q_EMIT userActionTakingPlace();
    const QModelIndex current = currentIndex();
    QModelIndex next = nextUnreadFeedIndex(current);
    if (!next.isValid()) {
        next = nextUnreadFeedIndex(model()->index(0, 0));
    }
    if (next.isValid()) {
        setCurrentIndex(next);
    }
}

void SubscriptionListView::slotItemBegin()
{
    if (!model()) {
        return;
    }
    Q_EMIT userActionTakingPlace();
    setCurrentIndex(nextFeedIndex(model()->index(0, 0)));
}

void SubscriptionListView::slotItemEnd()
{
    if (!model()) {
        return;
    }
    Q_EMIT userActionTakingPlace();
    setCurrentIndex(lastLeaveChild(model()));
}

void SubscriptionListView::slotItemLeft()
{
    if (!model()) {
        return;
    }
    Q_EMIT userActionTakingPlace();
    const QModelIndex current = currentIndex();
    if (!current.isValid()) {
        setCurrentIndex(nextFeedIndex(model()->index(0, 0)));
        return;
    }
    if (current.parent().isValid()) {
        setCurrentIndex(current.parent());
    }
}

void SubscriptionListView::slotItemRight()
{
    if (!model()) {
        return;
    }
    Q_EMIT userActionTakingPlace();
    const QModelIndex current = currentIndex();
    if (!current.isValid()) {
        setCurrentIndex(nextFeedIndex(model()->index(0, 0)));
        return;
    }
    if (model()->rowCount(current) > 0) {
        setCurrentIndex(model()->index(0, 0, current));
    }
}

void SubscriptionListView::slotItemUp()
{
    if (!model()) {
        return;
    }
    Q_EMIT userActionTakingPlace();
    const QModelIndex current = currentIndex();
    QModelIndex prev = current.row() > 0 ? current.sibling(current.row() - 1, current.column()) : current.parent();
    if (!prev.isValid()) {
        prev = lastLeaveChild(model());
    }
    if (prev.isValid()) {
        setCurrentIndex(prev);
    }
}

void SubscriptionListView::slotItemDown()
{
    if (!model()) {
        return;
    }
    Q_EMIT userActionTakingPlace();
    const QModelIndex current = currentIndex();
    if (current.row() >= model()->rowCount(current.parent())) {
        return;
    }
    setCurrentIndex(current.sibling(current.row() + 1, current.column()));
}

void SubscriptionListView::slotSetHideReadFeeds(bool setting)
{
    QAbstractItemModel *m = model();
    if (!m) {
        return;
    }

    FilterUnreadProxyModel *filter = qobject_cast<FilterUnreadProxyModel *>(m);
    if (!filter) {
        qCCritical(AKREGATOR_LOG) << "Unable to cast model to FilterUnreadProxyModel*";
        return;
    }

    Settings::setHideReadFeeds(setting);
    filter->setDoFilter(setting);
}

void Akregator::SubscriptionListView::slotSetAutoExpandFolders(bool setting)
{
    Settings::setAutoExpandFolders(setting);
    if (!setting) {
        return;
    }

    //expand any current subscriptions with unread items
    QQueue<QModelIndex> indexes;
    //start at the root node
    indexes.enqueue(QModelIndex());

    QAbstractItemModel *m = model();
    if (!m) {
        return;
    }

    while (!indexes.isEmpty()) {
        QModelIndex parent = indexes.dequeue();
        int rows = m->rowCount(parent);

        for (int row = 0; row < rows; ++row) {
            QModelIndex current = m->index(row, 0, parent);

            if (m->hasChildren(current)) {
                indexes.enqueue(current);
            }

            if (!m->data(current, SubscriptionListModel::HasUnreadRole).toBool()) {
                continue;
            }

            setExpanded(current, true);
        }
    }
}

void Akregator::SubscriptionListView::ensureNodeVisible(Akregator::TreeNode *)
{
}

void Akregator::SubscriptionListView::startNodeRenaming(Akregator::TreeNode *node)
{
    Q_UNUSED(node);
    const QModelIndex current = currentIndex();
    if (!current.isValid()) {
        return;
    }
    edit(current);
}
