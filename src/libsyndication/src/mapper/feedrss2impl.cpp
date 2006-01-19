/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2006 Frank Osterfeld <frank.osterfeld@kdemail.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "categoryrss2impl.h"
#include "feedrss2impl.h"
#include "itemrss2impl.h"
#include "../rss2/category.h"
#include "../rss2/item.h"

#include <ksharedptr.h>

#include <QList>
#include <QString>

namespace LibSyndication {
namespace Mapper {

FeedRSS2Impl::FeedRSS2Impl(LibSyndication::RSS2::DocumentPtr doc) : m_doc(doc)
{
}

LibSyndication::AbstractDocumentPtr FeedRSS2Impl::document() const
{
    return LibSyndication::AbstractDocumentPtr::staticCast(m_doc);
}
        
QList<LibSyndication::ItemPtr> FeedRSS2Impl::items() const
{
    QList<ItemPtr> items;
    QList<LibSyndication::RSS2::Item> entries = m_doc->items();
    QList<LibSyndication::RSS2::Item>::ConstIterator it = entries.begin();
    QList<LibSyndication::RSS2::Item>::ConstIterator end = entries.end();
    
    for ( ; it != end; ++it)
    {
        ItemRSS2ImplPtr item = new ItemRSS2Impl(*it);
        items.append(ItemPtr::staticCast(item));
    }
    
    return items;
}

QList<LibSyndication::CategoryPtr> FeedRSS2Impl::categories() const
{
    QList<CategoryPtr> categories;
    QList<LibSyndication::RSS2::Category> entries = m_doc->categories();
    QList<LibSyndication::RSS2::Category>::ConstIterator it = entries.begin();
    QList<LibSyndication::RSS2::Category>::ConstIterator end = entries.end();
    
    for ( ; it != end; ++it)
    {
        CategoryRSS2ImplPtr item = new CategoryRSS2Impl(*it);
        categories.append(CategoryPtr::staticCast(item));
    }
    
    return categories;
}

QString FeedRSS2Impl::title() const
{
    return m_doc->title();
}

QString FeedRSS2Impl::link() const
{
    return m_doc->link();
}

QString FeedRSS2Impl::description() const
{
    return m_doc->description();
}

QString FeedRSS2Impl::author() const
{
    return QString::null;
}

QString FeedRSS2Impl::language() const
{
    return "TODO";
}

} // namespace Mapper
} // namespace LibSyndication
