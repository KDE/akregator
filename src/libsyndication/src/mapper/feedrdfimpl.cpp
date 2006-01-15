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

#include "feedrdfimpl.h"
#include "itemrdfimpl.h"
#include "../rdf/dublincore.h"
#include "../rdf/item.h"

#include <ksharedptr.h>

#include <QString>
#include <QList>

namespace LibSyndication {
namespace Mapper {

FeedRDFImpl::FeedRDFImpl(LibSyndication::RDF::DocumentPtr doc) : m_doc(doc)
{
}

LibSyndication::AbstractDocumentPtr FeedRDFImpl::document() const
{
    return LibSyndication::AbstractDocumentPtr::staticCast(m_doc);
}

QList<LibSyndication::ItemPtr> FeedRDFImpl::items() const
{
    QList<ItemPtr> items;
    QList<LibSyndication::RDF::Item> entries = m_doc->items();
    QList<LibSyndication::RDF::Item>::ConstIterator it = entries.begin();
    QList<LibSyndication::RDF::Item>::ConstIterator end = entries.end();
    
    for ( ; it != end; ++it)
    {
        ItemRDFImplPtr item = new ItemRDFImpl(*it);
        items.append(ItemPtr::staticCast(item));
    }
    
    return items;
}

QString FeedRDFImpl::title() const
{
    return m_doc->title();
}

QString FeedRDFImpl::link() const
{
    return m_doc->link();
}

QString FeedRDFImpl::description() const
{
    return m_doc->description();
}

QString FeedRDFImpl::author() const
{
    return m_doc->dc().creator();
}

QString FeedRDFImpl::language() const
{
    return "TODO";
}

} // namespace Mapper
} // namespace LibSyndication
