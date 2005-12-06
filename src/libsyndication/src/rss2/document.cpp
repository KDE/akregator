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

#include "../documentvisitor.h"
#include "../shared.h"

#include "category.h"
#include "cloud.h"
#include "document.h"
#include "image.h"
#include "item.h"
#include "textinput.h"
#include "tools.h"

#include <QDateTime>
#include <QDomDocument>
#include <QDomElement>
#include <QList>
#include <QSet>
#include <QString>

#include <krfcdate.h>
#include <kstaticdeleter.h>

namespace LibSyndication {
namespace RSS2 {

class Document::DocumentPrivate : public LibSyndication::Shared
{
    public:

    bool isNull;
    QString title;
    QString description;
    QString link;
    QString language;
    QString copyright;
    QString managingEditor;
    QString webMaster;
    QDateTime pubDate;
    QDateTime lastBuildDate;
    QList<Category> categories;
    QString generator;
    QString docs;
    Cloud cloud;
    int ttl;
    Image image;
    TextInput textInput;
    QSet<int> skipHours;
    QSet<DayOfWeek> skipDays;
    QList<Item> items;

    bool operator==(const DocumentPrivate& other) const
    {
        return (isNull && other.isNull) || (
                title == other.title &&
                description == other.description &&
                link == other.link &&
                language == other.language &&
                copyright == other.copyright &&
                managingEditor == other.managingEditor &&
                webMaster == other.webMaster &&
                pubDate == other.pubDate &&
                lastBuildDate == other.lastBuildDate &&
                categories == other.categories &&
                generator == other.generator &&
                docs == other.docs &&
                cloud == other.cloud &&
                ttl == other.ttl &&
                image == other.image &&
                textInput == other.textInput &&
                skipHours == other.skipHours &&
                skipDays == other.skipDays &&
                items == other.items);
    }

