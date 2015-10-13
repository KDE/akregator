/*
 * This file is part of akregatorstorageexporter
 *
 * Copyright (C) 2009 Frank Osterfeld <osterfeld@kde.org>
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
#include "feedstorage.h"
#include "storage.h"
#include "storagefactory.h"
#include "storagefactoryregistry.h"
#include "plugin.h"

#include <Syndication/Constants>
#include <Syndication/Atom/Atom>

#include <QDateTime>
#include <QFile>
#include <QIODevice>
#include <QStringList>
#include <QXmlStreamWriter>
#include <QVariant>
#include <QDebug>

#include <KPluginLoader>
#include <KService>
#include <KServiceTypeTrader>
#include <QUrl>

#include <iostream>

#include <cassert>

using namespace Akregator;
using namespace Akregator::Backend;

namespace
{
static QString akregatorNamespace()
{
    return QStringLiteral("http://akregator.kde.org/StorageExporter#");
}

enum TextMode {
    PlainText,
    Html
};

enum Status {
    Deleted = 0x01,
    Trash = 0x02,
    New = 0x04,
    Read = 0x08,
    Keep = 0x10
};

class Element
{
public:
    Element(const QString &ns_, const QString &name_) : ns(ns_), name(name_), qualifiedName(ns + QLatin1Char(':') + name)
    {
    }

    const QString ns;
    const QString name;
    const QString qualifiedName;

    void writeStartElement(QXmlStreamWriter &writer) const
    {
        if (!ns.isNull()) {
            writer.writeStartElement(ns, name);
        } else {
            writer.writeStartElement(name);
        }
    }

    void write(const QVariant &value, QXmlStreamWriter &writer, TextMode mode = PlainText) const
    {
        const QVariant qv(value);
        Q_ASSERT(qv.canConvert(QVariant::String));
        const QString str = qv.toString();
        if (str.isEmpty()) {
            return;
        }

        if (ns.isEmpty()) {
            writer.writeStartElement(name);
        } else {
            writer.writeStartElement(ns, name);
        }
        if (mode == Html) {
            writer.writeAttribute(QStringLiteral("type"), QStringLiteral("html"));
        }
        writer.writeCharacters(str);
        writer.writeEndElement();
    }
};

struct Elements {
    Elements() : atomNS(Syndication::Atom::atom1Namespace()),
        akregatorNS(akregatorNamespace()),
        commentNS(Syndication::commentApiNamespace()),
        title(atomNS, QStringLiteral("title")),
        summary(atomNS, QStringLiteral("summary")),
        content(atomNS, QStringLiteral("content")),
        link(atomNS, QStringLiteral("link")),
        language(atomNS, QStringLiteral("language")),
        feed(atomNS, QStringLiteral("feed")),
        guid(atomNS, QStringLiteral("id")),
        published(atomNS, QStringLiteral("published")),
        updated(atomNS, QStringLiteral("updated")),
        commentsCount(Syndication::slashNamespace(), QStringLiteral("comments")),
        commentsFeed(commentNS, QStringLiteral("commentRss")),
        commentPostUri(commentNS, QStringLiteral("comment")),
        commentsLink(akregatorNS, QStringLiteral("commentsLink")),
        hash(akregatorNS, QStringLiteral("hash")),
        guidIsHash(akregatorNS, QStringLiteral("idIsHash")),
        name(atomNS, QStringLiteral("name")),
        uri(atomNS, QStringLiteral("uri")),
        email(atomNS, QStringLiteral("email")),
        author(atomNS, QStringLiteral("author")),
        category(atomNS, QStringLiteral("category")),
        entry(atomNS, QStringLiteral("entry")),
        itemProperties(akregatorNS, QStringLiteral("itemProperties")),
        readStatus(akregatorNS, QStringLiteral("readStatus")),
        deleted(akregatorNS, QStringLiteral("deleted")),
        important(akregatorNS, QStringLiteral("important"))

    {}
    const QString atomNS;
    const QString akregatorNS;
    const QString commentNS;
    const Element title;
    const Element summary;
    const Element content;
    const Element link;
    const Element language;
    const Element feed;
    const Element guid;
    const Element published;
    const Element updated;
    const Element commentsCount;
    const Element commentsFeed;
    const Element commentPostUri;
    const Element commentsLink;
    const Element hash;
    const Element guidIsHash;
    const Element name;
    const Element uri;
    const Element email;
    const Element author;
    const Element category;
    const Element entry;
    const Element itemProperties;
    const Element readStatus;
    const Element deleted;
    const Element important;
    static const Elements instance;
};

const Elements Elements::instance;

void writeAttributeIfNotEmpty(const QString &element, const QVariant &value, QXmlStreamWriter &writer)
{
    const QString text = value.toString();
    if (text.isEmpty()) {
        return;
    }
    writer.writeAttribute(element, text);
}

void writeEnclosure(const QString &url, const QString &type, int length, QXmlStreamWriter &writer)
{
    Elements::instance.link.writeStartElement(writer);
    writer.writeAttribute(QStringLiteral("rel"), QStringLiteral("enclosure"));
    writeAttributeIfNotEmpty(QStringLiteral("href"), url, writer);
    writeAttributeIfNotEmpty(QStringLiteral("type"), type, writer);
    if (length > 0) {
        writer.writeAttribute(QStringLiteral("length"), QString::number(length));
    }
    writer.writeEndElement();
}

void writeLink(const QString &url, QXmlStreamWriter &writer)
{
    if (url.isEmpty()) {
        return;
    }
    Elements::instance.link.writeStartElement(writer);
    writer.writeAttribute(QStringLiteral("rel"), QStringLiteral("alternate"));
    writeAttributeIfNotEmpty(QStringLiteral("href"), url, writer);
    writer.writeEndElement();
}

void writeAuthor(const QString &name, const QString &uri, const QString &email, QXmlStreamWriter &writer)
{
    if (name.isEmpty() && uri.isEmpty() && email.isEmpty()) {
        return;
    }

    const QString atomNS = Syndication::Atom::atom1Namespace();
    Elements::instance.author.writeStartElement(writer);
    Elements::instance.name.write(name, writer);
    Elements::instance.uri.write(uri, writer);
    Elements::instance.email.write(email, writer);
    writer.writeEndElement(); // </author>
}

static void writeItem(FeedStorage *storage, const QString &guid, QXmlStreamWriter &writer)
{
    Elements::instance.entry.writeStartElement(writer);
    Elements::instance.guid.write(guid, writer);

    const uint published = storage->pubDate(guid);
    if (published > 0) {
        const QString pdStr = QDateTime::fromTime_t(published).toString(Qt::ISODate);
        Elements::instance.published.write(pdStr, writer);
    }

    const int status = storage->status(guid);

    Elements::instance.itemProperties.writeStartElement(writer);

    if (status & Deleted) {
        Elements::instance.deleted.write(QStringLiteral("true"), writer);
        writer.writeEndElement(); // </itemProperties>
        writer.writeEndElement(); // </item>
        return;
    }

    Elements::instance.hash.write(QString::number(storage->hash(guid)), writer);
    if (storage->guidIsHash(guid)) {
        Elements::instance.guidIsHash.write(QStringLiteral("true"), writer);
    }
    if (status & New) {
        Elements::instance.readStatus.write(QStringLiteral("new"), writer);
    } else if ((status & Read) == 0) {
        Elements::instance.readStatus.write(QStringLiteral("unread"), writer);
    }
    if (status & Keep) {
        Elements::instance.important.write(QStringLiteral("true"), writer);
    }
    writer.writeEndElement(); // </itemProperties>

    Elements::instance.title.write(storage->title(guid), writer, Html);
    writeLink(storage->guidIsPermaLink(guid) ? guid :  storage->link(guid), writer);

    Elements::instance.summary.write(storage->description(guid), writer, Html);
    Elements::instance.content.write(storage->content(guid), writer, Html);
    writeAuthor(storage->authorName(guid),
                storage->authorUri(guid),
                storage->authorEMail(guid),
                writer);

    if (const int commentsCount = storage->comments(guid)) {
        Elements::instance.commentsCount.write(QString::number(commentsCount), writer);
    }

    Elements::instance.commentsLink.write(storage->commentsLink(guid), writer);

    bool hasEnc = false;
    QString encUrl, encType;
    int encLength = 0;
    storage->enclosure(guid, hasEnc, encUrl, encType, encLength);
    if (hasEnc) {
        writeEnclosure(encUrl, encType, encLength, writer);
    }
    writer.writeEndElement(); // </item>
}

static void serialize(FeedStorage *storage, const QString &url, QIODevice *device)
{
    Q_ASSERT(storage);
    Q_ASSERT(device);
    QXmlStreamWriter writer(device);
    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(2);
    writer.writeStartDocument();

    Elements::instance.feed.writeStartElement(writer);

    writer.writeDefaultNamespace(Syndication::Atom::atom1Namespace());
    writer.writeNamespace(Syndication::commentApiNamespace(), QStringLiteral("comment"));
    writer.writeNamespace(akregatorNamespace(), QStringLiteral("akregator"));
    writer.writeNamespace(Syndication::itunesNamespace(), QStringLiteral("itunes"));

    Elements::instance.title.write(QStringLiteral("Akregator Export for %1").arg(url), writer, Html);

    Q_FOREACH (const QString &i, storage->articles()) {
        writeItem(storage, i, writer);
    }
    writer.writeEndElement(); // </feed>
    writer.writeEndDocument();
}

static void serialize(Storage *storage, const QString &url, QIODevice *device)
{
    serialize(storage->archiveFor(url), url, device);
}

static KService::List queryStoragePlugins()
{
    return KServiceTypeTrader::self()->query(QStringLiteral("Akregator/Plugin"),
            QStringLiteral("[X-KDE-akregator-framework-version] == %1 and [X-KDE-akregator-plugintype] == 'storage' and [X-KDE-akregator-rank] > 0").arg(QString::number(AKREGATOR_PLUGIN_INTERFACE_VERSION)));
}

static Plugin *createFromService(const KService::Ptr &service)
{
    KPluginLoader loader(*service);
    KPluginFactory *factory = loader.factory();
    if (!factory) {
        qCritical() << QStringLiteral(" Could not create plugin factory for: %1\n"
                                      " Error message: %2").arg(service->library(), loader.errorString());
        return 0;
    }
    return factory->create<Akregator::Plugin>();
}

static void printUsage()
{
    std::cout << "akregatorstorageexporter [--base64] url" << std::endl;
}
}

int main(int argc, char **argv)
{
    const QString backend = QStringLiteral("metakit");

    if (argc < 2) {
        printUsage();
        return 1;
    }

    const bool base64 = qstrcmp(argv[1], "--base64") == 0;

    if (base64 && argc < 3) {
        printUsage();
        return 1;
    }

    const int pos = base64 ? 2 : 1;
    const QString url = QUrl::fromEncoded(base64 ? QByteArray::fromBase64(argv[pos]) : QByteArray(argv[pos])).toString();

    Q_FOREACH (const KService::Ptr &i, queryStoragePlugins())
        if (Plugin *const plugin = createFromService(i)) {
            plugin->initialize();
        }

    const StorageFactory *const storageFactory = StorageFactoryRegistry::self()->getFactory(backend);
    if (!storageFactory) {
        qCritical("Could not create storage factory for %s.", qPrintable(backend));
        return 1;
    }

    Storage *const storage = storageFactory->createStorage(QStringList());
    if (!storage) {
        qCritical("Could not create storage object for %s.", qPrintable(backend));
        return 1;
    }

    QFile out;
    if (!out.open(stdout, QIODevice::WriteOnly)) {
        qCritical("Could not open stdout for writing: %s", qPrintable(out.errorString()));
        return 1;
    }

    serialize(storage, url, &out);

    return 0;
}
