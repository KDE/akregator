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

#ifndef LIBSYNDICATION_ATOM_SOURCE_H
#define LIBSYNDICATION_ATOM_SOURCE_H

#include "../elementwrapper.h"

#include <ctime>

class QDomElement;
class QString;

template <class T> class QList;

namespace LibSyndication {
namespace Atom {

class Category;
class Generator;
class Link;
class Person;

class Source : public ElementWrapper
{
    public:

        Source();
        Source(const QDomElement& element);

        QList<Person> authors() const;

        QList<Person> contributors() const;

        QList<Category> categories() const;

        Generator generator() const;

        QString icon() const;

        QString id() const;

        QList<Link> links() const;

        QString logo() const;

        QString rights() const;

        QString subtitle() const;

        QString title() const;

        time_t updated() const;

        QString debugInfo() const;
};

} // namespace Atom
} // namespace LibSyndication

#endif // LIBSYNDICATION_ATOM_SOURCE_H
