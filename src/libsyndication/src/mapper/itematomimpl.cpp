/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2006 Frank Osterfeld <frank.osterfeld@kdemail.net>
 *
 * This library is free software you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "enclosureatomimpl.h"
#include "itematomimpl.h"

#include "../atom/content.h"
#include "../atom/link.h"
#include "../atom/person.h"
#include "../enclosure.h"

#include <QList>
#include <QString>


using LibSyndication::Atom::Content;
using LibSyndication::Atom::Link;
using LibSyndication::Atom::Person;

namespace LibSyndication {
namespace Mapper {

ItemAtomImpl::ItemAtomImpl(const LibSyndication::Atom::Entry& entry) : m_entry(entry)
{
}

QString ItemAtomImpl::title() const
{
    return m_entry.title();
}

QString ItemAtomImpl::link() const
{
    QList<Link> links = m_entry.links();
    if (links.isEmpty())
        return QString::null;
    // TODO: if there is more than one link, find the best suited one (like text/html)
    
    return links.first().href();
}

QString ItemAtomImpl::description() const
{
    return m_entry.summary();
}

QString ItemAtomImpl::content() const
{
    Content content = m_entry.content();
    if (content.isNull() || content.format() == Content::Binary)
        return QString::null;
    
    return content.asString();
}

QString ItemAtomImpl::author() const
{
    QList<Person> authors = m_entry.authors();
    
    if (authors.isEmpty())
        return QString::null;
    
    QString name = authors.first().name();
    QString mail = authors.first().email();
    // TODO: this needs i18n. maybe create a person struct for the abstraction, too?
    return QString("%1 <%2>").arg(name).arg(mail);
}

time_t ItemAtomImpl::datePublished() const 
{
    return m_entry.published();
}

time_t ItemAtomImpl::dateUpdated() const 
{
    time_t upd = m_entry.updated();
    if (upd == 0)
        return m_entry.published();
}
   
QString ItemAtomImpl::language() const
{
    return m_entry.xmlLang();
}

QString ItemAtomImpl::id() const
{
    QString id = m_entry.id();
    if (id.isEmpty())
    {
        // TODO: calc hash
        return QString::null;
    }
    return id;
}


QList<LibSyndication::EnclosurePtr> ItemAtomImpl::enclosures() const
{
    QList<LibSyndication::EnclosurePtr> list;

    QList<LibSyndication::Atom::Link> links = m_entry.links();
    QList<LibSyndication::Atom::Link>::ConstIterator it = links.begin();
    QList<LibSyndication::Atom::Link>::ConstIterator end = links.end();

    for ( ; it != end; ++it)
    {
        if ((*it).rel() == QString::fromLatin1("enclosure"))
        {
            EnclosureAtomImplPtr impl = new EnclosureAtomImpl(*it);
            list.append(LibSyndication::EnclosurePtr::staticCast(impl));
        }
    }

    return list;
}

} // namespace Mapper
} // namespace LibSyndication
