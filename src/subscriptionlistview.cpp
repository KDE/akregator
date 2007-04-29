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

#include "feedlist.h"
#include "subscriptionlistmodel.h"
#include "subscriptionlistview.h"

Akregator::SubscriptionListView::SubscriptionListView( QWidget* parent ) : QTreeView( parent ), m_feedList( 0 )
{
    setSelectionMode( QAbstractItemView::SingleSelection );
    setRootIsDecorated( false );
    setAlternatingRowColors( true );
    setUniformRowHeights( true );

    connect( selectionModel(), SIGNAL( currentChanged( QModelIndex, QModelIndex ) ),
             this, SLOT( currentChanged( QModelIndex, QModelIndex ) ) );
}

namespace {

    static QAbstractItemModel* createModelForFeedList( Akregator::FeedList* list, QObject* parent )
    {
        return list ? new Akregator::SubscriptionListModel( list, parent ) : 0;
    }

    static Akregator::TreeNode* nodeForIndex( const QModelIndex& index, Akregator::FeedList* list )
    {
        if ( !index.isValid() || !list )
            return 0L;

         return list->findByID( index.internalId() );
    }
}

Akregator::Folder* Akregator::SubscriptionListView::rootNode()
{
    return m_feedList->rootNode();
}

void Akregator::SubscriptionListView::setFeedList( Akregator::FeedList* feedList )
{
    delete model();
    m_feedList = feedList;
    setModel( ::createModelForFeedList( feedList, this ) );
}

Akregator::TreeNode* Akregator::SubscriptionListView::selectedNode() {
    const QModelIndexList indexes = selectedIndexes();
    return indexes.isEmpty() ? 0L : ::nodeForIndex( indexes.first(), m_feedList );
}

void Akregator::SubscriptionListView::slotPrevFeed()
{
}

void Akregator::SubscriptionListView::slotNextFeed()
{
}

void Akregator::SubscriptionListView::slotPrevUnreadFeed()
{
}

void Akregator::SubscriptionListView::slotNextUnreadFeed()
{
}

void Akregator::SubscriptionListView::ensureNodeVisible( Akregator::TreeNode* )
{
}

Akregator::TreeNode* Akregator::SubscriptionListView::findNodeByTitle( const QString& )
{
    return 0L;
}

void Akregator::SubscriptionListView::currentChanged( const QModelIndex& current, const QModelIndex& )
{
    emit signalNodeSelected( ::nodeForIndex( current, m_feedList ) );
}

void Akregator::SubscriptionListView::startNodeRenaming( Akregator::TreeNode* node )
{
}

#include "subscriptionlistview.moc"
