/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <osterfeld@kde.org>

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

#include "feedstoragedummyimpl.h"
#include "storagedummyimpl.h"

#include <feed.h>

#include <QDateTime>
#include <QHash>
#include <QList>
#include <QMap>
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
        Entry() :
            status(0)
          , pubDate()
          , hash(0)
          , guidIsHash(false)
          , guidIsPermaLink(false)
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
        QString commentsLink;
        int comments;
        int status;
        int enclosureLength;
        QDateTime pubDate;
        uint hash;
        bool guidIsHash = false;
        bool guidIsPermaLink = false;
        bool hasEnclosure = false;
    };

    QHash<QString, Entry> entries;

    Storage *mainStorage;
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

void FeedStorageDummyImpl::commit()
{
}

void FeedStorageDummyImpl::rollback()
{
}

void FeedStorageDummyImpl::close()
{
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

int FeedStorageDummyImpl::comments(const QString &guid) const
{
    return contains(guid) ? d->entries[guid].comments : 0;
}

QString FeedStorageDummyImpl::commentsLink(const QString &guid) const
{
    return contains(guid) ? d->entries[guid].commentsLink : QString();
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
    entry.commentsLink.clear();
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

void FeedStorageDummyImpl::setCommentsLink(const QString &guid, const QString &commentsLink)
{
    if (contains(guid)) {
        d->entries[guid].commentsLink = commentsLink;
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

void FeedStorageDummyImpl::setComments(const QString &guid, int comments)
{
    if (contains(guid)) {
        d->entries[guid].comments = comments;
    }
}

void FeedStorageDummyImpl::setGuidIsPermaLink(const QString &guid, bool isPermaLink)
{
    if (contains(guid)) {
        d->entries[guid].guidIsPermaLink = isPermaLink;
    }
}

void FeedStorageDummyImpl::add(FeedStorage *source)
{
    QStringList articles = source->articles();
    for (QStringList::ConstIterator it = articles.constBegin(); it != articles.constEnd(); ++it) {
        copyArticle(*it, source);
    }
    setUnread(source->unread());
    setLastFetch(source->lastFetch());
    setTotalCount(source->totalCount());
}

void FeedStorageDummyImpl::copyArticle(const QString &guid, FeedStorage *source)
{
    if (!contains(guid)) {
        addEntry(guid);
    }

    setComments(guid, source->comments(guid));
    setCommentsLink(guid, source->commentsLink(guid));
    setDescription(guid, source->description(guid));
    setContent(guid, source->content(guid));
    setGuidIsHash(guid, source->guidIsHash(guid));
    setGuidIsPermaLink(guid, source->guidIsPermaLink(guid));
    setHash(guid, source->hash(guid));
    setLink(guid, source->link(guid));
    setPubDate(guid, source->pubDate(guid));
    setStatus(guid, source->status(guid));
    setTitle(guid, source->title(guid));
}

void FeedStorageDummyImpl::clear()
{
    d->entries.clear();
    setUnread(0);
    setTotalCount(0);
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
} // namespace Backend
} // namespace Akregator
