/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2007 Frank Osterfeld <frank.osterfeld@kdemail.net>
    SPDX-FileCopyrightText: 2009 Jonathan Marten <jjm@keelhaul.me.uk>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "subscriptionlistdelegate.h"
#include "subscriptionlistmodel.h"

#include "akregator_debug.h"

#include <QApplication>
#include <QHeaderView>
#include <QStyle>
#include <QTreeView>

using namespace Akregator;

Akregator::SubscriptionListDelegate::SubscriptionListDelegate(QWidget *parent)
    : QStyledItemDelegate(parent)
{
    // TODO reimplement
    // connect(KGlobalSettings::self(), &KGlobalSettings::appearanceChanged, this, &SubscriptionListDelegate::recalculateRowHeight);
    recalculateRowHeight();
}

Akregator::SubscriptionListDelegate::~SubscriptionListDelegate()
{
}

QSize Akregator::SubscriptionListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(qMax(size.height(), (m_viewIconHeight + 2)));
    // +2 for row top/bottom margin
    return size;
}

void Akregator::SubscriptionListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem newOption = option;
    if (index.data(SubscriptionListModel::HasUnreadRole).toBool()) {
        // feed has unread articles
        newOption.font.setBold(true);
    }

    // fix [Bug 190052] numeric columns aligned to the left
    if (index.column() == SubscriptionListModel::UnreadCountColumn || index.column() == SubscriptionListModel::TotalCountColumn) {
        newOption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
    } else {
        newOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
    }

    // No need to translate the painter here - the item is vertically centered
    // within its sizeHint rectangle.
    QStyledItemDelegate::paint(painter, newOption, index);
}

void Akregator::SubscriptionListDelegate::recalculateRowHeight()
{
    m_viewIconHeight = QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize);
    qCDebug(AKREGATOR_LOG) << "icon height" << m_viewIconHeight;
}

void Akregator::SubscriptionListDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);

    if (index.column() != 0) {
        // Append unread count to the title column only (it is always the first
        // one)
        return;
    }

    auto *view = static_cast<QTreeView *>(parent());
    if (!view->header()->isSectionHidden(SubscriptionListModel::UnreadCountColumn)) {
        // Do not append unread count to the title if the unread count column
        // is visible
        return;
    } else {
        view->header()->resizeSection(SubscriptionListModel::UnreadCountColumn, QHeaderView::ResizeToContents);
    }

    if (!view->header()->isSectionHidden(SubscriptionListModel::TotalCountColumn)) {
        view->header()->resizeSection(SubscriptionListModel::TotalCountColumn, QHeaderView::ResizeToContents);
    }

    auto *optionV4 = qstyleoption_cast<QStyleOptionViewItem *>(option);
    if (!optionV4) {
        // Should never happen, but play it safe
        return;
    }

    QModelIndex unreadIndex = index.sibling(index.row(), SubscriptionListModel::UnreadCountColumn);
    int unread = unreadIndex.data().toInt();
    if (unread > 0) {
        optionV4->text += QStringLiteral(" (%1)").arg(unread);
    }
}
