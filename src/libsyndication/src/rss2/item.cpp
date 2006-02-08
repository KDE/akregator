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
#include "enclosure.h"
#include "item.h"
#include "source.h"
#include "tools.h"

#include "../constants.h"
#include "../tools.h"

#include <QDateTime>
#include <QDomElement>
#include <QString>
#include <QList>

#include <krfcdate.h>

namespace LibSyndication {
namespace RSS2 {

Item::Item() : ElementWrapper()
{
}

Item::Item(const QDomElement& element) : ElementWrapper(element)
{
}

QString Item::title() const
{
    QString t = extractElementText(QString::fromUtf8("title"));
    
    if (t.isNull())
    {
        t = extractElementTextNS(LibSyndication::Constants::dublinCoreNamespace(),
                                 QString::fromUtf8("title"));
    }
    return htmlize(t);
}

QString Item::link() const
{
    return extractElementText(QString::fromUtf8("link") );
}

QString Item::description() const
{
    QString d = extractElementText(QString::fromUtf8("description"));
    
    if (d.isNull())
    {
        d = extractElementTextNS(LibSyndication::Constants::dublinCoreNamespace(),
                                 QString::fromUtf8("description"));
    }
    
    return htmlize(d);
}

QString Item::content() const
{
    // parse encoded stuff from content:encoded, xhtml:body and friends into content
    return extractContent(*this);
}

QList<Category> Item::categories() const
{
    QList<QDomElement> cats = elementsByTagName(QString::fromUtf8("category"));

    QList<Category> categories;

    for (QList<QDomElement>::ConstIterator it = cats.begin(); it != cats.end(); ++it)
    {
        categories.append(Category(*it));
    }
    return categories;
}

QString Item::comments() const
{
    return extractElementText(QString::fromUtf8("comments") );
}

QString Item::author() const
{
    QString a = extractElementText(QString::fromUtf8("author") );
    if (!a.isNull()) 
    {
        return a;
    }
    else
    {
        // if author is not available, fall back to dc:creator
        return extractElementTextNS(Constants::dublinCoreNamespace(), QString::fromUtf8("creator") );
    }
    
}

Enclosure Item::enclosure() const
{
    QDomNode enc = element().namedItem(QString::fromUtf8("enclosure"));
    return Enclosure(enc.toElement());
}

QString Item::guid() const
{
    QString guid;

    QDomNode guidNode = element().namedItem(QString::fromUtf8("guid"));
    if (guidNode.isElement())
    {
        QDomElement guidElem = guidNode.toElement();
        guid = guidElem.text();
    }

    return guid;
}

bool Item::guidIsPermaLink() const
{
    bool guidIsPermaLink = true;  // true is default

    QDomNode guidNode = element().namedItem(QString::fromUtf8("guid"));
    if (guidNode.isElement())
    {
        QDomElement guidElem = guidNode.toElement();

        if (guidElem.attribute(QString::fromUtf8("isPermaLink")) == QString::fromUtf8("false"))
            guidIsPermaLink = false;
    }

    return guidIsPermaLink;
}

time_t Item::pubDate() const
{
    QString str = extractElementText(QString::fromUtf8("pubDate"));
    
    if (!str.isNull())
    {
        return parseRFCDate(str);
    }
    
    // if there is no pubDate, check for dc:date
    str = extractElementTextNS(LibSyndication::Constants::dublinCoreNamespace(), QString::fromUtf8("date"));
    return parseISODate(str);
}

Source Item::source() const
{
    QDomNode s = element().namedItem(QString::fromUtf8("source"));
    return Source(s.toElement());
}

QString Item::debugInfo() const
{
    QString info;
    info += "### Item: ###################\n";
    if (!title().isNull())
        info += "title: #" + title() + "#\n";
    if (!link().isNull())
        info += "link: #" + link() + "#\n";
    if (!description().isNull())
        info += "description: #" + description() + "#\n";
    if (!content().isNull())
        info += "content: #" + content() + "#\n";
    if (!author().isNull())
        info += "author: #" + author() + "#\n";
    if (!comments().isNull())
        info += "comments: #" + comments() + "#\n";
    QDateTime dpubdate;
    dpubdate.setTime_t(pubDate());
    if (dpubdate.isValid())
        info += "pubDate: #" + dpubdate.toString() + "#\n";
    if (!guid().isNull())
        info += "guid: #" + guid() + "#\n";
    if (guidIsPermaLink())
        info += "guid is PL: #true#\n";
    if (!enclosure().isNull())
        info += enclosure().debugInfo();
    if (!source().isNull())
         info += source().debugInfo();
    
    QList<Category> cats = categories();
    for (QList<Category>::ConstIterator it = cats.begin(); it != cats.end(); ++it)
        info += (*it).debugInfo();
    info += "### Item end ################\n";
    return info;
}

} // namespace RSS2
} // namespace LibSyndication
