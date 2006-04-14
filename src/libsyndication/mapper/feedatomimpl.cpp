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
#include <personimpl.h>
#include <atom/category.h>
#include <atom/entry.h>
#include <atom/link.h>
#include <atom/person.h>

#include <QString>
#include <QList>

namespace LibSyndication {

FeedAtomImpl::FeedAtomImpl(LibSyndication::Atom::FeedDocumentPtr doc) : m_doc(doc)
{
}

LibSyndication::SpecificDocumentPtr FeedAtomImpl::specificDocument() const
{
    return m_doc;
}

QList<LibSyndication::ItemPtr> FeedAtomImpl::items() const
{
    QList<ItemPtr> items;
    QList<LibSyndication::Atom::Entry> entries = m_doc->entries();
    QList<LibSyndication::Atom::Entry>::ConstIterator it = entries.begin();
    QList<LibSyndication::Atom::Entry>::ConstIterator end = entries.end();
    
    for ( ; it != end; ++it)
    {
        ItemAtomImplPtr item(new ItemAtomImpl(*it));
        items.append(item);
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
        CategoryAtomImplPtr item(new CategoryAtomImpl(*it));
        categories.append(item);
    }
    
    return categories;
}

QString FeedAtomImpl::title() const
{
    return m_doc->title();
}

QString FeedAtomImpl::link() const
{
    QList<LibSyndication::Atom::Link> links = m_doc->links();
    QList<LibSyndication::Atom::Link>::ConstIterator it = links.begin();
    QList<LibSyndication::Atom::Link>::ConstIterator end = links.end();

    // return first link where rel="alternate"
    // TODO: if there are multiple "alternate" links, find other criteria to choose one of them
    for ( ; it != end; ++it)
    {
        if ((*it).rel() == QString::fromUtf8("alternate"))
        {
            return (*it).href();
        }
    }
    
    return QString::null;
}

QString FeedAtomImpl::description() const
{
    return m_doc->subtitle();
}

QList<PersonPtr> FeedAtomImpl::authors() const
{
    QList<LibSyndication::Atom::Person> atomps = m_doc->authors();
    QList<LibSyndication::Atom::Person>::ConstIterator it = atomps.begin();
    QList<LibSyndication::Atom::Person>::ConstIterator end = atomps.end();
    
    QList<PersonPtr> list;
    
    for ( ; it != end; ++it)
    {
        PersonImplPtr ptr(new PersonImpl((*it).name(), (*it).uri(), (*it).email()));
        list.append(ptr);
    }
    
    atomps = m_doc->contributors();
    
    it = atomps.begin();
    end = atomps.end();
    
    for ( ; it != end; ++it)
    {
        PersonImplPtr ptr(new PersonImpl((*it).name(), (*it).uri(), (*it).email()));
        list.append(ptr);
    }
    
    return list;
}

QString FeedAtomImpl::language() const
{
    return m_doc->xmlLang();
}

QString FeedAtomImpl::copyright() const
{
    return m_doc->rights();
}


ImagePtr FeedAtomImpl::image() const
{
    return ImageAtomImplPtr(new ImageAtomImpl(m_doc->logo()));
}

} // namespace LibSyndication
