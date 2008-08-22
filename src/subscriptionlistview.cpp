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

#include "subscriptionlistmodel.h"
#include "subscriptionlistview.h"
#include "akregatorconfig.h"

#include <QHeaderView>
#include <QStack>
#include <QPointer>

#include <KMenu>
#include <KLocale>
#include <KDebug>
#include <KConfigGroup>

#include <cassert>

using namespace Akregator;

namespace {

QModelIndex prevIndex( const QModelIndex& idx )
{
    if ( !idx.isValid() )
        return QModelIndex();
    const QAbstractItemModel* const model = idx.model();
    assert( model );

    if ( idx.row() > 0 )
    {
        QModelIndex i = idx.sibling( idx.row() - 1, idx.column() );
        while ( model->hasChildren( i ) )
            i = i.child( model->rowCount( i ) - 1, i.column() );
        return i;
    }
    else 
        return idx.parent();
}


QModelIndex prevFeedIndex( const QModelIndex& idx, bool allowPassed = false )
{
    QModelIndex prev = allowPassed ? idx : prevIndex( idx );
    while ( prev.isValid() && prev.data( SubscriptionListModel::IsAggregationRole ).toBool() )
        prev = prevIndex( prev );
    return prev;
}

QModelIndex prevUnreadFeedIndex( const QModelIndex& idx, bool allowPassed = false )
{
    QModelIndex prev = allowPassed ? idx : prevIndex( idx );
    while ( prev.isValid() && ( prev.data( SubscriptionListModel::IsAggregationRole ).toBool() || prev.sibling( prev.row(), SubscriptionListModel::UnreadCountColumn ).data().toInt() == 0 ) )
        prev = prevIndex( prev );
    return prev;
}

QModelIndex lastLeaveChild( const QAbstractItemModel* const model )
{
    assert( model );
    if ( model->rowCount() == 0 )
        return QModelIndex();
    QModelIndex idx = model->index( model->rowCount() - 1, 0 );
    while ( model->hasChildren( idx ) )
        idx = idx.child( model->rowCount( idx ) - 1, idx.column() );
    return idx;
}

QModelIndex nextIndex( const QModelIndex& idx )
{
    if ( !idx.isValid() )
        return QModelIndex();
    const QAbstractItemModel* const model = idx.model();
    assert( model );
    if ( model->hasChildren( idx ) )
        return idx.child( 0, idx.column() );
    QModelIndex i = idx;
    while ( true )
    {
        if ( !i.isValid() )
            return i;
        const int siblings = model->rowCount( i.parent() );
        if ( i.row() + 1 < siblings )
            return i.sibling( i.row() + 1, i.column() );
        i = i.parent();
    }   
}

QModelIndex nextFeedIndex( const QModelIndex& idx )
{
    QModelIndex next = nextIndex( idx );
    while ( next.isValid() && next.data( SubscriptionListModel::IsAggregationRole ).toBool() )
        next = nextIndex( next );
    return next;
}

QModelIndex nextUnreadFeedIndex( const QModelIndex& idx )
{
    QModelIndex next = nextIndex( idx );
    while ( next.isValid() && ( next.data( SubscriptionListModel::IsAggregationRole ).toBool() || next.sibling( next.row(), SubscriptionListModel::UnreadCountColumn ).data().toInt() == 0 ) )
        next = nextIndex( next );
    return next;
}

}

Akregator::SubscriptionListView::SubscriptionListView( QWidget* parent ) : QTreeView( parent )
{
    setSelectionMode( QAbstractItemView::SingleSelection );
    setRootIsDecorated( false );
    setAlternatingRowColors( true );
    setUniformRowHeights( true );
    setContextMenuPolicy( Qt::CustomContextMenu );
    setDragDropMode( QAbstractItemView::DragDrop );
    setDropIndicatorShown( true );
    setAcceptDrops( true );
    m_headerSetup = false;
    connect( header(), SIGNAL( customContextMenuRequested( const QPoint & ) ), this, SLOT( showHeaderMenu( const QPoint& ) ) );
}

Akregator::SubscriptionListView::~SubscriptionListView()
{
    saveHeaderSettings();
}

void Akregator::SubscriptionListView::setModel( QAbstractItemModel* model )
{
    QTreeView::setModel( model );
    
    QStack<QModelIndex> stack;
    stack.push( rootIndex() );
    while ( !stack.isEmpty() )
    {
        const QModelIndex i = stack.pop();
        const int childCount = model->rowCount( i );
        for ( int j = 0; j < childCount; ++j )
        {
            const QModelIndex child = model->index( j, 0, i );
            if ( child.isValid() )
                stack.push( child );
        }
        setExpanded( i, i.data( Akregator::SubscriptionListModel::IsOpenRole ).toBool() );
    }

    header()->setContextMenuPolicy( Qt::CustomContextMenu );

    if( ! m_headerSetup ) {
        loadHeaderSettings();
        m_headerSetup = true;
    }
}

