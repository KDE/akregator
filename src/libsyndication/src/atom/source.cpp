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
#include "constants.h"
#include "generator.h"
#include "link.h"
#include "person.h"
#include "source.h"
#include "tools.h"

#include <QDateTime>
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
    QList<QDomElement> a = 
            Tools::elementsByTagNameNS(element(), Constants::atom1NameSpace(), 
                                       QString::fromLatin1("author"));
    QList<Person> list;
                                       
    QList<QDomElement>::ConstIterator it = a.begin();
    QList<QDomElement>::ConstIterator end = a.end();
    
    
    for ( ; it != end; ++it)
    {
        list.append(Person(*it));
    }
        
    return list;
}

QList<Person> Source::contributors() const
{
        QList<QDomElement> a = 
                Tools::elementsByTagNameNS(element(), Constants::atom1NameSpace(), 
                                       QString::fromLatin1("contributor"));
    QList<Person> list;
                                       
    QList<QDomElement>::ConstIterator it = a.begin();
    QList<QDomElement>::ConstIterator end = a.end();
    
    
    for ( ; it != end; ++it)
    {
        list.append(Person(*it));
    }
        
    return list;
}

QList<Category> Source::categories() const
{
    QList<QDomElement> a = 
            Tools::elementsByTagNameNS(element(), Constants::atom1NameSpace(),
                                       QString::fromLatin1("category"));
    QList<Category> list;
    
    QList<QDomElement>::ConstIterator it = a.begin();
    QList<QDomElement>::ConstIterator end = a.end();


    for ( ; it != end; ++it)
    {
        list.append(Category(*it));
    }

    return list;
}

Generator Source::generator() const
{
    return Generator(Tools::firstElementByTagNameNS(element(),
                     Constants::atom1NameSpace(),
                     QString::fromLatin1("generator")));
}

QString Source::icon() const
{
    return Tools::extractElementTextNS(element(), 
                                       Constants::atom1NameSpace(),
                                       QString::fromLatin1("icon"));
}

QString Source::id() const
{
    return Tools::extractElementTextNS(element(), 
                                       Constants::atom1NameSpace(),
                                       QString::fromLatin1("id"));
}

QList<Link> Source::links() const
{
    QList<QDomElement> a = 
            Tools::elementsByTagNameNS(element(), Constants::atom1NameSpace(),
                                       QString::fromLatin1("link"));
    QList<Link> list;

    QList<QDomElement>::ConstIterator it = a.begin();
    QList<QDomElement>::ConstIterator end = a.end();


    for ( ; it != end; ++it)
    {
        list.append(Link(*it));
    }

    return list;
}

QString Source::logo() const
{
    return Tools::extractElementTextNS(element(), 
                                       Constants::atom1NameSpace(),
                                       QString::fromLatin1("logo"));
}

QString Source::rights() const
{
    return Tools::extractElementTextNS(element(), 
                                       Constants::atom1NameSpace(),
                                       QString::fromLatin1("rights"));
}

QString Source::subtitle() const
{
    return Tools::extractElementTextNS(element(), 
                                       Constants::atom1NameSpace(),
                                       QString::fromLatin1("subtitle"));
}

QString Source::title() const
{
    return Tools::extractElementTextNS(element(), 
                                       Constants::atom1NameSpace(),
                                       QString::fromLatin1("title"));
}

time_t Source::updated() const
{
    QString upd = Tools::extractElementTextNS(element(), 
                                              Constants::atom1NameSpace(),
                                              QString::fromLatin1("updated"));
    QDateTime dt = QDateTime::fromString(upd, Qt::ISODate);
    return dt.toTime_t();
}

QString Source::debugInfo() const
{
    return "TODO";
}

} // namespace Atom
} //namespace LibSyndication
