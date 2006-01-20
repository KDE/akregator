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

#include "categoryatomimpl.h"
#include "feedatomimpl.h"
#include "imageatomimpl.h"
#include "itematomimpl.h"
#include "../atom/category.h"
#include "../atom/entry.h"

#include <ksharedptr.h>

#include <QString>
#include <QList>

namespace LibSyndication {

FeedAtomImpl::FeedAtomImpl(LibSyndication::Atom::FeedDocumentPtr doc) : m_doc(doc)
{
}

LibSyndication::AbstractDocumentPtr FeedAtomImpl::document() const
{
    return LibSyndication::AbstractDocumentPtr::staticCast(m_doc);
}

QList<LibSyndication::ItemPtr> FeedAtomImpl::items() const
{
    QList<ItemPtr> items;
    QList<LibSyndication::Atom::Entry> entries = m_doc->entries();
    QList<LibSyndication::Atom::Entry>::ConstIterator it = entries.begin();
    QList<LibSyndication::Atom::Entry>::ConstIterator end = entries.end();
    
    for ( ; it != end; ++it)
    {
        ItemAtomImplPtr item = new ItemAtomImpl(*it);
        items.append(ItemPtr::staticCast(item));
    }
    
    return items;
}

QList<LibSyndication::CategoryPtr> FeedAtomImpl::categories() const
{
    QList<CategoryPtr> categories;
    QList<LibSyndication::Atom::Category> entries = m_doc->categories();
    QList<LibSyndication::Atom::Category>::ConstIterator it = entries.begin();
    QList<LibSyndication::Atom::Category>::ConstIterator end = entries.end();
    
    for ( ; it != end; ++it)
    {
        CategoryAtomImplPtr item = new CategoryAtomImpl(*it);
        categories.append(CategoryPtr::staticCast(item));
    }
    
    return categories;
}

QString FeedAtomImpl::title() const
{
    return m_doc->title();
}

QString FeedAtomImpl::link() const
{
    return "TODO";
}

QString FeedAtomImpl::description() const
{
    return m_doc->subtitle();
}

QString FeedAtomImpl::author() const
{
    return "TODO";
}

QString FeedAtomImpl::language() const
{
    return m_doc->xmlLang();
}

ImagePtr FeedAtomImpl::image() const
{
    ImageAtomImplPtr ptr = new ImageAtomImpl;
    return ImagePtr::staticCast(ptr);
}

} // namespace LibSyndication
