/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "article.h"
#include "feed.h"
#include "shared.h"
#include "storage/feedstorage.h"
#include "storage/storage.h"
#include "utils.h"

#include <Syndication/Syndication>

#include <QDateTime>
#include <QList>
#include <QRegularExpression>
#include <qdom.h>

#include <QUrl>
#include <cassert>

using namespace Syndication;

namespace
{
QString buildTitle(const QString &description)
{
    QString s = description;
    if (description.trimmed().isEmpty()) {
        return {};
    }

    int i = s.indexOf(QLatin1Char('>'), 500); /*avoid processing too much */
    if (i != -1) {
        s = s.left(i + 1);
    }
    const QRegularExpression rx(QStringLiteral("(<([^\\s>]*)(?:[^>]*)>)[^<]*"));
    int offset = 0;
    QRegularExpressionMatch rmatch;
    // We get the opening tag (e.g. <i>) in one iteration and then the closing
    // tag (e.g. </i>), in the next one. Note that <br> doesn't have a closing tag
    while (s.indexOf(rx, offset, &rmatch) != -1) {
        const QString tagName = rmatch.captured(2);
        QString toReplace;
        QString replaceWith;
        int repStart = 0;
        if (tagName.compare(QLatin1StringView("script"), Qt::CaseInsensitive) == 0) {
            // E.g.: <script foo="bar">some js here</script>
            // strip tag AND tag contents
            toReplace = rmatch.captured(0);
            repStart = rmatch.capturedStart(0);
        } else if (tagName.startsWith(QLatin1StringView("br"), Qt::CaseInsensitive)) {
            toReplace = rmatch.captured(1);
            repStart = rmatch.capturedStart(1);
            replaceWith = QLatin1Char(' ');
        } else {
            // Any other tag, <i>text</i> ... etc
            toReplace = rmatch.captured(1); // strip just tag
            repStart = rmatch.capturedStart(1);
        }
        s.replace(repStart, toReplace.length(), replaceWith); // do the deed
        offset = repStart + replaceWith.length();
    }
    if (s.length() > 90) {
        s = s.left(90) + QLatin1StringView("…");
    }
    return s.simplified();
}
}

namespace Akregator
{
struct Article::Private : public Shared {
    Private();
    Private(const QString &guid, Feed *feed, Backend::FeedStorage *archive);
    Private(const ItemPtr &article, Feed *feed, Backend::FeedStorage *archive);

    /** The status of the article is stored in an int, the bits having the
        following meaning:

        0000 0001 Deleted
        0000 0010 Trash
        0000 0100 New
        0000 1000 Read
        0001 0000 Keep
     */
    enum Status { Deleted = 0x01, Trash = 0x02, New = 0x04, Read = 0x08, Keep = 0x10 };

    Feed *feed = nullptr;
    QString guid;
    Backend::FeedStorage *archive = nullptr;
    int status;
    uint hash;
    QDateTime pubDate;
    QString title; // Cache the title, for performance
    mutable QSharedPointer<const Enclosure> enclosure;
};

namespace
{
class EnclosureImpl : public Enclosure
{
public:
    EnclosureImpl(const QString &url, const QString &type, uint length)
        : m_url(url)
        , m_type(type)
        , m_length(length)
    {
    }

    [[nodiscard]] QString url() const override
    {
        return m_url;
    }

    [[nodiscard]] QString type() const override
    {
        return m_type;
    }

    [[nodiscard]] QString title() const override
    {
        return m_title;
    }

    [[nodiscard]] uint length() const override
    {
        return m_length;
    }

    [[nodiscard]] uint duration() const override
    {
        return 0;
    }

