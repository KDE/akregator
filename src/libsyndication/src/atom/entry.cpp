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

#include "category.h"
#include "content.h"
#include "entry.h"
#include "link.h"
#include "person.h"
#include "source.h"

#include <QDomElement>
#include <QList>
#include <QString>

namespace LibSyndication {
namespace Atom {

Entry::Entry() : ElementWrapper()
{
}

Entry::Entry(const QDomElement& element) : ElementWrapper(element)
{
}

QList<Person> Entry::authors() const
{
    return QList<Person>(); // TODO
}

QList<Category> Entry::categories() const
{
    return QList<Category>(); // TODO
}

QList<Person> Entry::contributors() const
{
    return QList<Person>(); // TODO
}

QString Entry::id() const
{
    return "TODO";
}

QList<Link> Entry::links() const
{
    return QList<Link>();  // TODO
}

QString Entry::rights() const
{
    return "TODO";
}

Source Entry::source() const
{
    return Source(); // TODO
}

time_t Entry::published() const
{
    return 0; // TODO
}

time_t Entry::updated() const
{
    return 0; // TODO
}

QString Entry::summary() const
{
    return "TODO";
}

QString Entry::title() const
{
    return "TODO";
}

Content Entry::content() const
{
    return Content(); // TODO
}

QString Entry::debugInfo() const
{
    return "TODO";
}

} // namespace Atom
} //namespace LibSyndication

