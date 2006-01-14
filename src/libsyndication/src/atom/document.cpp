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
#include "link.h"
#include "person.h"
#include "tools.h"

#include "../documentvisitor.h"

#include <QDomElement>
#include <QDateTime>
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
    return visitor->visit(this);
}

QList<Person> FeedDocument::authors() const
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

QList<Person> FeedDocument::contributors() const
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

QList<Category> FeedDocument::categories() const
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

QString FeedDocument::icon() const
{
    return Tools::extractElementTextNS(element(), 
                                       Constants::atom1NameSpace(),
                                       QString::fromLatin1("icon"));

}

QString FeedDocument::logo() const
{
    return Tools::extractElementTextNS(element(), 
                                       Constants::atom1NameSpace(),
                                       QString::fromLatin1("logo"));
}

QString FeedDocument::id() const
{
    return Tools::extractElementTextNS(element(), 
                                       Constants::atom1NameSpace(),
                                       QString::fromLatin1("id"));
}

QString FeedDocument::rights() const
{
    return Tools::extractElementTextNS(element(), 
                                       Constants::atom1NameSpace(),
                                       QString::fromLatin1("rights"));
}

QString FeedDocument::title() const
{
    return Tools::extractElementTextNS(element(), 
                                       Constants::atom1NameSpace(),
                                       QString::fromLatin1("title"));
}

QString FeedDocument::subtitle() const
{
    return Tools::extractElementTextNS(element(), 
                                       Constants::atom1NameSpace(),
                                       QString::fromLatin1("subtitle"));
}

time_t FeedDocument::updated() const
{
    QString upd = Tools::extractElementTextNS(element(), 
                                              Constants::atom1NameSpace(),
                                              QString::fromLatin1("updated"));
    QDateTime dt = QDateTime::fromString(upd, Qt::ISODate);
    return dt.toTime_t();
}

QList<Link> FeedDocument::links() const
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

QList<Entry> FeedDocument::entries() const
{
    QList<QDomElement> a = 
            Tools::elementsByTagNameNS(element(), Constants::atom1NameSpace(),
                                       QString::fromLatin1("entry"));
    QList<Entry> list;

    QList<QDomElement>::ConstIterator it = a.begin();
    QList<QDomElement>::ConstIterator end = a.end();


    for ( ; it != end; ++it)
    {
        list.append(Entry(*it));
    }

    return list;
}

QString FeedDocument::debugInfo() const
{
    return "TODO";
}

EntryDocument::EntryDocument() : ElementWrapper()
{
}

EntryDocument::EntryDocument(const QDomElement& element)  : ElementWrapper(element)
{
}

bool EntryDocument::accept(DocumentVisitor* visitor)
{
    return visitor->visit(this);
}

Entry EntryDocument::entry() const
{
    return Entry(Tools::firstElementByTagNameNS(element(),
                 Constants::atom1NameSpace(),
                 QString::fromLatin1("entry")));
}

QString EntryDocument::debugInfo() const
{
    return "TODO";
}

} // namespace Atom
} // namespace LibSyndication
