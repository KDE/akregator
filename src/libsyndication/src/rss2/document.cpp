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

#include "../constants.h"
#include "../documentvisitor.h"
#include "../tools.h"

#include "category.h"
#include "cloud.h"
#include "constants.h"
#include "document.h"
#include "image.h"
#include "item.h"
#include "textinput.h"

#include <QDomDocument>
#include <QDomElement>
#include <QList>
#include <QSet>
#include <QString>

#include <kstaticdeleter.h>

namespace LibSyndication {
namespace RSS2 {


Document::Document(const QDomElement& element) : AbstractDocument(), ElementWrapper(element)
{
}

Document Document::fromXML(const QDomDocument& doc)
{
    QDomNode channelNode = doc.namedItem(QString::fromUtf8("rss")).namedItem(QString::fromUtf8("channel"));

    return Document(channelNode.toElement());
}

Document::Document() : AbstractDocument(), ElementWrapper()
{
}

QString Document::title() const
{
    QString t = extractElementText(QString::fromUtf8("title"));
    
    if (t.isNull())
    {
        t = extractElementTextNS(LibSyndication::Constants::dublinCoreNamespace(),
                                 QString::fromUtf8("title"));
    }
    
    return t;
}

QString Document::link() const
{
    return extractElementText(QString::fromUtf8("link") );
}

QString Document::description() const
{
    QString d = extractElementText(QString::fromUtf8("description"));
    
    if (d.isNull())
    {
        d = extractElementTextNS(LibSyndication::Constants::dublinCoreNamespace(),
                                 QString::fromUtf8("description"));
    }

    return htmlize(d);
}

QString Document::language() const
{
    return extractElementText(QString::fromUtf8("language"));
}

QString Document::copyright() const
{
    QString rights = extractElementText(QString::fromUtf8("copyright"));
    if (!rights.isNull())
    {
        return rights;
    }
    else
    {
        // if <copyright> is not provided, use <dc:rights>
        return extractElementTextNS(
                LibSyndication::Constants::dublinCoreNamespace(), QString::fromUtf8("rights"));
    }
}

QString Document::managingEditor() const
{
    return extractElementText(QString::fromUtf8("managingEditor"));
}

QString Document::webMaster() const
{
    return extractElementText(QString::fromUtf8("webMaster"));
}

time_t Document::pubDate() const
{
    QString str = extractElementText(QString::fromUtf8("pubDate"));
    
    if (!str.isNull())
    {
        return parseRFCDate(str);
    }
    else
    {   // if there is no pubDate, check for dc:date
        str = extractElementTextNS(LibSyndication::Constants::dublinCoreNamespace(), QString::fromUtf8("date"));
        return parseISODate(str);
    }
}

time_t Document::lastBuildDate() const
{
    QString str = extractElementText(QString::fromUtf8("lastBuildDate"));
    
    return parseRFCDate(str);
}

QList<Category> Document::categories() const
{
    QList<Category> categories;

    QList<QDomElement> catNodes = elementsByTagName(QString::fromUtf8("category"));
    for (QList<QDomElement>::ConstIterator it = catNodes.begin(); it != catNodes.end(); ++it)
    {
        categories.append(Category(*it));
    }
    
    return categories;
}

QString Document::generator() const
{
    return extractElementText(QString::fromUtf8("generator"));
}

QString Document::docs() const
{
    return extractElementText(QString::fromUtf8("docs"));
}

Cloud Document::cloud() const
{
    return Cloud(element().namedItem(QString::fromUtf8("cloud")).toElement());
}

int Document::ttl() const
{
    bool ok;
    int c;

    QString text = extractElementText(QString::fromUtf8("ttl"));
    c = text.toInt(&ok);
    return ok ? c : 0;
}

Image Document::image() const
{
    QDomNode imageNode = element().namedItem(QString::fromUtf8("image"));
    return Image(imageNode.toElement());
}

TextInput Document::textInput() const
{
    QDomNode textInputNode = element().namedItem(QString::fromUtf8("textInput"));
    return TextInput(textInputNode.toElement());
}

QSet<int> Document::skipHours() const
{
    QSet<int> skipHours;
    QDomNode skipHoursNode = element().namedItem(QString::fromUtf8("skipHours"));
    if (skipHoursNode.isElement())
    {
        ElementWrapper skipHoursWrapper(skipHoursNode.toElement());
        bool ok = false;
        QList<QDomElement> hours = skipHoursWrapper.elementsByTagName(QString::fromUtf8("hour"));
        for (QList<QDomElement>::ConstIterator it = hours.begin(); it != hours.end(); ++it)
        {
            int h = (*it).text().toInt(&ok);
            if (ok)
                skipHours.insert(h);
        }
    }

    return skipHours;
}

QSet<Document::DayOfWeek> Document::skipDays() const
{
    QSet<DayOfWeek> skipDays;
    QDomNode skipDaysNode = element().namedItem(QString::fromUtf8("skipDays"));
    if (skipDaysNode.isElement())
    {
        ElementWrapper skipDaysWrapper(skipDaysNode.toElement());
        QHash<QString, DayOfWeek> weekDays;

        weekDays[QString::fromUtf8("Monday")] = Monday;
        weekDays[QString::fromUtf8("Tuesday")] = Tuesday;
        weekDays[QString::fromUtf8("Wednesday")] = Wednesday;
        weekDays[QString::fromUtf8("Thursday")] = Thursday;
        weekDays[QString::fromUtf8("Friday")] = Friday;
        weekDays[QString::fromUtf8("Saturday")] = Saturday;
        weekDays[QString::fromUtf8("Sunday")] = Sunday;

        QList<QDomElement> days = skipDaysWrapper.elementsByTagName( QString::fromUtf8("day"));
        for (QList<QDomElement>::ConstIterator it = days.begin(); it != days.end(); ++it)
        {
            if (weekDays.contains((*it).text()))
                skipDays.insert(weekDays[(*it).text()]);
        }
    }

    return skipDays;
}

QList<Item> Document::items() const
{
    QList<QDomElement> itemNodes = elementsByTagName(QString::fromUtf8("item"));

    QList<Item> items;

    for (QList<QDomElement>::ConstIterator it = itemNodes.begin(); it != itemNodes.end(); ++it)
    {
        items.append(Item(*it));
    }

    return items;
}

QString Document::debugInfo() const
{
    QString info;
    info += "### Document: ###################\n";
    if (!title().isNull())
        info += "title: #" + title() + "#\n";
    if (!description().isNull())
        info += "description: #" + description() + "#\n";
    if (!link().isNull())
        info += "link: #" + link() + "#\n";
    if (!language().isNull())
        info += "language: #" + language() + "#\n";
    if (!copyright().isNull())
        info += "copyright: #" + copyright() + "#\n";
    if (!managingEditor().isNull())
        info += "managingEditor: #" + managingEditor() + "#\n";
    if (!webMaster().isNull())
    info += "webMaster: #" + webMaster() + "#\n";
    
    QString dpubdate = dateTimeToString(pubDate());
    if (!dpubdate.isNull())
        info += "pubDate: #" + dpubdate + "#\n";
    
    QString dlastbuilddate = dateTimeToString(lastBuildDate());
    if (!dlastbuilddate.isNull())
        info += "lastBuildDate: #" + dlastbuilddate + "#\n";
    
    if (!textInput().isNull())
        info += textInput().debugInfo();
    if (!cloud().isNull())
        info += cloud().debugInfo();
    if (!image().isNull())
        info += image().debugInfo();
    
    QList<Category> cats = categories();
    
    for (QList<Category>::ConstIterator it = cats.begin(); it != cats.end(); ++it)
        info += (*it).debugInfo();
    QList<Item> litems = items();
    for (QList<Item>::ConstIterator it = litems.begin(); it != litems.end(); ++it)
        info += (*it).debugInfo();
    info += "### Document end ################\n";
    return info;
}

bool Document::accept(DocumentVisitor* visitor)
{
    return visitor->visitRSS2Document(this);
}

} // namespace RSS2
} // namespace LibSyndication
