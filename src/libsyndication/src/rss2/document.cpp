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

#include <constants.h>
#include <documentvisitor.h>
#include <tools.h>

#include <rss2/category.h>
#include <rss2/cloud.h>
#include <rss2/document.h>
#include <rss2/image.h>
#include <rss2/item.h>
#include <rss2/textinput.h>

#include <QDomDocument>
#include <QList>
#include <QSet>
#include <QString>

#include <kstaticdeleter.h>

namespace LibSyndication {
namespace RSS2 {


Document::Document(const QDomElement& element) : SpecificDocument(), ElementWrapper(element)
{
}

Document Document::fromXML(const QDomDocument& doc)
{
    QDomNode channelNode = doc.namedItem(QString::fromUtf8("rss")).namedItem(QString::fromUtf8("channel"));

    return Document(channelNode.toElement());
}

Document::Document() : SpecificDocument(), ElementWrapper()
{
}

bool Document::isValid() const
{
    return !isNull();
}
        
QString Document::title() const
{
    QString t = extractElementTextNS(QString(), QString::fromUtf8("title"));
    
    if (t.isNull())
    {
        t = extractElementTextNS(dublinCoreNamespace(),
                                 QString::fromUtf8("title"));
    }
    
    return t;
}

QString Document::link() const
{
    return extractElementTextNS(QString(), QString::fromUtf8("link") );
}

QString Document::description() const
{
    QString d = extractElementTextNS(QString(), QString::fromUtf8("description"));
    
    if (d.isNull())
    {
        d = extractElementTextNS(dublinCoreNamespace(),
                                 QString::fromUtf8("description"));
    }

    return htmlize(d);
}

QString Document::language() const
{
    QString lang = extractElementTextNS(QString(),
                                        QString::fromUtf8("language"));
    
    if (!lang.isNull())
    {
        return lang;
    }
    else
    {
        return extractElementTextNS(
            dublinCoreNamespace(), QString::fromUtf8("language"));   
    }
    
}

QString Document::copyright() const
{
    QString rights = extractElementTextNS(QString(),
                                          QString::fromUtf8("copyright"));
    if (!rights.isNull())
    {
        return rights;
    }
    else
    {
        // if <copyright> is not provided, use <dc:rights>
        return extractElementTextNS(dublinCoreNamespace(),
                                    QString::fromUtf8("rights"));
    }
}

QString Document::managingEditor() const
{
    return extractElementTextNS(QString(), QString::fromUtf8("managingEditor"));
}

QString Document::webMaster() const
{
    return extractElementTextNS(QString(), QString::fromUtf8("webMaster"));
}

time_t Document::pubDate() const
{
    QString str = extractElementTextNS(QString(), QString::fromUtf8("pubDate"));
    
    if (!str.isNull())
    {
        return parseDate(str, RFCDate);
    }
    else
    {   // if there is no pubDate, check for dc:date
        str = extractElementTextNS(dublinCoreNamespace(), QString::fromUtf8("date"));
        return parseDate(str, ISODate);
    }
}

time_t Document::lastBuildDate() const
{
    QString str = extractElementTextNS(QString(), QString::fromUtf8("lastBuildDate"));
    
    return parseDate(str, RFCDate);
}

QList<Category> Document::categories() const
{
    QList<Category> categories;

    QList<QDomElement> catNodes = elementsByTagNameNS(QString(),
            QString::fromUtf8("category"));
    QList<QDomElement>::ConstIterator it = catNodes.begin();
    for ( ; it != catNodes.end(); ++it)
    {
        categories.append(Category(*it));
    }
    
    return categories;
}

QString Document::generator() const
{
    return extractElementTextNS(QString(), QString::fromUtf8("generator"));
}

QString Document::docs() const
{
    return extractElementTextNS(QString(), QString::fromUtf8("docs"));
}

Cloud Document::cloud() const
{
    return Cloud(firstElementByTagNameNS(QString(), QString::fromUtf8("cloud")));
}

int Document::ttl() const
{
    bool ok;
    int c;

    QString text = extractElementTextNS(QString(), QString::fromUtf8("ttl"));
    c = text.toInt(&ok);
    return ok ? c : 0;
}

Image Document::image() const
{
    return Image(firstElementByTagNameNS(QString(), QString::fromUtf8("image")));
}

TextInput Document::textInput() const
{
    TextInput ti = firstElementByTagNameNS(QString(), QString::fromUtf8("textInput"));
    
    if (!ti.isNull())
        return ti;
    
    // Netscape's version of RSS 0.91 has textinput, not textInput
    return firstElementByTagNameNS(QString(), QString::fromUtf8("textinput"));
}

QSet<int> Document::skipHours() const
{
    QSet<int> skipHours;
    QDomElement skipHoursNode = firstElementByTagNameNS(QString(),
            QString::fromUtf8("skipHours"));
    if (!skipHoursNode.isNull())
    {
        ElementWrapper skipHoursWrapper(skipHoursNode);
        bool ok = false;
        QList<QDomElement> hours = 
                skipHoursWrapper.elementsByTagNameNS(QString(),
                QString::fromUtf8("hour"));
        QList<QDomElement>::ConstIterator it = hours.begin();
        for ( ; it != hours.end(); ++it)
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
    QDomElement skipDaysNode = firstElementByTagNameNS(QString(), QString::fromUtf8("skipDays"));
    if (!skipDaysNode.isNull())
    {
        ElementWrapper skipDaysWrapper(skipDaysNode);
        QHash<QString, DayOfWeek> weekDays;

        weekDays[QString::fromUtf8("Monday")] = Monday;
        weekDays[QString::fromUtf8("Tuesday")] = Tuesday;
        weekDays[QString::fromUtf8("Wednesday")] = Wednesday;
        weekDays[QString::fromUtf8("Thursday")] = Thursday;
        weekDays[QString::fromUtf8("Friday")] = Friday;
        weekDays[QString::fromUtf8("Saturday")] = Saturday;
        weekDays[QString::fromUtf8("Sunday")] = Sunday;

        QList<QDomElement> days = skipDaysWrapper.elementsByTagNameNS(QString(), QString::fromUtf8("day"));
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
    QList<QDomElement> itemNodes = elementsByTagNameNS(QString(), QString::fromUtf8("item"));

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
