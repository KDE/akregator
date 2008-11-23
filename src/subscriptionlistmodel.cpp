/*
    This file is part of Akregator.

    Copyright (C) 2007 Frank Osterfeld <osterfeld@kde.org>

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
#include "feed.h"
#include "feedlist.h"
#include "folder.h"
#include "subscriptionlistjobs.h"
#include "treenode.h"

#include <KDebug>
#include <KIconLoader>
#include <KLocalizedString>

#include <QByteArray>
#include <QDataStream>
#include <QFont>
#include <QIcon>
#include <QList>
#include <QMimeData>
#include <QUrl>
#include <QVariant>

#include <cassert>

using namespace Akregator;
using namespace Syndication;

#define AKREGATOR_TREENODE_MIMETYPE "akregator/treenode-id"


namespace {

    QString errorCodeToString( Syndication::ErrorCode err )
    {
        switch ( err )
        {
            case Timeout:
                return i18n( "Timeout on remote server" );
            case UnknownHost:
                return i18n( "Unknown host" );
            case FileNotFound:
                return i18n( "Feed file not found on remote server" );
            case InvalidXml:
                return i18n( "Could not read feed (invalid XML)" );
            case XmlNotAccepted:
                return i18n( "Could not read feed (unknown format)" );
            case InvalidFormat:
                return i18n( "Could not read feed (invalid feed)" );
            case Success:
            case Aborted:
            default:
                return QString();
        }
    }

    static const Akregator::TreeNode* nodeForIndex( const QModelIndex& index, const Akregator::FeedList* feedList )
    {
        return ( !index.isValid() || !feedList ) ? 0 : feedList->findByID( index.internalId() );
    }
}

Akregator::SubscriptionListModel::SubscriptionListModel( const Akregator::FeedList* feedList, QObject* parent ) : QAbstractItemModel( parent ), m_feedList( feedList ), m_beganRemoval( false )
{
    if ( feedList )
    {
        connect( feedList, SIGNAL( signalDestroyed( Akregator::FeedList* ) ),
                 this, SLOT( feedListDestroyed( Akregator::FeedList* ) ) );
        connect( feedList, SIGNAL( signalNodeAdded( Akregator::TreeNode* ) ),
                 this, SLOT( subscriptionAdded( Akregator::TreeNode* ) ) );
        connect( feedList, SIGNAL( signalAboutToRemoveNode( Akregator::TreeNode* ) ),
                 this, SLOT( aboutToRemoveSubscription( Akregator::TreeNode* ) ) );
        connect( feedList, SIGNAL( signalNodeRemoved( Akregator::TreeNode* ) ),
                   this, SLOT( subscriptionRemoved( Akregator::TreeNode* ) ) );
        connect( feedList, SIGNAL( signalNodeChanged( Akregator::TreeNode* ) ),
                 this, SLOT( subscriptionChanged( Akregator::TreeNode* ) ) );
        connect( feedList, SIGNAL( fetchStarted( Akregator::Feed* ) ),
                 this, SLOT( fetchStarted( Akregator::Feed* ) ) );
        connect( feedList, SIGNAL( fetched( Akregator::Feed* ) ),
                 this, SLOT( fetched( Akregator::Feed* ) ) );
        connect( feedList, SIGNAL( fetchAborted( Akregator::Feed* ) ),
                 this, SLOT( fetchAborted( Akregator::Feed* ) ) );
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

    const Akregator::TreeNode* const node = nodeForIndex( parent, m_feedList );
    return node ? node->children().count() : 0;
}

uint Akregator::SubscriptionListModel::nodeIdForIndex( const QModelIndex& idx ) const
{
    return idx.isValid() ? idx.internalId() : 0;
}

QVariant Akregator::SubscriptionListModel::data( const QModelIndex& index, int role ) const
{
    if ( !index.isValid() )
        return QVariant();

    const Akregator::TreeNode* const node = nodeForIndex( index, m_feedList );

    if ( !node )
        return QVariant();

    switch ( role )
    {
        case Qt::EditRole:
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
        case Qt::FontRole:
        {
            if(node->unread() > 0) {
                QFont font;
                font.setBold(true);
                return font;
            }
        }
        case Qt::ToolTipRole:
        {
            if ( node->isGroup() || node->isAggregation() )
                return node->title();
            const Feed* const feed = qobject_cast<const Feed* const>( node );
            if ( !feed )
                return QString();
            if ( feed->fetchErrorOccurred() )
                return i18n( "Could not fetch feed: %1", errorCodeToString( feed->fetchErrorCode() ) );
            return feed->title();
        }
        case Qt::DecorationRole:
        {
            if ( index.column() != TitleColumn )
                return QVariant();
            const Feed* const feed = qobject_cast<const Feed* const>( node );
            return feed && feed->isFetching() ? node->icon().pixmap( KIconLoader::SizeSmall, QIcon::Active ) : node->icon();
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
        case LinkRole:
        {
        	const Feed* const feed = qobject_cast<const Feed* const>( node );
        	return feed ? feed->xmlUrl() : QVariant();
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
            return i18nc("Feedlist's column header", "Feeds");
        case UnreadCountColumn:
            return i18nc("Feedlist's column header", "Unread");
        case TotalCountColumn:
            return i18nc("Feedlist's column header", "Total");
    }

    return QVariant();
}

QModelIndex Akregator::SubscriptionListModel::parent( const QModelIndex& index ) const
{
    const Akregator::TreeNode* const node = nodeForIndex( index, m_feedList );

    if ( !node || !node->parent() )
        return QModelIndex();

    const Akregator::Folder* parent = node->parent();

    if ( !parent->parent() )
        return createIndex( 0, 0, parent->id() );

    const Akregator::Folder* const grandparent = parent->parent();

    const int row = grandparent->indexOf( parent );

    Q_ASSERT( row != -1 );

    return createIndex( row, 0, parent->id() );
}

QModelIndex Akregator::SubscriptionListModel::index( int row, int column, const QModelIndex& parent ) const
{
    if ( !parent.isValid() )
        return ( row == 0 && m_feedList ) ? createIndex( row, column , m_feedList->rootNode()->id() ) : QModelIndex();

    const Akregator::TreeNode* const parentNode = nodeForIndex( parent, m_feedList );
    const Akregator::TreeNode* const childNode = parentNode->childAt( row );
    return  childNode ? createIndex( row, column, childNode->id() ) : QModelIndex();
}


QModelIndex SubscriptionListModel::indexForNode( const TreeNode* node ) const
{
    if ( !node || !m_feedList )
        return QModelIndex();
    const Folder* const parent = node->parent();
    if ( !parent )
        return index( 0, 0 );
    const int row = parent->indexOf( node );
    Q_ASSERT( row >= 0 );
    const QModelIndex idx = index( row, 0, indexForNode( parent ) );
    Q_ASSERT( idx.internalId() == node->id() );
    return idx;
}

void Akregator::SubscriptionListModel::feedListDestroyed( Akregator::FeedList* )
{
    m_feedList = 0;
}

void Akregator::SubscriptionListModel::subscriptionAdded( Akregator::TreeNode* subscription )
{
    const Folder* const parent = subscription->parent();
    const int row = parent ? parent->indexOf( subscription ) : 0;
    Q_ASSERT( row >= 0 );
    beginInsertRows( indexForNode( parent ), row, row );
    endInsertRows();
}

void Akregator::SubscriptionListModel::aboutToRemoveSubscription( Akregator::TreeNode* subscription )
{
    kDebug() << subscription->id() << endl;
    const Folder* const parent = subscription->parent();
    const int row = parent ? parent->indexOf( subscription ) : -1;
    if ( row < 0 )
        return;
    beginRemoveRows( indexForNode( parent ), row, row );
    m_beganRemoval = true;
}

void Akregator::SubscriptionListModel::subscriptionRemoved( TreeNode* subscription )
{
    kDebug() << subscription->id() << endl;
    if ( m_beganRemoval )
    {
        m_beganRemoval = false;
        endRemoveRows();
    }
}

void Akregator::SubscriptionListModel::subscriptionChanged( TreeNode* node )
{
    const QModelIndex idx = indexForNode( node );
    if ( !idx.isValid() )
        return;
    emit dataChanged( index( idx.row(), 0, idx.parent() ),
                      index( idx.row(), ColumnCount - 1, idx.parent() ) );
}

void SubscriptionListModel::fetchStarted( Akregator::Feed* node )
{
    subscriptionChanged( node );
}

void SubscriptionListModel::fetched( Akregator::Feed* node )
{
    subscriptionChanged( node );
}

void SubscriptionListModel::fetchError( Akregator::Feed* node )
{
    subscriptionChanged( node );
}

void SubscriptionListModel::fetchAborted( Akregator::Feed* node )
{
    subscriptionChanged( node );
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

Qt::ItemFlags SubscriptionListModel::flags( const QModelIndex& idx ) const
{
    const Qt::ItemFlags flags = QAbstractItemModel::flags( idx );
    if ( !idx.isValid() || ( idx.column() != TitleColumn ) )
        return flags;
    if ( !idx.parent().isValid() ) // the root folder is neither draggable nor editable
        return flags | Qt::ItemIsDropEnabled;
    return flags | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable;
}

QStringList SubscriptionListModel::mimeTypes() const
{
    QStringList types;
    types << "text/uri-list" << AKREGATOR_TREENODE_MIMETYPE;
    return types;
}

QMimeData* SubscriptionListModel::mimeData( const QModelIndexList& indexes ) const
{
    QMimeData* mimeData = new QMimeData;

    QList<QUrl> urls;
    Q_FOREACH ( const QModelIndex& i, indexes )
    {
        const QUrl url = i.data( LinkRole ).toString();
        if ( !url.isEmpty() )
            urls << url;
    }

    mimeData->setUrls( urls );

    QByteArray idList;
    QDataStream idStream( &idList, QIODevice::WriteOnly );
    Q_FOREACH ( const QModelIndex& i, indexes )
        if ( i.isValid() )
            idStream << i.data( SubscriptionIdRole ).toInt();

    mimeData->setData( AKREGATOR_TREENODE_MIMETYPE, idList );

    return mimeData;
}


bool SubscriptionListModel::setData( const QModelIndex& idx, const QVariant& value, int role )
{
    if ( !idx.isValid() || idx.column() != TitleColumn || role != Qt::EditRole )
        return false;
    const TreeNode* const node = nodeForIndex( idx, m_feedList );
    if ( !node )
        return false;
    RenameSubscriptionJob* job = new RenameSubscriptionJob( this );
    job->setSubscriptionId( node->id() );
    job->setName( value.toString() );
    job->start();
    return true;
}

bool SubscriptionListModel::dropMimeData( const QMimeData* data,
                                          Qt::DropAction action,
                                          int row,
                                          int column,
                                          const QModelIndex& parent )
{
    if ( action == Qt::IgnoreAction )
        return true;

    //if ( column != TitleColumn )
    //    return false;

    if ( data->hasFormat( AKREGATOR_TREENODE_MIMETYPE ) )
    {
        const TreeNode* const droppedOnNode = qobject_cast<const TreeNode*>( nodeForIndex( parent, m_feedList ) );

        const Folder* const destFolder = droppedOnNode->isGroup() ? qobject_cast<const Folder*>( droppedOnNode ) : droppedOnNode->parent();
        if ( !destFolder )
            return false;

        QByteArray idData = data->data( AKREGATOR_TREENODE_MIMETYPE );
        QList<int> ids;
        QDataStream stream( &idData, QIODevice::ReadOnly );
        while ( !stream.atEnd() )
        {
            int id;
            stream >> id;
            ids << id;
        }

        //don't drop nodes into their own subtree
        Q_FOREACH ( const int id, ids )
        {
            const Folder* const asFolder = qobject_cast<const Folder*>( m_feedList->findByID( id ) );
            if ( asFolder && ( asFolder == destFolder || asFolder->subtreeContains( destFolder ) ) )
                return false;
        }

        const TreeNode* const after = droppedOnNode->isGroup() ? destFolder->childAt( row ) : droppedOnNode;

        Q_FOREACH ( const int id, ids )
        {
            const TreeNode* const node = m_feedList->findByID( id );
            if ( !node )
                continue;
            MoveSubscriptionJob* job = new MoveSubscriptionJob( this );
            job->setSubscriptionId( node->id() );
            job->setDestination( destFolder->id(), after ? after->id() : -1 );
            job->start();
        }
        return true;
    }

    return false;
}

#include "subscriptionlistmodel.moc"
