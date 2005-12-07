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

class Document::DocumentPrivate : public KShared
{
    public:

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
        return (
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
};

Document* Document::m_null = 0;
static KStaticDeleter<Document> documentsd;

const Document& Document::null()
{
    if (m_null == 0)
         documentsd.setObject(m_null, new Document);

    return *m_null;
}

Document::Document(const QString& title, const QString& link,
                 const QString& description, const QString& language,
                 const QString& copyright, const QString& managingEditor,
                 const QString& webMaster, const QDateTime& pubDate,
                 const QDateTime& lastBuildDate,
                 const QList<Category>& categories,
                 const QString& generator, const QString& docs,
                 const Cloud& cloud, int ttl, const Image& image, 
                 const TextInput& textInput, const QSet<int>& skipHours,
                 const QSet<DayOfWeek>& skipDays, const QList<Item>& items) : d(new DocumentPrivate)
{
    d->title = title;
    d->link = link;
    d->description = description;
    d->language = language;
    d->copyright = copyright;
    d->managingEditor = managingEditor;
    d->webMaster = webMaster;
    d->pubDate = pubDate;
    d->lastBuildDate = lastBuildDate;
    d->categories = categories;
    d->generator = generator;
    d->docs = docs;
    d->cloud = cloud;
    d->ttl = ttl;
    d->image = image;
    d->textInput = textInput;
    d->skipHours = skipHours;
    d->skipDays = skipDays;
    d->items = items;
}

Document Document::fromXML(const QDomDocument& doc)
{
    QDomNode channelNode = doc.namedItem(QString::fromLatin1("rss")).namedItem(QString::fromLatin1("channel"));

    QDomElement e;

    if (channelNode.isElement())
        e = channelNode.toElement();
    else
    {
        return Document(); // if there is no channel, we just return a null object
    }

    QString title = Tools::extractElementText(e, QString::fromLatin1("title"));
    QString description = Tools::extractElementText(e, QString::fromLatin1("description") );
    QString link = Tools::extractElementText(e, QString::fromLatin1("link") );
    QString language = Tools::extractElementText(e, QString::fromLatin1("language") );
    QString copyright = Tools::extractElementText(e, QString::fromLatin1("copyright") );
    QString managingEditor = Tools::extractElementText(e, QString::fromLatin1("managingEditor") );
    QString webMaster = Tools::extractElementText(e, QString::fromLatin1("webMaster") );
    QString generator = Tools::extractElementText(e, QString::fromLatin1("generator") );
    QString docs = Tools::extractElementText(e, QString::fromLatin1("docs"));

    QList<QDomElement> itemNodes = Tools::elementsByTagName(e, QString::fromLatin1("item"));

    QList<Item> items;

    for (QList<QDomElement>::ConstIterator it = itemNodes.begin(); it != itemNodes.end(); ++it)
    {
        Item i = Item::fromXML(*it);
        if (!i.isNull())
            items.append(i);
    }

    QList<Category> categories;

    QList<QDomElement> catNodes = Tools::elementsByTagName(e, QString::fromLatin1("category"));
    for (QList<QDomElement>::ConstIterator it = catNodes.begin(); it != catNodes.end(); ++it)
    {
        Category i = Category::fromXML(*it);
        if (!i.isNull())
            categories.append(i);
    }

    TextInput textInput;

    QDomNode textInputNode = e.namedItem(QString::fromLatin1("textInput"));
    if (textInputNode.isElement())
        textInput = TextInput::fromXML(textInputNode.toElement());

    Image image;
    QDomNode imageNode = e.namedItem(QString::fromLatin1("image"));
    if (imageNode.isElement())
        image = Image::fromXML(imageNode.toElement());

    Cloud cloud;
    QDomNode cloudNode = e.namedItem(QString::fromLatin1("cloud"));
    if (cloudNode.isElement())
        cloud = Cloud::fromXML(cloudNode.toElement());

    QSet<int> skipHours;
    QDomNode skipHoursNode = e.namedItem(QString::fromLatin1("skipHours"));
    if (skipHoursNode.isElement())
    {
        bool ok = false;
        QList<QDomElement> hours = Tools::elementsByTagName(skipHoursNode.toElement(), QString::fromLatin1("hour"));
        for (QList<QDomElement>::ConstIterator it = hours.begin(); it != hours.end(); ++it)
        {
            int h = (*it).text().toInt(&ok);
            if (ok)
                skipHours.insert(h);
        }
    }

    QDateTime pubDate;

    QString pubDateStr = Tools::extractElementText(e, QString::fromLatin1("pubDate"));
    if (!pubDateStr.isNull())
    {
        time_t time = KRFCDate::parseDate(pubDateStr);
        pubDate.setTime_t(time);
    }

    QDateTime lastBuildDate;

    QString lastBuildDateStr = Tools::extractElementText(e, QString::fromLatin1("lastBuildDate"));
    if (!lastBuildDateStr.isNull())
    {
        time_t time = KRFCDate::parseDate(lastBuildDateStr);
        lastBuildDate.setTime_t(time);
     }
    //TODO: skipdays, rating, ttl
    int ttl = 0;
    QSet<DayOfWeek> skipDays;

    return Document(title, link, description, language, copyright, managingEditor, webMaster, pubDate, lastBuildDate, categories, generator, docs, cloud, ttl, image, textInput, skipHours, skipDays, items);
}

Document::Document() : d(0)
{
}

Document::~Document()
{
}

bool Document::isNull() const
{
    return !d;
}

Document::Document(const Document& other)  : LibSyndication::Document(other), d(0)
{
    *this = other;
}

Document& Document::operator=(const Document& other)
{
    d = other.d;
    return *this;
}

bool Document::operator==(const Document& other) const
{
    if (!d || !other.d)
        return d == other.d;
    return *d == *other.d;
}

QString Document::title() const
{
    return d ? d->title : QString::null;
}

QString Document::link() const
{
    return d ? d->link : QString::null;
}

QString Document::description() const
{
    return d ? d->description : QString::null;
}

QString Document::language() const
{
    return d ? d->language : QString::null;
}

QString Document::copyright() const
{
    return d ? d->copyright : QString::null;
}

QString Document::managingEditor() const
{
    return d ? d->managingEditor : QString::null;
}

QString Document::webMaster() const
{
    return d ? d->webMaster : QString::null;
}

QDateTime Document::pubDate() const
{
    return d ? d->pubDate : QDateTime();
}

QDateTime Document::lastBuildDate() const
{
    return d ? d->lastBuildDate : QDateTime();
}

QList<Category> Document::categories() const
{
    return d ? d->categories : QList<Category>();
}

QString Document::generator() const
{
    return d ? d->generator : QString::null;
}

QString Document::docs() const
{
    return d ? d->docs : QString::null;
}

Cloud Document::cloud() const
{
    return d ? d->cloud : Cloud::null();
}

int Document::ttl() const
{
    return d ? d->ttl : -1;
}

Image Document::image() const
{
    return d ? d->image : Image::null();
}

TextInput Document::textInput() const
{
    return d ? d->textInput : TextInput::null();
}

QSet<int> Document::skipHours() const
{
    return d ? d->skipHours : QSet<int>();
}

QSet<Document::DayOfWeek> Document::skipDays() const
{
    return d ? d->skipDays : QSet<DayOfWeek>();
}

QList<Item> Document::items() const
{
    return d ? d->items : QList<Item>();
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
    return visitor->visit(this);
}

} // namespace RSS2
} // namespace LibSyndication
