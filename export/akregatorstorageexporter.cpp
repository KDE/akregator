/*
 * This file is part of akregatorstorageexporter
 *
 * SPDX-FileCopyrightText: 2009 Frank Osterfeld <osterfeld@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 */
#include "storage/feedstorage.h"
#include "storage/storage.h"
#include <KLocalizedString>
#include <Syndication/Atom/Atom>
#include <Syndication/Constants>

#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QIODevice>
#include <QVariant>
#include <QXmlStreamWriter>

#include <QDebug>
#include <QUrl>

#include <iostream>
#include <utility>

#include <KPluginFactory>
#include <cassert>

using namespace Qt::Literals::StringLiterals;

using namespace Akregator;
using namespace Akregator::Backend;

namespace
{
static QString akregatorNamespace()
{
    return u"http://akregator.kde.org/StorageExporter#"_s;
}

enum TextMode {
    PlainText,
    Html,
};

enum Status {
    Deleted = 0x01,
    Trash = 0x02,
    New = 0x04,
    Read = 0x08,
    Keep = 0x10,
};

class Element
{
public:
    Element(QString ns_, QString name_)
        : ns(std::move(ns_))
        , name(std::move(name_))
        , qualifiedName(ns + u':' + name)
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
        Q_ASSERT(value.canConvert<QString>());
        const QString str = value.toString();
        if (str.isEmpty()) {
            return;
        }

        if (ns.isEmpty()) {
            writer.writeStartElement(name);
        } else {
            writer.writeStartElement(ns, name);
        }
        if (mode == Html) {
            writer.writeAttribute(u"type"_s, u"html"_s);
        }
        writer.writeCharacters(str);
        writer.writeEndElement();
    }
};

struct Elements {
    Elements()
        : atomNS(Syndication::Atom::atom1Namespace())
        , akregatorNS(akregatorNamespace())
        , commentNS(Syndication::commentApiNamespace())
        , title(atomNS, u"title"_s)
        , summary(atomNS, u"summary"_s)
        , content(atomNS, u"content"_s)
        , link(atomNS, u"link"_s)
        , language(atomNS, u"language"_s)
        , feed(atomNS, u"feed"_s)
        , guid(atomNS, u"id"_s)
        , published(atomNS, u"published"_s)
        , updated(atomNS, u"updated"_s)
        , commentsCount(Syndication::slashNamespace(), u"comments"_s)
        , commentsFeed(commentNS, u"commentRss"_s)
        , commentPostUri(commentNS, u"comment"_s)
        , commentsLink(akregatorNS, u"commentsLink"_s)
        , hash(akregatorNS, u"hash"_s)
        , guidIsHash(akregatorNS, u"idIsHash"_s)
        , name(atomNS, u"name"_s)
        , uri(atomNS, u"uri"_s)
        , email(atomNS, u"email"_s)
        , author(atomNS, u"author"_s)
        , category(atomNS, u"category"_s)
        , entry(atomNS, u"entry"_s)
        , itemProperties(akregatorNS, u"itemProperties"_s)
        , readStatus(akregatorNS, u"readStatus"_s)
        , deleted(akregatorNS, u"deleted"_s)
        , important(akregatorNS, u"important"_s)
    {
    }

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
    writer.writeAttribute(u"rel"_s, u"enclosure"_s);
    writeAttributeIfNotEmpty(u"href"_s, url, writer);
    writeAttributeIfNotEmpty(u"type"_s, type, writer);
    if (length > 0) {
        writer.writeAttribute(u"length"_s, QString::number(length));
    }
    writer.writeEndElement();
}

void writeLink(const QString &url, QXmlStreamWriter &writer)
{
    if (url.isEmpty()) {
        return;
    }
    Elements::instance.link.writeStartElement(writer);
    writer.writeAttribute(u"rel"_s, u"alternate"_s);
    writeAttributeIfNotEmpty(u"href"_s, url, writer);
    writer.writeEndElement();
}

void writeAuthor(const QString &name, const QString &uri, const QString &email, QXmlStreamWriter &writer)
{
    if (name.isEmpty() && uri.isEmpty() && email.isEmpty()) {
        return;
    }

    // const QString atomNS = Syndication::Atom::atom1Namespace();
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

    const QDateTime published = storage->pubDate(guid);
    if (published.isValid()) {
        const QString pdStr = published.toString(Qt::ISODate);
        Elements::instance.published.write(pdStr, writer);
    }

    const int status = storage->status(guid);

    Elements::instance.itemProperties.writeStartElement(writer);

    if (status & Deleted) {
        Elements::instance.deleted.write(u"true"_s, writer);
        writer.writeEndElement(); // </itemProperties>
        writer.writeEndElement(); // </item>
        return;
    }

    Elements::instance.hash.write(QString::number(storage->hash(guid)), writer);
    if (storage->guidIsHash(guid)) {
        Elements::instance.guidIsHash.write(u"true"_s, writer);
    }
    if (status & New) {
        Elements::instance.readStatus.write(u"new"_s, writer);
    } else if ((status & Read) == 0) {
        Elements::instance.readStatus.write(u"unread"_s, writer);
    }
    if (status & Keep) {
        Elements::instance.important.write(u"true"_s, writer);
    }
    writer.writeEndElement(); // </itemProperties>

    Elements::instance.title.write(storage->title(guid), writer, Html);
    writeLink(storage->guidIsPermaLink(guid) ? guid : storage->link(guid), writer);

    Elements::instance.summary.write(storage->description(guid), writer, Html);
    Elements::instance.content.write(storage->content(guid), writer, Html);
    writeAuthor(storage->authorName(guid), storage->authorUri(guid), storage->authorEMail(guid), writer);

    bool hasEnc = false;
    QString encUrl;
    QString encType;
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
    writer.writeNamespace(Syndication::commentApiNamespace(), u"comment"_s);
    writer.writeNamespace(akregatorNamespace(), u"akregator"_s);
    writer.writeNamespace(Syndication::itunesNamespace(), u"itunes"_s);

    Elements::instance.title.write(i18n("Akregator Export for %1", url), writer, Html);

    const auto articles = storage->articles();
    for (const QString &i : articles) {
        writeItem(storage, i, writer);
    }
    writer.writeEndElement(); // </feed>
    writer.writeEndDocument();
}

static void serialize(Storage *storage, const QString &url, QIODevice *device)
{
    serialize(storage->archiveFor(url), url, device);
}

static void printUsage()
{
    std::cout << "akregatorstorageexporter [--base64] url" << std::endl;
}
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

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

    Storage storage;

    QFile out;
    if (!out.open(stdout, QIODevice::WriteOnly)) {
        qCritical() << "Could not open stdout for writing: " << qPrintable(out.errorString());
        return 1;
    }

    serialize(&storage, url, &out);

    return 0;
}
