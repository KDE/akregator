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

#include "../tools.h"

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
            elementsByTagNameNS(Constants::atom1NameSpace(),
                                QString::fromUtf8("author"));
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
                elementsByTagNameNS(Constants::atom1NameSpace(),
                                    QString::fromUtf8("contributor"));
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
            elementsByTagNameNS(Constants::atom1NameSpace(),
                                QString::fromUtf8("category"));
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
    return Generator(firstElementByTagNameNS(Constants::atom1NameSpace(),
                     QString::fromUtf8("generator")));
}

QString Source::icon() const
{
    return extractElementTextNS(Constants::atom1NameSpace(),
                                QString::fromUtf8("icon"));
}

QString Source::id() const
{
    return extractElementTextNS(Constants::atom1NameSpace(),
                                QString::fromUtf8("id"));
}

QList<Link> Source::links() const
{
    QList<QDomElement> a = 
            elementsByTagNameNS(Constants::atom1NameSpace(),
                                QString::fromUtf8("link"));
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
    return extractElementTextNS(Constants::atom1NameSpace(),
                                QString::fromUtf8("logo"));
}

QString Source::rights() const
{
    return extractAtomText(*this, QString::fromUtf8("rights"));
}

QString Source::subtitle() const
{
    return extractAtomText(*this, QString::fromUtf8("subtitle"));
}

QString Source::title() const
{
    return extractAtomText(*this, QString::fromUtf8("title"));
}

time_t Source::updated() const
{
    QString upd = extractElementTextNS(Constants::atom1NameSpace(),
                                       QString::fromUtf8("updated"));
    return parseISODate(upd);
}

QString Source::debugInfo() const
{
    QString info;
    info += "### Source: ###################\n";
    if (!title().isEmpty())
        info += "title: #" + title() + "#\n";
    if (!subtitle().isEmpty())
        info += "subtitle: #" + subtitle() + "#\n";
    if (!id().isEmpty())
        info += "id: #" + id() + "#\n";

    if (!rights().isEmpty())
        info += "rights: #" + rights() + "#\n";
    if (!icon().isEmpty())
        info += "icon: #" + icon() + "#\n";
    if (!logo().isEmpty())
        info += "logo: #" + logo() + "#\n";
    if (!generator().isNull())
        info += generator().debugInfo();
    
    
    QDateTime dupdated;
    dupdated.setTime_t(updated()); 
    info += "updated: #" + dupdated.toString(Qt::ISODate) + "#\n";
    
    QList<Link> dlinks = links();
    QList<Link>::ConstIterator endlinks = dlinks.end();
    for (QList<Link>::ConstIterator it = dlinks.begin(); it != endlinks; ++it)
        info += (*it).debugInfo();
    
    QList<Category> dcats = categories();
    QList<Category>::ConstIterator endcats = dcats.end();
    for (QList<Category>::ConstIterator it = dcats.begin(); it != endcats; ++it)
        info += (*it).debugInfo();

    info += "### Authors: ###################\n";
    
    QList<Person> dauthors = authors();
    QList<Person>::ConstIterator endauthors = dauthors.end();
    for (QList<Person>::ConstIterator it = dauthors.begin(); it != endauthors; ++it)
        info += (*it).debugInfo();

    info += "### Contributors: ###################\n";
    
    QList<Person> dcontri = contributors();
    QList<Person>::ConstIterator endcontri = dcontri.end();
    for (QList<Person>::ConstIterator it = dcontri.begin(); it != endcontri; ++it)
        info += (*it).debugInfo();
    
    info += "### Source end ################\n";

    return info;
}

} // namespace Atom
} //namespace LibSyndication
