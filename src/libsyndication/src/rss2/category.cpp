/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>
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
#include "tools.h"

#include <QDomElement>
#include <QString>

#include <ksharedptr.h>
#include <kstaticdeleter.h>

class QString;

namespace LibSyndication {
namespace RSS2 {

class Category::CategoryPrivate : public KShared
{
    public:

    QString category;
    QString domain;
    
    bool operator==(const CategoryPrivate& other) const
    {
        return category == other.category && domain == other.domain;
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
    return !d;
}

Category Category::fromXML(const QDomElement& e)
{
    QString domain = e.attribute(QString::fromLatin1("domain"));
    QString cat = e.text();

    return Category(cat, domain);
}

Category::Category() : d(0)
{
}

Category::Category(const Category& other) : d(0)
{
     *this = other;
}

Category::Category(const QString& category, const QString& domain) : d(new CategoryPrivate)
{
    d->category = category;
    d->domain = domain;
}

Category::~Category()
{
}

Category& Category::operator=(const Category& other)
{
    d = other.d;
    return *this;
}

bool Category::operator==(const Category &other) const
{
    if (!d || !other.d)
        return d == other.d;
    return *d == *other.d;
}

QString Category::category() const
{
    return d ? d->category : QString::null;
}

QString Category::domain() const
{
    return d ? d->domain : QString::null;
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
