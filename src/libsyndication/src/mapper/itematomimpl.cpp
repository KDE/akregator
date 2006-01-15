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

#include "../atom/content.h"
#include "../atom/link.h"
#include "../atom/person.h"

#include "itematomimpl.h"

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

//LibSyndication::Enclosure ItemAtomImpl::enclosure() const
//{
//}

} // namespace Mapper
} // namespace LibSyndication
