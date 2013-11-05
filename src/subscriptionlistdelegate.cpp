/*
    This file is part of Akregator.

    Copyright (C) 2007 Frank Osterfeld <frank.osterfeld@kdemail.net>
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

#include "subscriptionlistdelegate.h"
#include "subscriptionlistmodel.h"

#include <KDebug>
#include <KGlobalSettings>
#include <KIconTheme>

#include <QHeaderView>
#include <QTreeView>

using namespace Akregator;


Akregator::SubscriptionListDelegate::SubscriptionListDelegate( QWidget *parent )
    : QStyledItemDelegate( parent )
{
    connect( KGlobalSettings::self(), SIGNAL(appearanceChanged()),
             SLOT(recalculateRowHeight()) );
    recalculateRowHeight();
}


Akregator::SubscriptionListDelegate::~SubscriptionListDelegate()
{
}


QSize Akregator::SubscriptionListDelegate::sizeHint( const QStyleOptionViewItem &option,
                                                     const QModelIndex &index ) const
{
    QSize size = QStyledItemDelegate::sizeHint( option, index );
    size.setHeight( qMax( size.height(), ( m_viewIconHeight + 2 ) ) );
                                                        // +2 for row top/bottom margin
    return ( size );
}


void Akregator::SubscriptionListDelegate::paint( QPainter *painter,
                                                 const QStyleOptionViewItem &option,
                                                 const QModelIndex &index ) const
{
    QStyleOptionViewItem newOption = option;
    if ( index.data( SubscriptionListModel::HasUnreadRole ).toBool() )
    {                                                  // feed has unread articles
        newOption.font.setBold(true);
    }

     //fix [Bug 190052] numeric columns aligned to the left
     if ( index.column() == SubscriptionListModel::UnreadCountColumn ||
         index.column() == SubscriptionListModel::TotalCountColumn )
     {
         newOption.displayAlignment = Qt::AlignRight;
     }

    // No need to translate the painter here - the item is vertically centered
    // within its sizeHint rectangle.
    QStyledItemDelegate::paint( painter, newOption, index );
}


void Akregator::SubscriptionListDelegate::recalculateRowHeight()
{
    KIconTheme *iconTheme = KIconLoader::global()->theme();
    m_viewIconHeight = ( iconTheme != NULL ) ? iconTheme->defaultSize( KIconLoader::Small ) : 0;
    kDebug() << "icon height" << m_viewIconHeight;
}


void Akregator::SubscriptionListDelegate::initStyleOption( QStyleOptionViewItem *option,
                                                           const QModelIndex &index ) const
{
    QStyledItemDelegate::initStyleOption( option, index );

    if ( index.column() != 0 )
    {
        // Append unread count to the title column only (it is always the first
        // one)
        return;
    }

    QTreeView *view = static_cast< QTreeView * >( parent() );
    if ( !view->header()->isSectionHidden( SubscriptionListModel::UnreadCountColumn ) )
    {
        // Do not append unread count to the title if the unread count column
        // is visible
        return;
    } else {
        view->header()->resizeSection( SubscriptionListModel::UnreadCountColumn, QHeaderView::ResizeToContents );
    }

    if ( !view->header()->isSectionHidden( SubscriptionListModel::TotalCountColumn ) ) {
        view->header()->resizeSection( SubscriptionListModel::TotalCountColumn, QHeaderView::ResizeToContents );
    }

    QStyleOptionViewItemV4 *optionV4 = qstyleoption_cast< QStyleOptionViewItemV4 * >( option );
    if ( !optionV4 )
    {
        // Should never happen, but play it safe
        return;
    }

    QModelIndex unreadIndex = index.sibling( index.row(), SubscriptionListModel::UnreadCountColumn );
    int unread = unreadIndex.data().toInt();
    if ( unread > 0 )
    {
        optionV4->text += QString( " (%1)" ).arg( unread );
    }
}


