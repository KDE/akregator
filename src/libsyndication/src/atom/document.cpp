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
#include "document.h"
#include "entry.h"
#include "generator.h"
#include "link.h"
#include "person.h"
#include "atomtools.h"

#include <documentvisitor.h>
#include <tools.h>

#include <QDomElement>
#include <QList>
#include <QString>

namespace LibSyndication {
namespace Atom {

FeedDocument::FeedDocument() : ElementWrapper()
{
}

FeedDocument::FeedDocument(const QDomElement& element)  : ElementWrapper(element)
{
}

bool FeedDocument::accept(DocumentVisitor* visitor)
{
    return visitor->visitAtomFeedDocument(this);
}

QList<Person> FeedDocument::authors() const
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

QList<Person> FeedDocument::contributors() const
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

QList<Category> FeedDocument::categories() const
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

Generator FeedDocument::generator() const
{
    return Generator(firstElementByTagNameNS(atom1Namespace(),
                     QString::fromUtf8("generator")));
}

QString FeedDocument::icon() const
{
    return completeURI(extractElementTextNS(atom1Namespace(),
                       QString::fromUtf8("icon")));

}

QString FeedDocument::logo() const
{
    return completeURI(extractElementTextNS(atom1Namespace(),
                       QString::fromUtf8("logo")));
}

QString FeedDocument::id() const
{
    return extractElementTextNS(atom1Namespace(),
                                QString::fromUtf8("id"));
}

QString FeedDocument::rights() const
{

    return extractAtomText(*this, QString::fromUtf8("rights"));
}

QString FeedDocument::title() const
{
    return extractAtomText(*this, QString::fromUtf8("title"));
}

QString FeedDocument::subtitle() const
{
    return extractAtomText(*this, QString::fromUtf8("subtitle"));
}

time_t FeedDocument::updated() const
{
    QString upd = extractElementTextNS(atom1Namespace(),
                                       QString::fromUtf8("updated"));
    return parseDate(upd, ISODate);
}

QList<Link> FeedDocument::links() const
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

QList<Entry> FeedDocument::entries() const
{
    QList<QDomElement> a = 
            elementsByTagNameNS(atom1Namespace(),
                                QString::fromUtf8("entry"));
    QList<Entry> list;

    QList<QDomElement>::ConstIterator it = a.begin();
    QList<QDomElement>::ConstIterator end = a.end();


    for ( ; it != end; ++it)
    {
        list.append(Entry(*it));
    }

    return list;
}

bool FeedDocument::isValid() const
{
    return !isNull();
}

QString FeedDocument::debugInfo() const
{
    QString info;
    info += "### FeedDocument: ###################\n";
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
    
    
    QString dupdated = dateTimeToString(updated());
    if (!dupdated.isNull())
        info += "updated: #" + dupdated + "#\n";
    
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
    
    QList<Entry> dentries = entries();
    QList<Entry>::ConstIterator endentries = dentries.end();
    for (QList<Entry>::ConstIterator it = dentries.begin(); it != endentries; ++it)
        info += (*it).debugInfo();
    
    info += "### FeedDocument end ################\n";

    return info;
}

EntryDocument::EntryDocument() : ElementWrapper()
{
}

EntryDocument::EntryDocument(const QDomElement& element)  : ElementWrapper(element)
{
}

bool EntryDocument::accept(DocumentVisitor* visitor)
{
    return visitor->visitAtomEntryDocument(this);
}

Entry EntryDocument::entry() const
{
    return Entry(element());
}

bool EntryDocument::isValid() const
{
    return !isNull();
}

QString EntryDocument::debugInfo() const
{
    QString info;
    info += "### EntryDocument: ##################\n";
    
    Entry dentry = entry();
    if (!dentry.isNull())
        info += dentry.debugInfo();
    
    info += "### EntryDocument end ###############\n";
    return info;
}

} // namespace Atom
} // namespace LibSyndication
