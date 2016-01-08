/*
    This file is part of Akregator.

    Copyright (C) 2007 Frank Osterfeld <osterfeld@kde.org>
    Copyright (C) 2009 Jonathan Marten <jjm@keelhaul.me.uk>

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

#ifndef AKREGATOR_SUBSCRIPTIONLISTDELEGATE_H
#define AKREGATOR_SUBSCRIPTIONLISTDELEGATE_H

#include <QStyledItemDelegate>

namespace Akregator
{

class SubscriptionListDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit SubscriptionListDelegate(QWidget *parent = Q_NULLPTR);
    ~SubscriptionListDelegate();

protected:
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const Q_DECL_OVERRIDE;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE;

    void initStyleOption(QStyleOptionViewItem *option,
                         const QModelIndex &index) const Q_DECL_OVERRIDE;

private Q_SLOTS:
    void recalculateRowHeight();

private:
    int m_viewIconHeight;
};

}

#endif // AKREGATOR_SUBSCRIPTIONLISTDELEGATE_H
