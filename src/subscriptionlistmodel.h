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
#ifndef AKREGATOR_SUBSCRIPTIONLISTMODEL_H
#define AKREGATOR_SUBSCRIPTIONLISTMODEL_H

#include <kdepim_export.h>

#include <QAbstractItemModel>

namespace Akregator {

class FeedList;
class TreeNode;

class AKREGATOR_EXPORT SubscriptionListModel : public QAbstractItemModel
{
Q_OBJECT
public:

    enum Role {
        TitleRole=Qt::UserRole,
        UnreadCountRole,
        TotalCountRole,
        SubscriptionIdRole
    };

    enum Column {
        TitleColumn=0,
        UnreadCountColumn=1,
        TotalCountColumn=2
    };

    explicit SubscriptionListModel( Akregator::FeedList* feedList, QObject* parent = 0 );

    //impl
    int columnCount( const QModelIndex& parent = QModelIndex() ) const;

    //impl
    int rowCount( const QModelIndex& parent = QModelIndex() ) const;

    //impl
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;

    //impl
    QModelIndex index( int row, int column, const QModelIndex& parent = QModelIndex() ) const; 

    //impl
    QModelIndex parent( const QModelIndex& index ) const;

    //reimpl
    QVariant headerData( int section, Qt::Orientation orientation, int role=Qt::DisplayRole ) const;

private Q_SLOTS:

    void feedListDestroyed( Akregator::FeedList* feedList );

    void subscriptionAdded( Akregator::TreeNode* );

    void subscriptionRemoved( Akregator::TreeNode* );

private:

    Akregator::FeedList* m_feedList;
};

} // namespace Akregator

#endif // AKREGATOR_SUBSCRIPTIONLISTMODEL_H
