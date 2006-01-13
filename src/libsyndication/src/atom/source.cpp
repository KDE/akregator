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
#include "generator.h"
#include "link.h"
#include "person.h"
#include "source.h"

#include <QDomElement>
#include <QList>
#include <QString>

namespace LibSyndication {
namespace Atom {

Source::Source() : ElementWrapper()
{
}

Source::Source(const QDomElement& element) : ElementWrapper(element)
{
}

QList<Person> Source::authors() const
{
    return QList<Person>(); // TODO
}

QList<Person> Source::contributors() const
{
    return QList<Person>(); // TODO
}

QList<Category> Source::categories() const
{
    return QList<Category>(); // TODO
}

Generator Source::generator() const
{
    return Generator(); // TODO
}

QString Source::icon() const
{
    return "TODO";
}

QString Source::id() const
{
    return "TODO";
}

QList<Link> Source::links() const
{
    return QList<Link>(); // TODO
}

QString Source::logo() const
{
    return "TODO";
}

QString Source::rights() const
{
    return "TODO";
}

QString Source::subtitle() const
{
    return "TODO";
}

QString Source::title() const
{
    return "TODO";
}

time_t Source::updated() const
{
    return 0; // TODO
}

QString Source::debugInfo() const
{
    return "TODO";
}

} // namespace Atom
} //namespace LibSyndication