    [[nodiscard]] bool isNull() const override
    {
        return m_url.isNull();
    }

private:
    const QString m_url;
    const QString m_type;
    const QString m_title; // TODO undefined.
    const uint m_length;
};
}

Article::Private::Private()
    : feed(nullptr)
    , archive(nullptr)
    , status(0)
    , hash(0)
    , pubDate(QDateTime::fromSecsSinceEpoch(1))
{
}

Article::Private::Private(const QString &guid_, Feed *feed_, Backend::FeedStorage *archive_)
    : feed(feed_)
    , guid(guid_)
    , archive(archive_)
{
    archive->article(guid, hash, title, status, pubDate);
}

Article::Private::Private(const ItemPtr &article, Feed *feed_, Backend::FeedStorage *archive_)
    : feed(feed_)
    , archive(archive_)
    , status(New)
    , hash(0)
{
    Q_ASSERT(archive);
    const QList<PersonPtr> authorList = article->authors();

    QString author;

    const PersonPtr firstAuthor = !authorList.isEmpty() ? authorList.first() : PersonPtr();

    hash = Utils::calcHash(article->title() + article->description() + article->content() + article->link() + author);

    guid = article->id();

    if (!archive->contains(guid)) {
        archive->addEntry(guid);

        archive->setHash(guid, hash);
        title = article->title();
        if (title.isEmpty()) {
            title = buildTitle(article->description());
        }
        archive->setTitle(guid, title);
        archive->setContent(guid, article->content());
        archive->setDescription(guid, article->description());
        archive->setLink(guid, article->link());
        archive->setGuidIsPermaLink(guid, false);
        archive->setGuidIsHash(guid, guid.startsWith(QLatin1StringView("hash:")));
        const time_t datePublished = article->datePublished();
        if (datePublished > 0) {
            pubDate = QDateTime::fromSecsSinceEpoch(datePublished);
        } else {
            pubDate = QDateTime::currentDateTime();
        }
        archive->setPubDate(guid, pubDate);
        if (firstAuthor) {
            archive->setAuthorName(guid, firstAuthor->name());
            archive->setAuthorUri(guid, firstAuthor->uri());
            archive->setAuthorEMail(guid, firstAuthor->email());
        }
    } else {
        // always update comments count, as it's not used for hash calculation
        if (hash != archive->hash(guid)) { // article is in archive, was it modified?
            // if yes, update
            pubDate = archive->pubDate(guid);
            archive->setHash(guid, hash);
            title = article->title();
            if (title.isEmpty()) {
                title = buildTitle(article->description());
            }
            archive->setTitle(guid, title);
            archive->setDescription(guid, article->description());
            archive->setContent(guid, article->content());
            archive->setLink(guid, article->link());
            if (firstAuthor) {
                archive->setAuthorName(guid, firstAuthor->name());
                archive->setAuthorUri(guid, firstAuthor->uri());
                archive->setAuthorEMail(guid, firstAuthor->email());
            }
        }
    }

    const QList<EnclosurePtr> encs = article->enclosures();
    if (!encs.isEmpty()) {
        archive->setEnclosure(guid, encs[0]->url(), encs[0]->type(), encs[0]->length());
    }
#if 0 // We need additionalProperties for Bug 366487
    qDebug() << "article " << article->additionalProperties().count();
    QMapIterator<QString, QDomElement> i(article->additionalProperties());
    while (i.hasNext()) {
        i.next();
        QString str;
        QTextStream s(&str, QIODevice::WriteOnly);
        i.value().save(s, 2);

        qDebug() << i.key() << ": " << str;
    }
#endif
}

Article::Article()
    : d(new Private)
{
}

Article::Article(const QString &guid, Feed *feed, Backend::FeedStorage *archive)
    : d()
{
    if (!archive) {
        archive = feed->storage()->archiveFor(feed->xmlUrl());
    }
    d = new Private(guid, feed, archive);
}

Article::Article(const Syndication::ItemPtr &article, Feed *feed)
    : d(new Private(article, feed, feed->storage()->archiveFor(feed->xmlUrl())))
{
}

Article::Article(const Syndication::ItemPtr &article, Backend::FeedStorage *archive)
    : d(new Private(article, nullptr, archive))
{
}

bool Article::isNull() const
{
    return d->archive == nullptr; // TODO: use proper null state
}

void Article::offsetPubDate(int secs)
{
    d->pubDate = d->pubDate.addSecs(secs);
    d->archive->setPubDate(d->guid, d->pubDate);
}

void Article::setDeleted()
{
    if (isDeleted()) {
        return;
    }

    setStatus(Read);
    d->status = Private::Deleted | Private::Read;
    d->archive->setStatus(d->guid, d->status);
    d->archive->setDeleted(d->guid);

    if (d->feed) {
        d->feed->setArticleDeleted(*this);
    }
}

bool Article::isDeleted() const
{
    return (d->status & Private::Deleted) != 0;
}

Article::Article(const Article &other)
    : d(other.d)
{
    d->ref();
}

Article::~Article()
{
    if (d->deref()) {
        delete d;
        d = nullptr;
    }
}

Article &Article::operator=(const Article &other)
{
    Article copy(other);
    swap(copy);
    return *this;
}

bool Article::operator<(const Article &other) const
{
    return pubDate() > other.pubDate() || (pubDate() == other.pubDate() && guid() < other.guid());
}

bool Article::operator<=(const Article &other) const
{
    return pubDate() > other.pubDate() || *this == other;
}

bool Article::operator>(const Article &other) const
{
    return pubDate() < other.pubDate() || (pubDate() == other.pubDate() && guid() > other.guid());
}

bool Article::operator>=(const Article &other) const
{
    return pubDate() > other.pubDate() || *this == other;
}

bool Article::operator==(const Article &other) const
{
    return d->guid == other.guid();
}

bool Article::operator!=(const Article &other) const
{
    return d->guid != other.guid();
}

int Article::status() const
{
    if ((d->status & Private::Read) != 0) {
        return Read;
    }

    if ((d->status & Private::New) != 0) {
        return New;
    }

    return Unread;
}

void Article::setStatus(int stat)
{
    int oldStatus = status();

    if (oldStatus != stat) {
        switch (stat) {
        case Read:
            d->status = (d->status | Private::Read) & ~Private::New;
            break;
        case Unread:
            d->status = (d->status & ~Private::Read) & ~Private::New;
            break;
        case New:
            d->status = (d->status | Private::New) & ~Private::Read;
            break;
        }
        if (d->archive) {
            d->archive->setStatus(d->guid, d->status);
        }
        if (d->feed) {
            d->feed->setArticleChanged(*this, oldStatus, stat != Read);
        }
    }
}

QString Article::title() const
{
    return d->title;
}

QString Article::authorName() const
{
    QString str;
    if (d->archive) {
        str = d->archive->authorName(d->guid);
    }
    return str;
}

QString Article::authorEMail() const
{
    QString str;
    if (d->archive) {
        str = d->archive->authorEMail(d->guid);
    }
    return str;
}

QString Article::authorUri() const
{
    QString str;
    if (d->archive) {
        str = d->archive->authorUri(d->guid);
    }
    return str;
}

QString Article::authorShort() const
{
    const QString name = authorName();
    if (!name.isEmpty()) {
        return name;
    }
    const QString email = authorEMail();
    if (!email.isEmpty()) {
        return email;
    }
    const QString uri = authorUri();
    if (!uri.isEmpty()) {
        return uri;
    }
    return {};
}

QString Article::authorAsHtml() const
{
    const QString name = authorName();
    const QString email = authorEMail();

    if (!email.isEmpty()) {
        if (!name.isEmpty()) {
            return QStringLiteral("<a href=\"mailto:%1\">%2</a>").arg(email, name);
        } else {
            return QStringLiteral("<a href=\"mailto:%1\">%1</a>").arg(email);
        }
    }

    const QString uri = authorUri();
    if (!name.isEmpty()) {
        if (!uri.isEmpty()) {
            return QStringLiteral("<a href=\"%1\">%2</a>").arg(uri, name);
        } else {
            return name;
        }
    }

    if (!uri.isEmpty()) {
        return QStringLiteral("<a href=\"%1\">%1</a>").arg(uri);
    }
    return {};
}

QUrl Article::link() const
{
    return QUrl(d->archive->link(d->guid));
}

QString Article::description() const
{
    return d->archive->description(d->guid);
}

QString Article::content(ContentOption opt) const
{
    const QString cnt = d->archive->content(d->guid);
    return opt == ContentAndOnlyContent ? cnt : (!cnt.isEmpty() ? cnt : description());
}

QString Article::guid() const
{
    return d->guid;
}

bool Article::guidIsPermaLink() const
{
    return d->archive->guidIsPermaLink(d->guid);
}

bool Article::guidIsHash() const
{
    return d->archive->guidIsHash(d->guid);
}

uint Article::hash() const
{
    return d->hash;
}

bool Article::keep() const
{
    return (d->status & Private::Keep) != 0;
}

void Article::setKeep(bool keep)
{
    d->status = keep ? (d->status | Private::Keep) : (d->status & ~Private::Keep);
    d->archive->setStatus(d->guid, d->status);
    if (d->feed) {
        d->feed->setArticleChanged(*this);
    }
}

Feed *Article::feed() const
{
    return d->feed;
}

QDateTime Article::pubDate() const
{
    return d->pubDate;
}

QSharedPointer<const Enclosure> Article::enclosure() const
{
    if (!d->enclosure) {
        QString url;
        QString type;
        int length;
        bool hasEnc;
        d->archive->enclosure(d->guid, hasEnc, url, type, length);
        if (hasEnc) {
            d->enclosure.reset(new EnclosureImpl(url, type, static_cast<uint>(length)));
        } else {
            d->enclosure.reset(new EnclosureImpl(QString(), QString(), 0));
        }
    }
    return d->enclosure;
}
} // namespace Akregator
