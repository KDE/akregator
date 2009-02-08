/*
    This file is part of Akregator.

    Copyright (C) 2008 Frank Osterfeld <osterfeld@kde.org>
    
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

#include "filtercolumnsproxymodel.h"

#include <cassert>

using namespace Akregator;

FilterColumnsProxyModel::FilterColumnsProxyModel( QObject* parent )
    : QSortFilterProxyModel( parent ), m_mode( Whitelist ), m_vecSize( 0 )
{
    
}

bool FilterColumnsProxyModel::filterAcceptsColumn( int source_column, const QModelIndex& ) const
{
    return source_column >= m_vecSize ? m_mode == Blacklist : m_columnStates[source_column];
}

void FilterColumnsProxyModel::setColumnEnabled( int col, bool enabled )
{
    if ( col >= m_columnStates.count() )
    {
        m_columnStates.resize( col + 1 );
        m_vecSize = col + 1;
    }
    m_columnStates[col] = enabled;
}


bool FilterColumnsProxyModel::isColumnEnabled( int col ) const
{
    assert( col >= 0 );
    return col < m_columnStates.count() ? m_columnStates[col] : m_mode == Blacklist;
}


FilterColumnsProxyModel::Mode FilterColumnsProxyModel::mode() const
{
    return m_mode;
}

void FilterColumnsProxyModel::setMode( Mode mode )
{
    m_mode = mode;
}

