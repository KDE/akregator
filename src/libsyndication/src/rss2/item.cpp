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
    QString t = extractElementText(QString::fromLatin1("title"));
    
    if (!t.isNull())
    {
        return t;
    }
    else
    {
        return extractElementTextNS(LibSyndication::Constants::dublinCoreNamespace(),
                                    QString::fromLatin1("title"));
    }
}

QString Item::link() const
{
    return extractElementText(QString::fromLatin1("link") );
}

QString Item::description() const
{
    QString d = extractElementText(QString::fromLatin1("description"));
    
    if (!d.isNull())
    {
        return d;
    }
    else
    {
        return extractElementTextNS(LibSyndication::Constants::dublinCoreNamespace(),
                                    QString::fromLatin1("description"));
    }
}

QString Item::content() const
{
    // parse encoded stuff from content:encoded, xhtml:body and friends into content
    return Tools::extractContent(*this);
}

QList<Category> Item::categories() const
{
    QList<QDomElement> cats = elementsByTagName(QString::fromLatin1("category"));

    QList<Category> categories;

    for (QList<QDomElement>::ConstIterator it = cats.begin(); it != cats.end(); ++it)
    {
        categories.append(Category(*it));
    }
    return categories;
}

QString Item::comments() const
{
    return extractElementText(QString::fromLatin1("comments") );
}

QString Item::author() const
{
    QString a = extractElementText(QString::fromLatin1("author") );
    if (!a.isNull()) 
    {
        return a;
    }
    else
    {
        // if author is not available, fall back to dc:creator
        return extractElementTextNS(Constants::dublinCoreNamespace(), QString::fromLatin1("creator") );
    }
    
}

Enclosure Item::enclosure() const
{
    QDomNode enc = element().namedItem(QString::fromLatin1("enclosure"));
    return Enclosure(enc.toElement());
}

QString Item::guid() const
{
    QString guid;

    QDomNode guidNode = element().namedItem(QString::fromLatin1("guid"));
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

    QDomNode guidNode = element().namedItem(QString::fromLatin1("guid"));
    if (guidNode.isElement())
    {
        QDomElement guidElem = guidNode.toElement();

        if (guidElem.attribute(QString::fromLatin1("isPermaLink")) == QString::fromLatin1("false"))
            guidIsPermaLink = false;
    }

    return guidIsPermaLink;
}

QDateTime Item::pubDate() const
{
    QDateTime pubDate;

    QString pubDateStr = extractElementText(QString::fromLatin1("pubDate"));
    
    if (!pubDateStr.isNull())
    {
        time_t time = KRFCDate::parseDate(pubDateStr);
        pubDate.setTime_t(time);
    }
    else
    {   // if there is no pubDate, check for dc:date
        pubDateStr = extractElementTextNS(LibSyndication::Constants::dublinCoreNamespace(), QString::fromLatin1("date"));
        
        if (!pubDateStr.isNull())
        {
            pubDate = QDateTime::fromString(pubDateStr, Qt::ISODate);
        }
    }
    
    return pubDate;
}

Source Item::source() const
{
    QDomNode s = element().namedItem(QString::fromLatin1("source"));
    return Source(s.toElement());
}

QString Item::debugInfo() const
{
    QString info;
    info += "### Item: ###################\n";
    info += "title: #" + title() + "#\n";
    info += "link: #" + link() + "#\n";
    info += "description: #" + description() + "#\n";
    info += "content: #" + content() + "#\n";
    info += "author: #" + author() + "#\n";
    info += "comments: #" + comments() + "#\n";
    info += "pubDate: #" + pubDate().toString() + "#\n";
    info += "guid: #" + guid() + "#\n";
    info += "guid is PL: #" + (guidIsPermaLink() ? QString("true") : QString("false")) + "#\n";
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
