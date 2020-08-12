/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "feedstoragedummyimpl.h"
#include "storagedummyimpl.h"

#include <feed.h>

#include <QDateTime>
#include <QHash>
#include <QString>
#include <QStringList>

namespace Akregator {
namespace Backend {
class FeedStorageDummyImpl::FeedStorageDummyImplPrivate
{
public:
    class Entry
    {
    public:
        Entry()
            : pubDate()
        {
        }

        StorageDummyImpl *mainStorage = nullptr;
        QString enclosureUrl;
        QString enclosureType;
        QString title;
        QString description;
        QString content;
        QString link;
        QString authorName;
        QString authorUri;
        QString authorEMail;
        QStringList categories;
        int status = 0;
        int enclosureLength = 0;
        QDateTime pubDate;
        uint hash = 0;
        bool guidIsHash = false;
        bool guidIsPermaLink = false;
        bool hasEnclosure = false;
    };

    QHash<QString, Entry> entries;

    StorageDummyImpl *mainStorage;
    QString url;
};

FeedStorageDummyImpl::FeedStorageDummyImpl(const QString &url, StorageDummyImpl *main) : d(new FeedStorageDummyImplPrivate)
{
    d->url = url;
    d->mainStorage = main;
}

FeedStorageDummyImpl::~FeedStorageDummyImpl()
{
    delete d;
    d = nullptr;
}

int FeedStorageDummyImpl::unread() const
{
    return d->mainStorage->unreadFor(d->url);
}

void FeedStorageDummyImpl::setUnread(int unread)
{
    d->mainStorage->setUnreadFor(d->url, unread);
}

int FeedStorageDummyImpl::totalCount() const
{
    return d->mainStorage->totalCountFor(d->url);
}

void FeedStorageDummyImpl::setTotalCount(int total)
{
    d->mainStorage->setTotalCountFor(d->url, total);
}

QDateTime FeedStorageDummyImpl::lastFetch() const
{
    return d->mainStorage->lastFetchFor(d->url);
}

void FeedStorageDummyImpl::setLastFetch(const QDateTime &lastFetch)
{
    d->mainStorage->setLastFetchFor(d->url, lastFetch);
}

QStringList FeedStorageDummyImpl::articles() const
{
    return QStringList(d->entries.keys());
}

void FeedStorageDummyImpl::addEntry(const QString &guid)
{
    if (!d->entries.contains(guid)) {
        d->entries[guid] = FeedStorageDummyImplPrivate::Entry();
        setTotalCount(totalCount() + 1);
    }
}

bool FeedStorageDummyImpl::contains(const QString &guid) const
{
    return d->entries.contains(guid);
}

void FeedStorageDummyImpl::deleteArticle(const QString &guid)
{
    if (!d->entries.contains(guid)) {
        return;
    }

    setDeleted(guid);

    d->entries.remove(guid);
}

bool FeedStorageDummyImpl::guidIsHash(const QString &guid) const
{
    return contains(guid) ? d->entries[guid].guidIsHash : false;
}

bool FeedStorageDummyImpl::guidIsPermaLink(const QString &guid) const
{
    return contains(guid) ? d->entries[guid].guidIsPermaLink : false;
}

uint FeedStorageDummyImpl::hash(const QString &guid) const
{
    return contains(guid) ? d->entries[guid].hash : 0;
}

void FeedStorageDummyImpl::setDeleted(const QString &guid)
{
    if (!contains(guid)) {
        return;
    }

    FeedStorageDummyImplPrivate::Entry entry = d->entries[guid];

    entry.description.clear();
    entry.content.clear();
    entry.title.clear();
    entry.link.clear();
}

QString FeedStorageDummyImpl::link(const QString &guid) const
{
    return contains(guid) ? d->entries[guid].link : QString();
}

QDateTime FeedStorageDummyImpl::pubDate(const QString &guid) const
{
    return contains(guid) ? d->entries[guid].pubDate : QDateTime();
}

int FeedStorageDummyImpl::status(const QString &guid) const
{
    return contains(guid) ? d->entries[guid].status : 0;
}

void FeedStorageDummyImpl::setStatus(const QString &guid, int status)
{
    if (contains(guid)) {
        d->entries[guid].status = status;
    }
}

void FeedStorageDummyImpl::article(const QString &guid, uint &hash, QString &title, int &status, QDateTime &pubDate) const
{
    if (contains(guid)) {
        auto &entry = d->entries[guid];
        hash = entry.hash;
        title = entry.title;
        status = entry.status;
        pubDate = entry.pubDate;
    }
}

QString FeedStorageDummyImpl::title(const QString &guid) const
{
    return contains(guid) ? d->entries[guid].title : QString();
}

QString FeedStorageDummyImpl::description(const QString &guid) const
{
    return contains(guid) ? d->entries[guid].description : QString();
}

QString FeedStorageDummyImpl::content(const QString &guid) const
{
    return contains(guid) ? d->entries[guid].content : QString();
}

QString FeedStorageDummyImpl::authorName(const QString &guid) const
{
    return contains(guid) ? d->entries[guid].authorName : QString();
}

QString FeedStorageDummyImpl::authorUri(const QString &guid) const
{
    return contains(guid) ? d->entries[guid].authorUri : QString();
}

QString FeedStorageDummyImpl::authorEMail(const QString &guid) const
{
    return contains(guid) ? d->entries[guid].authorEMail : QString();
}

void FeedStorageDummyImpl::setPubDate(const QString &guid, const QDateTime &pubdate)
{
    if (contains(guid)) {
        d->entries[guid].pubDate = pubdate;
    }
}

void FeedStorageDummyImpl::setGuidIsHash(const QString &guid, bool isHash)
{
    if (contains(guid)) {
        d->entries[guid].guidIsHash = isHash;
    }
}

void FeedStorageDummyImpl::setLink(const QString &guid, const QString &link)
{
    if (contains(guid)) {
        d->entries[guid].link = link;
    }
}

void FeedStorageDummyImpl::setHash(const QString &guid, uint hash)
{
    if (contains(guid)) {
        d->entries[guid].hash = hash;
    }
}

void FeedStorageDummyImpl::setTitle(const QString &guid, const QString &title)
{
    if (contains(guid)) {
        d->entries[guid].title = title;
    }
}

void FeedStorageDummyImpl::setDescription(const QString &guid, const QString &description)
{
    if (contains(guid)) {
        d->entries[guid].description = description;
    }
}

void FeedStorageDummyImpl::setContent(const QString &guid, const QString &content)
{
    if (contains(guid)) {
        d->entries[guid].content = content;
    }
}

void FeedStorageDummyImpl::setAuthorName(const QString &guid, const QString &author)
{
    if (contains(guid)) {
        d->entries[guid].authorName = author;
    }
}

void FeedStorageDummyImpl::setAuthorUri(const QString &guid, const QString &author)
{
    if (contains(guid)) {
        d->entries[guid].authorUri = author;
    }
}

void FeedStorageDummyImpl::setAuthorEMail(const QString &guid, const QString &author)
{
    if (contains(guid)) {
        d->entries[guid].authorEMail = author;
    }
}

void FeedStorageDummyImpl::setGuidIsPermaLink(const QString &guid, bool isPermaLink)
{
    if (contains(guid)) {
        d->entries[guid].guidIsPermaLink = isPermaLink;
    }
}

void FeedStorageDummyImpl::setEnclosure(const QString &guid, const QString &url, const QString &type, int length)
{
    if (contains(guid)) {
        FeedStorageDummyImplPrivate::Entry entry = d->entries[guid];
        entry.hasEnclosure = true;
        entry.enclosureUrl = url;
        entry.enclosureType = type;
        entry.enclosureLength = length;
    }
}

void FeedStorageDummyImpl::removeEnclosure(const QString &guid)
{
    if (contains(guid)) {
        FeedStorageDummyImplPrivate::Entry entry = d->entries[guid];
        entry.hasEnclosure = false;
        entry.enclosureUrl.clear();
        entry.enclosureType.clear();
        entry.enclosureLength = -1;
    }
}

void FeedStorageDummyImpl::enclosure(const QString &guid, bool &hasEnclosure, QString &url, QString &type, int &length)  const
{
    if (contains(guid)) {
        FeedStorageDummyImplPrivate::Entry entry = d->entries[guid];
        hasEnclosure = entry.hasEnclosure;
        url = entry.enclosureUrl;
        type = entry.enclosureType;
        length = entry.enclosureLength;
    } else {
        hasEnclosure = false;
        url.clear();
        type.clear();
        length = -1;
    }
}

void FeedStorageDummyImpl::setCategories(const QString &guid, const QStringList &categories)
{
    if (contains(guid)) {
        d->entries[guid].categories = categories;
    }
}

QStringList FeedStorageDummyImpl::categories(const QString &guid) const
{
    return contains(guid) ? d->entries[guid].categories : QStringList();
}
} // namespace Backend
} // namespace Akregator
