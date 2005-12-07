/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "category.h"
#include "enclosure.h"
#include "item.h"
#include "../shared.h"
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

class Item::ItemPrivate : public LibSyndication::Shared
{
    public:

    bool isNull;
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
        return (isNull && other.isNull) ||
                (title == other.title &&
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

    static ItemPrivate* copyOnWrite(ItemPrivate* ep)
    {
        if (ep->count > 1)
        {
            ep->deref();
            ep = new ItemPrivate(*ep);
        }
        return ep;
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
    Item obj;
    obj.setTitle(Tools::extractElementText(e, QString::fromLatin1("title") ));
    obj.setLink(Tools::extractElementText(e, QString::fromLatin1("link") ));
    obj.setDescription(Tools::extractElementText(e, QString::fromLatin1("description") ));
    // parse encoded stuff from content:encoded, xhtml:body and friends into content
    obj.setContent(Tools::extractContent(e));
    obj.setAuthor(Tools::extractElementText(e, QString::fromLatin1("author") ));
    obj.setComments(Tools::extractElementText(e, QString::fromLatin1("comments") ));

    QDomNode enclosure = e.namedItem(QString::fromLatin1("enclosure"));
    if (enclosure.isElement())
        obj.setEnclosure(Enclosure::fromXML(enclosure.toElement()));

    QDomNode source = e.namedItem(QString::fromLatin1("source"));
    if (source.isElement())
        obj.setSource(Source::fromXML(source.toElement()));

    QList<QDomElement> categories = Tools::elementsByTagName(e, QString::fromLatin1("category"));
    for (QList<QDomElement>::ConstIterator it = categories.begin(); it != categories.end(); ++it)
    {
        Category i = Category::fromXML(*it);
        if (!i.isNull())
            obj.addCategory(i);
    }

    QDomNode guid = e.namedItem(QString::fromLatin1("guid"));
    if (guid.isElement())
    {
        QDomElement guidElem = guid.toElement();
        obj.setGuid(guidElem.text());

        if (guidElem.attribute(QString::fromLatin1("isPermaLink")) == QString::fromLatin1("false"))
            obj.setGuidIsPermaLink(false);
        else
            obj.setGuidIsPermaLink(true); // true is default
    }
    //TODO: pubdate

    QString pubDateStr = Tools::extractElementText(e, QString::fromLatin1("pubDate"));
    if (!pubDateStr.isNull())
    {
        time_t time = KRFCDate::parseDate(pubDateStr);
        QDateTime pubDate;
        pubDate.setTime_t(time);
        obj.setPubDate(pubDate);
    }
    return obj;
}

Item::Item() : d(new ItemPrivate)
{
    d->isNull = true;
    d->guidIsPermaLink = true;
}

Item::~Item()
{
    if (d->deref())
    {
        delete d;
        d = 0;
    }
}

Item::Item(const Item& other) : d(0)
{
    *this = other;
}

Item& Item::operator=(const Item& other)
{
    if (d != other.d)
    {
        other.d->ref();
        if (d && d->deref())
            delete d;
        d = other.d;
    }
    return *this;
}

bool Item::operator==(const Item& other) const
{
    return *d == *other.d;
}

bool Item::isNull() const
{
    return d->isNull;
}

void Item::setTitle(const QString& title)
{
    d = ItemPrivate::copyOnWrite(d);
    d->isNull = false;
    d->title = title;
}

QString Item::title() const
{
    return !d->isNull ? d->title : QString::null;
}

void Item::setLink(const QString& link)
{
    d = ItemPrivate::copyOnWrite(d);
    d->isNull = false;
    d->link = link;
}

QString Item::link() const
{
    return !d->isNull ? d->link : QString::null;
}

void Item::setDescription(const QString& description)
{
    d = ItemPrivate::copyOnWrite(d);
    d->isNull = false;
    d->description = description;
}

QString Item::description() const
{
    return !d->isNull ? d->description : QString::null;
}

void Item::setContent(const QString& content)
{
    d = ItemPrivate::copyOnWrite(d);
    d->isNull = false;
    d->content = content;
}

QString Item::content() const
{
    return !d->isNull ? d->content : QString::null;
}

void Item::addCategory(const Category& category)
{
    d = ItemPrivate::copyOnWrite(d);
    d->isNull = false;
    d->categories.append(category);
}

void Item::setCategories(const QList<Category>& categories)
{
    d = ItemPrivate::copyOnWrite(d);
    d->isNull = false;
    d->categories = categories;
}

void Item::removeCategory(const Category& category)
{
    d = ItemPrivate::copyOnWrite(d);
    d->isNull = false;
    d->categories.remove(category);
}

 QList<Category> Item::categories() const
{
    return !d->isNull ? d->categories : QList<Category>();
}

void Item::setComments(const QString& comments)
{
    d = ItemPrivate::copyOnWrite(d);
    d->isNull = false;
    d->comments = comments;
}

QString Item::comments() const
{
    return !d->isNull ? d->comments : QString::null;
}

void Item::setAuthor(const QString& author)
{
    d = ItemPrivate::copyOnWrite(d);
    d->isNull = false;
    d->author = author;
}

QString Item::author() const
{
    return !d->isNull ? d->author : QString::null;
}


void Item::setEnclosure(const Enclosure& enclosure)
{
    d = ItemPrivate::copyOnWrite(d);
    d->isNull = false;
    d->enclosure = enclosure;
}

Enclosure Item::enclosure() const
{
    return !d->isNull ? d->enclosure : Enclosure::null();
}

void Item::setGuid(const QString& guid)
{
    d = ItemPrivate::copyOnWrite(d);
    d->isNull = false;
    d->guid = guid;
}

QString Item::guid() const
{
    return !d->isNull ? d->guid : QString::null;
}

void Item::setGuidIsPermaLink(bool isPermaLink)
{
    d = ItemPrivate::copyOnWrite(d);
    d->isNull = false;
    d->guidIsPermaLink = isPermaLink;
}

bool Item::guidIsPermaLink() const
{
    return !d->isNull ? d->guidIsPermaLink : false;
}

void Item::setPubDate(const QDateTime& pubDate)
{
    d = ItemPrivate::copyOnWrite(d);
    d->isNull = false;
    d->pubDate = pubDate;
}

QDateTime Item::pubDate() const
{
    return !d->isNull ? d->pubDate : QDateTime();
}

void Item::setSource(const Source& source)
{
    d = ItemPrivate::copyOnWrite(d);
    d->isNull = false;
    d->source = source;
}

Source Item::source() const
{
    return !d->isNull ? d->source : Source::null();
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