    static DocumentPrivate* copyOnWrite(DocumentPrivate* ep)
    {
        if (ep->count > 1)
        {
            ep->deref();
            ep = new DocumentPrivate(*ep);
        }
        return ep;
    }
};

Document* Document::m_null = 0;
static KStaticDeleter<Document> documentsd;

const Document& Document::null()
{
    if (m_null == 0)
         documentsd.setObject(m_null, new Document);

    return *m_null;
}

Document Document::fromXML(const QDomDocument& doc)
{
    Document obj;

    QDomNode channelNode = doc.namedItem(QString::fromLatin1("rss")).namedItem(QString::fromLatin1("channel"));

    QDomElement e;

    if (channelNode.isElement())
        e = channelNode.toElement();
    else
    {
        //obj.setTitle("foo");
        return obj; // if there is no channel, we just return a null object
    }

    //QString text;

    obj.setTitle(Tools::extractElementText(e, QString::fromLatin1("title") ));
    obj.setDescription(Tools::extractElementText(e, QString::fromLatin1("description") ));
    obj.setLink(Tools::extractElementText(e, QString::fromLatin1("link") ));
    obj.setLanguage(Tools::extractElementText(e, QString::fromLatin1("language") ));
    obj.setCopyright(Tools::extractElementText(e, QString::fromLatin1("copyright") ));
    obj.setManagingEditor(Tools::extractElementText(e, QString::fromLatin1("managingEditor") ));
    obj.setWebMaster(Tools::extractElementText(e, QString::fromLatin1("webMaster") ));
    obj.setGenerator(Tools::extractElementText(e, QString::fromLatin1("generator") ));
    obj.setDocs(Tools::extractElementText(e, QString::fromLatin1("docs")));

    QList<QDomElement> items = Tools::elementsByTagName(e, QString::fromLatin1("item"));

    for (QList<QDomElement>::ConstIterator it = items.begin(); it != items.end(); ++it)
    {
        Item i = Item::fromXML(*it);
        if (!i.isNull())
            obj.addItem(i);
    }

    QList<QDomElement> categories = Tools::elementsByTagName(e, QString::fromLatin1("category"));
    for (QList<QDomElement>::ConstIterator it = categories.begin(); it != categories.end(); ++it)
    {
        Category i = Category::fromXML(*it);
        if (!i.isNull())
            obj.addCategory(i);
    }

    QDomNode textInput = e.namedItem(QString::fromLatin1("textInput"));
    if (textInput.isElement())
        obj.setTextInput(TextInput::fromXML(textInput.toElement()));

    QDomNode image = e.namedItem(QString::fromLatin1("image"));
    if (image.isElement())
        obj.setImage(Image::fromXML(image.toElement()));

    QDomNode cloud = e.namedItem(QString::fromLatin1("cloud"));
    if (cloud.isElement())
        obj.setCloud(Cloud::fromXML(cloud.toElement()));

    QDomNode skipHours = e.namedItem(QString::fromLatin1("skipHours"));
    if (skipHours.isElement())
    {
        bool ok = false;
        QList<QDomElement> hours = Tools::elementsByTagName(skipHours.toElement(), QString::fromLatin1("hour"));
        for (QList<QDomElement>::ConstIterator it = hours.begin(); it != hours.end(); ++it)
        {
            int h = (*it).text().toInt(&ok);
            if (ok)
                obj.addSkipHour(h);
        }
    }

    QString pubDateStr = Tools::extractElementText(e, QString::fromLatin1("pubDate"));
    if (!pubDateStr.isNull())
    {
        time_t time = KRFCDate::parseDate(pubDateStr);
        QDateTime pubDate;
        pubDate.setTime_t(time);
        obj.setPubDate(pubDate);
    }

    QString lastBuildDateStr = Tools::extractElementText(e, QString::fromLatin1("lastBuildDate"));
    if (!lastBuildDateStr.isNull())
    {
        time_t time = KRFCDate::parseDate(lastBuildDateStr);
        QDateTime lastBuildDate;
        lastBuildDate.setTime_t(time);
        obj.setLastBuildDate(lastBuildDate);
    }
    //TODO: pubdate, skipdays, lastBuildDate, rating, ttl
    return obj;
}

QDomElement Document::toXML(QDomDocument document) const
{

    QDomElement e = document.createElement(QString::fromLatin1("channel"));

    if (!d->title.isNull()) // required
    {
        QDomElement c = document.createElement(QString::fromLatin1("title"));
        c.appendChild(document.createTextNode(d->title));
        e.appendChild(c);
    }

    if (!d->description.isNull()) // required
    {
        QDomElement c = document.createElement(QString::fromLatin1("description"));
        c.appendChild(document.createTextNode(d->description));
        e.appendChild(c);
    }

    if (!d->link.isNull()) // required
    {
        QDomElement c = document.createElement(QString::fromLatin1("link"));
        c.appendChild(document.createTextNode(d->link));
        e.appendChild(c);
    }

    if (!d->language.isNull())
    {
        QDomElement c = document.createElement(QString::fromLatin1("language"));
        c.appendChild(document.createTextNode(d->language));
        e.appendChild(c);
    }

    if (!d->copyright.isNull())
    {
        QDomElement c = document.createElement(QString::fromLatin1("copyright"));
        c.appendChild(document.createTextNode(d->copyright));
        e.appendChild(c);
    }

    if (!d->managingEditor.isNull())
    {
        QDomElement c = document.createElement(QString::fromLatin1("managingEditor"));
        c.appendChild(document.createTextNode(d->managingEditor));
        e.appendChild(c);
    }

    if (!d->webMaster.isNull())
    {
        QDomElement c = document.createElement(QString::fromLatin1("webMaster"));
        c.appendChild(document.createTextNode(d->webMaster));
        e.appendChild(c);
    }

    //if (d->pubDate.isValid())
    //{
    //    QDomElement c = document.createElement(QString::fromLatin1("pubDate"));
    //    c.appendChild(document.createTextNode(d->pubDate)); //TODO: format as RFC 822-compliant date
    //    e.appendChild(c);
    //}

    //if (d->lastBuildDate.isValid())
    //{
    //    QDomElement c = document.createElement(QString::fromLatin1("pubDate"));
    //    c.appendChild(document.createTextNode(d->lastBuildDate)); //TODO: format as RFC 822-compliant date
    //    e.appendChild(c);
    //}

    if (d->ttl != -1)
    {
        QDomElement c = document.createElement(QString::fromLatin1("ttl"));
        c.appendChild(document.createTextNode(QString::number(d->ttl)));
        e.appendChild(c);
    }

    if (!d->generator.isNull())
    {
        QDomElement c = document.createElement(QString::fromLatin1("generator"));
        c.appendChild(document.createTextNode(d->generator));
        e.appendChild(c);
    }

    if (!d->docs.isNull())
    {
        QDomElement c = document.createElement(QString::fromLatin1("docs"));
        c.appendChild(document.createTextNode(d->docs));
        e.appendChild(c);
    }

    for (QList<Category>::ConstIterator it = d->categories.begin(); it != d->categories.end(); ++it)
        e.appendChild((*it).toXML(document));

    if (!d->cloud.isNull())
        e.appendChild(d->cloud.toXML(document));

    if (!d->image.isNull())
        e.appendChild(d->image.toXML(document));

    if (!d->textInput.isNull())
        e.appendChild(d->textInput.toXML(document));

    //TODO: rating, skipdays

    if (!d->skipHours.isEmpty())
    {
        QDomElement sh = document.createElement(QString::fromLatin1("skipHours"));
        e.appendChild(sh);
        for (QList<int>::ConstIterator it = d->skipHours.begin(); it != d->skipHours.end(); ++it)
        {
            QDomElement c = document.createElement(QString::fromLatin1("hour"));
            c.appendChild(document.createTextNode(QString::number(*it)));
            sh.appendChild(c);
        }
    }

    for (QList<Item>::ConstIterator it = d->items.begin(); it != d->items.end(); ++it)
        e.appendChild((*it).toXML(document));

    return e;
}

Document::Document() : d(new DocumentPrivate)
{
    d->isNull = true;
}

Document::~Document()
{
    if (d->deref())
    {
        delete d;
        d = 0;
    }
}

bool Document::isNull() const
{
    return d->isNull;
}

Document::Document(const Document& other)  : d(0)
{
    *this = other;
}

Document& Document::operator=(const Document& other)
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

bool Document::operator==(const Document& other) const
{
    return *d == *other.d;
}

void Document::setTitle(const QString& title)
{
    d = DocumentPrivate::copyOnWrite(d);
    d->isNull = false;
    d->title = title;
}

QString Document::title() const
{
    return !d->isNull ? d->title : QString::null;
}

void Document::setLink(const QString& link)
{
    d = DocumentPrivate::copyOnWrite(d);
    d->isNull = false;
    d->link = link;
}

QString Document::link() const
{
    return !d->isNull ? d->link : QString::null;
}

void Document::setDescription(const QString& description)
{
    d = DocumentPrivate::copyOnWrite(d);
    d->isNull = false;
    d->description = description;
}

QString Document::description() const
{
    return !d->isNull ? d->description : QString::null;
}

void Document::setLanguage(const QString& language)
{
    d = DocumentPrivate::copyOnWrite(d);
    d->isNull = false;
    d->language = language;
}

QString Document::language() const
{
    return !d->isNull ? d->language : QString::null;
}

void Document::setCopyright(const QString& copyright)
{
    d = DocumentPrivate::copyOnWrite(d);
    d->isNull = false;
    d->copyright = copyright;
}

QString Document::copyright() const
{
    return !d->isNull ? d->copyright : QString::null;
}

void Document::setManagingEditor(const QString& managingEditor)
{
    d = DocumentPrivate::copyOnWrite(d);
    d->isNull = false;
    d->managingEditor = managingEditor;
}

QString Document::managingEditor() const
{
    return !d->isNull ? d->managingEditor : QString::null;
}

void Document::setWebMaster(const QString& webMaster)
{
    d = DocumentPrivate::copyOnWrite(d);
    d->isNull = false;
    d->webMaster = webMaster;
}

QString Document::webMaster() const
{
    return !d->isNull ? d->webMaster : QString::null;
}


void Document::setPubDate(const QDateTime& pubDate)
{
    d = DocumentPrivate::copyOnWrite(d);
    d->isNull = false;
    d->pubDate = pubDate;
}

QDateTime Document::pubDate() const
{
    return !d->isNull ? d->pubDate : QDateTime();
}

void Document::setLastBuildDate(const QDateTime& lastBuildDate)
{
    d = DocumentPrivate::copyOnWrite(d);
    d->isNull = false;
    d->lastBuildDate = lastBuildDate;
}

QDateTime Document::lastBuildDate() const
{
    return !d->isNull ? d->lastBuildDate : QDateTime();
}

void Document::addCategory(const Category& category)
{
    d = DocumentPrivate::copyOnWrite(d);
    d->isNull = false;
    d->categories.append(category);
}

void Document::setCategories(const QList<Category>& categories)
{
    d = DocumentPrivate::copyOnWrite(d);
    d->isNull = false;
    d->categories = categories;
}

void Document::removeCategory(const Category& category)
{
    d = DocumentPrivate::copyOnWrite(d);
    d->isNull = false;
    d->categories.remove(category);
}

QList<Category> Document::categories() const
{
    return !d->isNull ? d->categories : QList<Category>();
}

void Document::setGenerator(const QString& generator)
{
    d = DocumentPrivate::copyOnWrite(d);
    d->isNull = false;
    d->generator = generator;
}

QString Document::generator() const
{
    return !d->isNull ? d->generator : QString::null;
}

void Document::setDocs(const QString& docs)
{
    d = DocumentPrivate::copyOnWrite(d);
    d->isNull = false;
    d->docs = docs;
}

QString Document::docs() const
{
    return !d->isNull ? d->docs : QString::null;
}

void Document::setCloud(const Cloud& cloud)
{
    d = DocumentPrivate::copyOnWrite(d);
    d->isNull = false;
    d->cloud = cloud;
}

Cloud Document::cloud() const
{
    return !d->isNull ? d->cloud : Cloud::null();
}

void Document::setTtl(int ttl)
{
    d = DocumentPrivate::copyOnWrite(d);
    d->isNull = false;
    d->ttl = ttl;
}

int Document::ttl() const
{
    return !d->isNull ? d->ttl : -1;
}

void Document::setImage(const Image& image)
{
    d = DocumentPrivate::copyOnWrite(d);
    d->isNull = false;
    d->image = image;
}

Image Document::image() const
{
    return !d->isNull ? d->image : Image::null();
}

void Document::setTextInput(const TextInput& textInput)
{
    d = DocumentPrivate::copyOnWrite(d);
    d->isNull = false;
    d->textInput = textInput;
}

TextInput Document::textInput() const
{
    return !d->isNull ? d->textInput : TextInput::null();
}

void Document::addSkipHour(int hour)
{
    d = DocumentPrivate::copyOnWrite(d);
    d->isNull = false;
    d->skipHours.append(hour);
}

void Document::setSkipHours(const QSet<int>& skipHours)
{
    d = DocumentPrivate::copyOnWrite(d);
    d->isNull = false;
    d->skipHours = skipHours;
}

void Document::removeSkipHour(int hour)
{
    d = DocumentPrivate::copyOnWrite(d);
    d->isNull = false;
    d->skipHours.remove(hour);
}

QSet<int> Document::skipHours() const
{
    return !d->isNull ? d->skipHours : QSet<int>();
}


void Document::addSkipDay(DayOfWeek day)
{
    d = DocumentPrivate::copyOnWrite(d);
    d->isNull = false;
    d->skipDays.append(day);
}

void Document::setSkipDays(const QSet<DayOfWeek>& skipDays)
{
    d = DocumentPrivate::copyOnWrite(d);
    d->isNull = false;
    d->skipDays = skipDays;
}

void Document::removeSkipDay(DayOfWeek day)
{
    d = DocumentPrivate::copyOnWrite(d);
    d->isNull = false;
    d->skipDays.remove(day);
}

QSet<DayOfWeek> Document::skipDays() const
{
    return !d->isNull ? d->skipDays : QSet<DayOfWeek>();
}

void Document::addItem(const Item& item)
{
    d = DocumentPrivate::copyOnWrite(d);
    d->isNull = false;
    d->items.append(item);
}

void Document::setItems(const QList<Item>& items)
{
    d = DocumentPrivate::copyOnWrite(d);
    d->isNull = false;
    d->items = items;
}

void Document::removeItem(const Item& item)
{
    d = DocumentPrivate::copyOnWrite(d);
    d->isNull = false;
    d->items.remove(item);
}

QList<Item> Document::items() const
{
    return !d->isNull ? d->items : QList<Item>();
}

QString Document::debugInfo() const
{
    QString info;
    info += "### Document: ###################\n";
    info += "title: #" + title() + "#\n";
    info += "description: #" + description() + "#\n";
    info += "link: #" + link() + "#\n";
    info += "language: #" + language() + "#\n";
    info += "copyright: #" + copyright() + "#\n";
    info += "managingEditor: #" + managingEditor() + "#\n";
    info += "webMaster: #" + webMaster() + "#\n";
    if (pubDate().isValid())
        info += "pubDate: #" + pubDate().toString() + "#\n";
    info += "lastBuildDate: #" + lastBuildDate().toString() + "#\n";
    if (!textInput().isNull())
        info += textInput().debugInfo();
    if (!cloud().isNull())
        info += cloud().debugInfo();
    if (!image().isNull())
        info += image().debugInfo();
    for (QList<Category>::ConstIterator it = d->categories.begin(); it != d->categories.end(); ++it)
        info += (*it).debugInfo();
    for (QList<Item>::ConstIterator it = d->items.begin(); it != d->items.end(); ++it)
        info += (*it).debugInfo();
    info += "### Document end ################\n";
    return info;
}

bool Document::accept(DocumentVisitor* visitor)
{
    visitor->visit(this);
}

} // namespace RSS2
} // namespace LibSyndication
