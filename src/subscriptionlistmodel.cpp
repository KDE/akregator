/*
    This file is part of Akregator.

    Copyright (C) 2007 Frank Osterfeld <frank.osterfeld@kdemail.net>

    This program is free software you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "subscriptionlistmodel.h"
#include "feedlist.h"
#include "folder.h"
#include "treenode.h"

#include <KIcon>
#include <KLocale>

#include <QStack>
#include <QTreeView>

namespace {
    static const Akregator::TreeNode* nodeForIndex( const QModelIndex& index, const Akregator::FeedList* feedList )
    {
        return ( !index.isValid() || !feedList ) ? 0 : feedList->findByID( index.internalId() );
    }
}

Akregator::SubscriptionListModel::SubscriptionListModel( const Akregator::FeedList* feedList, QObject* parent ) : QAbstractItemModel( parent ), m_feedList( feedList )
{
    if ( feedList )
    {
        connect( feedList, SIGNAL( signalDestroyed( Akregator::FeedList* ) ),
                 this, SLOT( feedListDestroyed( Akregator::FeedList* ) ) );
        connect( feedList, SIGNAL( signalTreeNodeAdded( Akregator::TreeNode* ) ),
                 this, SLOT( subscriptionAdded( Akregator::TreeNode* ) ) );
        connect( feedList, SIGNAL( signalTreeNodeRemoved( Akregator::TreeNode* ) ),
                 this, SLOT( subscriptionRemoved( Akregator::TreeNode* ) ) );
    }
}

int Akregator::SubscriptionListModel::columnCount( const QModelIndex& ) const
{
    return 3;
}

int Akregator::SubscriptionListModel::rowCount( const QModelIndex& parent ) const
{
    if ( !parent.isValid() )
        return 1;

    const Akregator::TreeNode* const node = ::nodeForIndex( parent, m_feedList );
    return node ? node->children().count() : 0;
}

int Akregator::SubscriptionListModel::nodeIdForIndex( const QModelIndex& idx ) const
{
    return idx.isValid() ? idx.internalId() : -1;
}

QVariant Akregator::SubscriptionListModel::data( const QModelIndex& index, int role ) const
{
    if ( !index.isValid() )
        return QVariant();
    
    const Akregator::TreeNode* const node = ::nodeForIndex( index, m_feedList );

    if ( !node )
        return QVariant();

    switch ( role )
    {
        case Qt::DisplayRole:
        {
            switch ( index.column() )
            {
                case TitleColumn:
                    return node->title();
                case UnreadCountColumn:
                    return node->unread();
                case TotalCountColumn:
                    return node->totalCount();
            }
        }
        case Qt::DecorationRole:
        {
            return index.column() == TitleColumn ? node->icon() : QVariant();
        }
        case SubscriptionIdRole:
        {
            return node->id();
        }
        case IsGroupRole:
        {
            return node->isGroup();
        }
        case IsFetchableRole:
        {
            return !node->isGroup() && !node->isAggregation();
        }
        case IsAggregationRole:
        {
            return node->isAggregation();
        }
        case IsOpenRole:
        {
            if ( !node->isGroup() )
                return false;
            const Akregator::Folder* const folder = qobject_cast<const Akregator::Folder* const>( node );
            Q_ASSERT( folder );
            return folder->isOpen();
        }
    }

    return QVariant();
}

QVariant Akregator::SubscriptionListModel::headerData( int section, Qt::Orientation, int role ) const
{
    if ( role != Qt::DisplayRole )
        return QVariant();

    switch (section)
    {
        case TitleColumn:
            return i18n("Feeds");
        case UnreadCountColumn:
            return i18n("Unread");
        case TotalCountColumn:
            return i18n("Total");
    }

    return QVariant();
}

QModelIndex Akregator::SubscriptionListModel::parent( const QModelIndex& index ) const
{
    const Akregator::TreeNode* const node = ::nodeForIndex( index, m_feedList );

    if ( !node || !node->parent() )
        return QModelIndex();

    Akregator::Folder* parent = node->parent();

    if ( !parent->parent() )
        return createIndex( 0, 0, parent->id() );

    const Akregator::Folder* const grandparent = parent->parent();

    const int row = grandparent->children().indexOf( parent );

    Q_ASSERT( row != -1 );

    return createIndex( row, 0, parent->id() );
}

QModelIndex Akregator::SubscriptionListModel::index( int row, int column, const QModelIndex& parent ) const
{
    if ( !parent.isValid() )
        return createIndex( row, column, row == 0 ? m_feedList->rootNode()->id() : -1 );

    const Akregator::TreeNode* const parentNode = ::nodeForIndex( parent, m_feedList );
    const int id = ( !parentNode || row > parentNode->children().count() ) ? -1 : parentNode->children().at( row )->id();
    return createIndex( row, column, id );
}

void Akregator::SubscriptionListModel::feedListDestroyed( Akregator::FeedList* )
{
    m_feedList = 0;
}

void Akregator::SubscriptionListModel::subscriptionAdded( Akregator::TreeNode* subscription )
{
    reset();
}

void Akregator::SubscriptionListModel::subscriptionRemoved( Akregator::TreeNode* subscription )
{
    reset();
}

void Akregator::FolderExpansionHandler::itemExpanded( const QModelIndex& idx )
{
    setExpanded( idx, true );
}


void Akregator::FolderExpansionHandler::itemCollapsed( const QModelIndex& idx )
{
    setExpanded( idx, false );
}

void Akregator::FolderExpansionHandler::setExpanded( const QModelIndex& idx, bool expanded )
{
    if ( !m_feedList || !m_model )
        return;
    Akregator::TreeNode* const node = m_feedList->findByID( m_model->nodeIdForIndex( idx ) );
    if ( !node || !node->isGroup() )
        return;

    Akregator::Folder* const folder = qobject_cast<Akregator::Folder*>( node );
    Q_ASSERT( folder );
    folder->setOpen( expanded );
}

Akregator::FolderExpansionHandler::FolderExpansionHandler( QObject* parent ) : QObject( parent ), m_feedList( 0 ), m_model( 0 )
{
}

void Akregator::FolderExpansionHandler::setModel( Akregator::SubscriptionListModel* model )
{
    m_model = model;
}

void Akregator::FolderExpansionHandler::setFeedList( Akregator::FeedList* feedList )
{
    m_feedList = feedList;
}

#include "subscriptionlistmodel.moc"
