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
#include "content.h"
#include "entry.h"
#include "link.h"
#include "person.h"
#include "source.h"
#include "atomtools.h"

#include <specificitemvisitor.h>
#include <tools.h>

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
    QList<QDomElement> a = 
            elementsByTagNameNS(atom1Namespace(), 
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

QList<Person> Entry::contributors() const
{
    QList<QDomElement> a = 
            elementsByTagNameNS(atom1Namespace(),
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

QList<Category> Entry::categories() const
{
    QList<QDomElement> a = 
            elementsByTagNameNS(atom1Namespace(),
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

QString Entry::id() const
{
    return extractElementTextNS(atom1Namespace(),
                                QString::fromUtf8("id"));

}

QList<Link> Entry::links() const
{
    QList<QDomElement> a = 
            elementsByTagNameNS(atom1Namespace(), 
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

QString Entry::rights() const
{
    return extractAtomText(*this, QString::fromUtf8("rights"));
}

Source Entry::source() const
{
    return Source(firstElementByTagNameNS(atom1Namespace(),
                  QString::fromUtf8("source")));
}

time_t Entry::published() const
{
    QString pub = extractElementTextNS(atom1Namespace(),
                                       QString::fromUtf8("published"));
    return parseDate(pub, ISODate);
}

time_t Entry::updated() const
{
    QString upd = extractElementTextNS(atom1Namespace(),
                                       QString::fromUtf8("published"));
    return parseDate(upd, ISODate);
}

QString Entry::summary() const
{
    return extractAtomText(*this, QString::fromUtf8("summary"));
}

QString Entry::title() const
{
    return extractAtomText(*this, QString::fromUtf8("title"));
}

Content Entry::content() const
{
    return Content(firstElementByTagNameNS(atom1Namespace(),
                   QString::fromUtf8("content")));
}

QString Entry::debugInfo() const
{
    QString info;
    info += "### Entry: ###################\n";
    if (!title().isEmpty())
        info += "title: #" + title() + "#\n";
    if (!summary().isEmpty())
        info += "summary: #" + summary() + "#\n";
    if (!id().isEmpty())
        info += "id: #" + id() + "#\n";
    if (!content().isNull())
        info += content().debugInfo();
    
    if (!rights().isEmpty())
        info += "rights: #" + rights() + "#\n";
    
    
    QString dupdated = dateTimeToString(updated());
    if (!dupdated.isNull())
        info += "updated: #" + dupdated + "#\n";
    
    QString dpublished = dateTimeToString(published());
    if (!dpublished.isNull())
        info += "published: #" + dpublished + "#\n";
    
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
    
    if (!source().isNull())
        info += source().debugInfo();
    
    info += "### Entry end ################\n";

    return info;
}

bool Entry::accept(SpecificItemVisitor* visitor)
{
    return visitor->visitAtomEntry(this);
}

} // namespace Atom
} //namespace LibSyndication

