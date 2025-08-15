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

#include <KPluginFactory>
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
    Element(const QString &ns_, const QString &name_)
        : ns(ns_)
        , name(name_)
        , qualifiedName(ns + QLatin1Char(':') + name)
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
        Q_ASSERT(qv.canConvert<QString>());
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
    Elements()
        : atomNS(Syndication::Atom::atom1Namespace())
        , akregatorNS(akregatorNamespace())
        , commentNS(Syndication::commentApiNamespace())
        , title(atomNS, QStringLiteral("title"))
        , summary(atomNS, QStringLiteral("summary"))
        , content(atomNS, QStringLiteral("content"))
        , link(atomNS, QStringLiteral("link"))
        , language(atomNS, QStringLiteral("language"))
        , feed(atomNS, QStringLiteral("feed"))
        , guid(atomNS, QStringLiteral("id"))
        , published(atomNS, QStringLiteral("published"))
        , updated(atomNS, QStringLiteral("updated"))
        , commentsCount(Syndication::slashNamespace(), QStringLiteral("comments"))
        , commentsFeed(commentNS, QStringLiteral("commentRss"))
        , commentPostUri(commentNS, QStringLiteral("comment"))
        , commentsLink(akregatorNS, QStringLiteral("commentsLink"))
        , hash(akregatorNS, QStringLiteral("hash"))
        , guidIsHash(akregatorNS, QStringLiteral("idIsHash"))
        , name(atomNS, QStringLiteral("name"))
        , uri(atomNS, QStringLiteral("uri"))
        , email(atomNS, QStringLiteral("email"))
        , author(atomNS, QStringLiteral("author"))
        , category(atomNS, QStringLiteral("category"))
        , entry(atomNS, QStringLiteral("entry"))
        , itemProperties(akregatorNS, QStringLiteral("itemProperties"))
        , readStatus(akregatorNS, QStringLiteral("readStatus"))
        , deleted(akregatorNS, QStringLiteral("deleted"))
        , important(akregatorNS, QStringLiteral("important"))
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
    writer.writeNamespace(Syndication::commentApiNamespace(), QStringLiteral("comment"));
    writer.writeNamespace(akregatorNamespace(), QStringLiteral("akregator"));
    writer.writeNamespace(Syndication::itunesNamespace(), QStringLiteral("itunes"));

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