void Akregator::SubscriptionListView::showHeaderMenu( const QPoint& pos )
{
    if( ! model() )
        return;

    QPointer<KMenu> menu = new KMenu( this );
    menu->addTitle( i18n( "Columns" ) );
    menu->setAttribute( Qt::WA_DeleteOnClose );
    connect(menu, SIGNAL( triggered( QAction* ) ), this, SLOT( headerMenuItemTriggered( QAction* ) ) );
    
    for (int i = 0; i < model()->columnCount(); i++)
    {
        QString col = model()->headerData( i, Qt::Horizontal, Qt::DisplayRole ).toString();
        QAction* act = menu->addAction( col );
        act->setCheckable( true );
        act->setChecked( !header()->isSectionHidden( i ) );
        act->setData( i );
    }
    
    menu->popup( header()->mapToGlobal( pos ) );
}

void Akregator::SubscriptionListView::headerMenuItemTriggered( QAction* act )
{
    assert( act );
    const int col = act->data().toInt();
    if ( act->isChecked() )
        header()->showSection( col );
    else
        header()->hideSection( col );
}

void Akregator::SubscriptionListView::saveHeaderSettings()
{ 
    QByteArray s = header()->saveState();
    KConfigGroup conf( Settings::self()->config(), "General" );
    conf.writeEntry( "SubscriptionListHeaders", s.toBase64() );

    //FIXME: HACK: Change back to Settings, when it starts to work again..
    // Settings::setFeedlistHeaderStates( s.toBase64() );
}

void Akregator::SubscriptionListView::loadHeaderSettings()
{
    // FIXME: HACK: Change back to Settings when it's working
    //QByteArray s = QByteArray::fromBase64( Settings::feedlistHeaderStates().toAscii() );
    
    KConfigGroup conf( Settings::self()->config(), "General" );
    QByteArray s = QByteArray::fromBase64( conf.readEntry( "SubscriptionListHeaders" ).toAscii() );
    if( !s.isNull() )
        header()->restoreState( s );
}

void Akregator::SubscriptionListView::slotPrevFeed()
{
    if ( !model() )
        return;
    const QModelIndex current = currentIndex();
    QModelIndex prev = prevFeedIndex( current );
    if ( !prev.isValid() )
    {
        prev = prevFeedIndex( lastLeaveChild( model() ), true );
    }
    if ( prev.isValid() )
        setCurrentIndex( prev );
    
}

void Akregator::SubscriptionListView::slotNextFeed()
{
    if ( !model() )
        return;
    const QModelIndex current = currentIndex();
    QModelIndex next = nextFeedIndex( current );
    if ( !next.isValid() )
        next = nextFeedIndex( model()->index( 0, 0 ) );
    if ( next.isValid() )
        setCurrentIndex( next );
}

void Akregator::SubscriptionListView::slotPrevUnreadFeed()
{
    if ( !model() )
        return;
    const QModelIndex current = currentIndex();
    QModelIndex prev = prevUnreadFeedIndex( current );
    if ( !prev.isValid() )
        prev = prevUnreadFeedIndex( lastLeaveChild( model() ), true );
    if ( prev.isValid() )
        setCurrentIndex( prev );
}

void Akregator::SubscriptionListView::slotNextUnreadFeed()
{
    if ( !model() )
        return;
    const QModelIndex current = currentIndex();
    QModelIndex next = nextUnreadFeedIndex( current );
    if ( !next.isValid() )
        next = nextUnreadFeedIndex( model()->index( 0, 0 ) );
    if ( next.isValid() )
        setCurrentIndex( next );
}

void SubscriptionListView::slotItemBegin()
{
    
}

void SubscriptionListView::slotItemEnd()
{
}

void SubscriptionListView::slotItemLeft()
{
    
}

void SubscriptionListView::slotItemRight()
{
    
}

void SubscriptionListView::slotItemUp()
{
    
}

void SubscriptionListView::slotItemDown()
{
    
}


void Akregator::SubscriptionListView::ensureNodeVisible( Akregator::TreeNode* )
{
}

void Akregator::SubscriptionListView::startNodeRenaming( Akregator::TreeNode* node )
{
    Q_UNUSED( node );
    const QModelIndex current = currentIndex();
    if ( !current.isValid() )
        return;
    openPersistentEditor( current );
}

#include "subscriptionlistview.moc"
