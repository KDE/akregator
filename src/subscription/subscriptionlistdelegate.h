/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2007 Frank Osterfeld <osterfeld@kde.org>
    SPDX-FileCopyrightText: 2009 Jonathan Marten <jjm@keelhaul.me.uk>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include <QStyledItemDelegate>

namespace Akregator
{
class SubscriptionListDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit SubscriptionListDelegate(QWidget *parent = nullptr);
    ~SubscriptionListDelegate() override;

protected:
    [[nodiscard]] QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;

private:
    void recalculateRowHeight();
    int m_viewIconHeight = 0;
};
}
