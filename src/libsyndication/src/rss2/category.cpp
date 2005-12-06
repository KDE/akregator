/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>

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

#include "../shared.h"

#include "category.h"
#include "tools.h"

#include <qdom.h>
#include <QString>

#include <kstaticdeleter.h>

class QString;

namespace LibSyndication {
namespace RSS2 {

class Category::CategoryPrivate : public LibSyndication::Shared
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

Category* Category::m_null = 0;
static KStaticDeleter<Category> categorysd;

const Category& Category::null()
{
    if (m_null == 0)
    {
        categorysd.setObject(m_null, new Category);
    }

    return *m_null;
}

bool Category::isNull() const
{
    return d->isNull;
}

Category Category::fromXML(const QDomElement& e)
{
    Category obj;
    if (e.hasAttribute(QString::fromLatin1("domain")))
        obj.setDomain(e.attribute(QString::fromLatin1("domain")));
    obj.setCategory(e.text());

}

QDomElement Category::toXML(QDomDocument document) const
{
    QDomElement e = document.createElement(QString::fromLatin1("category"));
    if (!d->domain.isNull())
        e.setAttribute(QString::fromLatin1("domain"), d->domain);
    if (!d->category.isNull())
        e.appendChild(document.createTextNode(d->category));
    return e;
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

void Category::setCategory(const QString& category)
{
    d = CategoryPrivate::copyOnWrite(d);
    d->isNull = false;
    d->category = category;
}

QString Category::category() const
{
    return !d->isNull ? d->category : QString::null;
}

void Category::setDomain(const QString& domain)
{
    d = CategoryPrivate::copyOnWrite(d);
    d->isNull = false;
    d->domain = domain;
}

QString Category::domain() const
{
    return !d->isNull ? d->domain : QString::null;
}

QString Category::debugInfo() const
{
    QString info;
    info += "### Category: ###################\n";
    info += "category: #" + category() + "#\n";
    info += "domain: #" + domain() + "#\n";
    info += "### Category end ################\n";
    return info;
}

} // namespace RSS2
} // namespace LibSyndication
