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

#include <QDateTime>
#include <qdom.h>
#include <QString>
#include <QList>

#include <krfcdate.h>
#include <kstaticdeleter.h>

namespace LibSyndication {
namespace RSS2 {

class Item::ItemPrivate : public KShared
{
    public:

    QString title;
    QString link;
    QString description;
    QString content;
    QList<Category> categories;
    QString comments;
    QString author;
    QString guid;
    bool guidIsPermaLink;
    QDateTime pubDate;
    Source source;
    Enclosure enclosure;

    bool operator==(const ItemPrivate& other) const
    {
        return (title == other.title &&
                link == other.link &&
                description == other.description &&
                content == other.content &&
                categories == other.categories &&
                comments == other.comments &&
                author == other.author &&
                guid == other.guid &&
                guidIsPermaLink == other.guidIsPermaLink &&
                pubDate == other.pubDate &&
                source == other.source &&
                enclosure == other.enclosure);
    }
};

Item* Item::m_null = 0;
static KStaticDeleter<Item> itemsd;

const Item& Item::null()
{
    if (m_null == 0)
        itemsd.setObject(m_null, new Item);
    return *m_null;
}

Item Item::fromXML(const QDomElement& e)
{
    QString title = Tools::extractElementText(e, QString::fromLatin1("title") );
    QString link = Tools::extractElementText(e, QString::fromLatin1("link") );
    
    QString description = Tools::extractElementText(e, QString::fromLatin1("description") );
    // parse encoded stuff from content:encoded, xhtml:body and friends into content
    QString content = Tools::extractContent(e);
    
    QString author = Tools::extractElementText(e, QString::fromLatin1("author") );

    QString comments = Tools::extractElementText(e, QString::fromLatin1("comments") );

    Enclosure enclosure; 
    QDomNode enc = e.namedItem(QString::fromLatin1("enclosure"));
    if (enc.isElement())
        enclosure = Enclosure::fromXML(enc.toElement());

    Source source;
    QDomNode s = e.namedItem(QString::fromLatin1("source"));
    if (s.isElement())
        source = Source::fromXML(s.toElement());

    QList<QDomElement> cats = Tools::elementsByTagName(e, QString::fromLatin1("category"));

    QList<Category> categories;

    for (QList<QDomElement>::ConstIterator it = cats.begin(); it != cats.end(); ++it)
    {
        Category i = Category::fromXML(*it);
        if (!i.isNull())
            categories.append(i);
    }

    QString guid;

    bool guidIsPermaLink = true;  // true is default

    QDomNode guidNode = e.namedItem(QString::fromLatin1("guid"));
    if (guidNode.isElement())
    {
        QDomElement guidElem = guidNode.toElement();
        guid = guidElem.text();

        if (guidElem.attribute(QString::fromLatin1("isPermaLink")) == QString::fromLatin1("false"))
            guidIsPermaLink = false;
    }
    
    QDateTime pubDate;

    QString pubDateStr = Tools::extractElementText(e, QString::fromLatin1("pubDate"));
    if (!pubDateStr.isNull())
    {
        time_t time = KRFCDate::parseDate(pubDateStr);
        pubDate.setTime_t(time);
    }

    return Item(title, link, description, content, categories, comments, 
                author, enclosure, guid, guidIsPermaLink, pubDate, source);
}

Item::Item() : d(0)
{
}

Item::Item(const QString& title, const QString& link, const QString& description,
     const QString& content, const QList<Category>& categories,
     const QString& comments, const QString& author,
     const Enclosure& enclosure, const QString& guid, bool guidIsPermaLink,
     const QDateTime& pubDate, const Source& source) : d(new ItemPrivate)
{
    d->title = title;
    d->link = link;
    d->description = description;
    d->content = content;
    d->categories = categories;
    d->comments = comments;
    d->author = author;
    d->enclosure = enclosure;
    d->guid = guid;
    d->guidIsPermaLink = guidIsPermaLink;
    d->pubDate = pubDate;
    d->source = source;
}


Item::~Item()
{
}

Item::Item(const Item& other) : d(0)
{
    *this = other;
}

Item& Item::operator=(const Item& other)
{
    d = other.d;
    return *this;
}

bool Item::operator==(const Item& other) const
{
    if (!d || !other.d)
        return d == other.d;
    return *d == *other.d;
}

bool Item::isNull() const
{
    return !d;
}

QString Item::title() const
{
    return d ? d->title : QString::null;
}

QString Item::link() const
{
    return d ? d->link : QString::null;
}

QString Item::description() const
{
    return d ? d->description : QString::null;
}

QString Item::content() const
{
    return d ? d->content : QString::null;
}

QList<Category> Item::categories() const
{
    return d ? d->categories : QList<Category>();
}

QString Item::comments() const
{
    return d ? d->comments : QString::null;
}

QString Item::author() const
{
    return d ? d->author : QString::null;
}

Enclosure Item::enclosure() const
{
    return d ? d->enclosure : Enclosure::null();
}

QString Item::guid() const
{
    return d ? d->guid : QString::null;
}

bool Item::guidIsPermaLink() const
{
    return d ? d->guidIsPermaLink : false;
}

QDateTime Item::pubDate() const
{
    return d ? d->pubDate : QDateTime();
}

Source Item::source() const
{
    return d ? d->source : Source::null();
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
    for (QList<Category>::ConstIterator it = d->categories.begin(); it != d->categories.end(); ++it)
        info += (*it).debugInfo();
    info += "### Item end ################\n";
    return info;
}

} // namespace RSS2
} // namespace LibSyndication
