/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

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

#include "category.h"
#include "tools_p.h"

#include <qdom.h>
#include <qstring.h>

class QString;

namespace RSS
{

class Category::CategoryPrivate : public Shared
{
    public:
    bool isNull;
    QString category;
    QString domain;

    bool operator==(const CategoryPrivate &other) const
    {
        return (isNull && other.isNull) || (category == other.category && domain == other.domain);
    }

    static CategoryPrivate* copyOnWrite(CategoryPrivate* ep)
    {
        if (ep->count > 1)
        {
            ep->deref();
            ep = new CategoryPrivate(*ep);
        }
        return ep;
    }
};

bool Category::isNull() const
{
    return d == 0;
}

Category Category::fromXML(const QDomElement& e)
{
    Category obj;
    if (e.hasAttribute(QString::fromLatin1("domain")))
        obj.d->domain = e.attribute(QString::fromLatin1("domain"));
    obj.d->category = e.text();
    obj.d->isNull = false;
    return obj;
}

Category::Category() : d(new CategoryPrivate)
{
    d->isNull = true;
}

Category::Category(const Category& other) : d(0)
{
     *this = other;
}

Category::Category(const QString& category, const QString& domain) : d(new CategoryPrivate)
{
    d->isNull = false;
    d->category = category;
    d->domain = domain;
}

Category::~Category()
{
    if (d->deref())
    {
        delete d;
        d = 0;
    }
}

Category& Category::operator=(const Category& other)
{
    if (d != other.d)
    {
        other.d->ref();
        if (d && d->deref())
            delete d;
        d = other.d;
    }
    return *this;
}

bool Category::operator==(const Category &other) const
{
    return *d == *other.d;
}

QString Category::category() const
{
    return !d->isNull ? d->category : QString::null;
}

QString Category::domain() const
{
    return !d->isNull ? d->domain : QString::null;
}

} // namespace RSS


