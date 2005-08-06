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

#include "feedstoragedummyimpl.h"
#include "storagedummyimpl.h"

#include <feed.h>

#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>

//typedef unsigned int uint;
namespace Akregator {
namespace Backend {

class FeedStorageDummyImpl::FeedStorageDummyImplPrivate
{
    public:
        class Entry
        {            
            public:
            Entry() : guidIsHash(false), guidIsPermaLink(false), status(0), pubDate(0), hash(0) {}
            StorageDummyImpl* mainStorage;
            QValueList<Category> categories;
            QString title;
            QString description;
            QString link;
            QString commentsLink;
            bool guidIsHash;
            bool guidIsPermaLink;
            int comments;
            int status;
            uint pubDate;
            uint hash;
            QStringList tags;
            bool hasEnclosure;
            QString enclosureUrl;
            QString enclosureType;
            int enclosureLength;
        };
    QMap<QString, Entry> entries;
    
    // all tags occurring in the feed
    QStringList tags;
    
    // tag -> articles index
    QMap<QString, QStringList > taggedArticles;

    QValueList<Category> categories;
    QMap<Category, QStringList> categorizedArticles;

    Storage* mainStorage;
    QString url;
};


void FeedStorageDummyImpl::convertOldArchive()
{
}

FeedStorageDummyImpl::FeedStorageDummyImpl(const QString& url, StorageDummyImpl* main) : d(new FeedStorageDummyImplPrivate)
{
    d->url = url;
    d->mainStorage = main;
}

FeedStorageDummyImpl::~FeedStorageDummyImpl()
{
    delete d; d = 0;
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

int FeedStorageDummyImpl::unread()
{
    return d->mainStorage->unreadFor(d->url);
}

void FeedStorageDummyImpl::setUnread(int unread)
{
    d->mainStorage->setUnreadFor(d->url, unread);
}

int FeedStorageDummyImpl::totalCount()
{
    return d->mainStorage->totalCountFor(d->url);
}

void FeedStorageDummyImpl::setTotalCount(int total)
{
    d->mainStorage->setTotalCountFor(d->url, total);
}

int FeedStorageDummyImpl::lastFetch()
{
    return d->mainStorage->lastFetchFor(d->url);
}

void FeedStorageDummyImpl::setLastFetch(int lastFetch)
{
    d->mainStorage->setLastFetchFor(d->url, lastFetch);
}

QStringList FeedStorageDummyImpl::articles(const QString& tag)
{
    return tag.isNull() ? d->entries.keys() : d->taggedArticles[tag];
}

QStringList FeedStorageDummyImpl::articles(const Category& cat)
{
    return d->categorizedArticles[cat];
}

void FeedStorageDummyImpl::addEntry(const QString& guid)
{
    if (!d->entries.contains(guid))
    {
        d->entries[guid] = FeedStorageDummyImplPrivate::Entry();
        setTotalCount(totalCount()+1);
    }
}

bool FeedStorageDummyImpl::contains(const QString& guid)
{
    return d->entries.contains(guid);
}

void FeedStorageDummyImpl::deleteArticle(const QString& guid)
{
    if (!d->entries.contains(guid))
        return;

    setDeleted(guid);

    d->entries.remove(guid);
}

int FeedStorageDummyImpl::comments(const QString& guid)
{
    
    return contains(guid) ? d->entries[guid].comments : 0;
}

QString FeedStorageDummyImpl::commentsLink(const QString& guid)
{
    return contains(guid) ? d->entries[guid].commentsLink : "";
}

bool FeedStorageDummyImpl::guidIsHash(const QString& guid)
{
    return contains(guid) ? d->entries[guid].guidIsHash : false;
}

bool FeedStorageDummyImpl::guidIsPermaLink(const QString& guid)
{
    return contains(guid) ? d->entries[guid].guidIsPermaLink : false;
}

uint FeedStorageDummyImpl::hash(const QString& guid)
{
    return contains(guid) ? d->entries[guid].hash : 0;
}


void FeedStorageDummyImpl::setDeleted(const QString& guid)
{
    if (!contains(guid))
        return;

    FeedStorageDummyImplPrivate::Entry entry = d->entries[guid];

    // remove article from tag->article index
    QStringList::ConstIterator it = entry.tags.begin();
    QStringList::ConstIterator end = entry.tags.end();

    for ( ; it != end; ++it)
    {
        d->taggedArticles[*it].remove(guid);
        if (d->taggedArticles[*it].count() == 0)
            d->tags.remove(*it);
    }

    // remove article from tag->category index
    QValueList<Category>::ConstIterator it2 = entry.categories.begin();
    QValueList<Category>::ConstIterator end2 = entry.categories.end();

    for ( ; it2 != end2; ++it2)
    {
        d->categorizedArticles[*it2].remove(guid);
        if (d->categorizedArticles[*it2].count() == 0)
            d->categories.remove(*it2);
    }

    entry.description = "";
    entry.title = "";
    entry.link = "";
    entry.commentsLink = "";
}

QString FeedStorageDummyImpl::link(const QString& guid)
{
    return contains(guid) ? d->entries[guid].link : "";
}

uint FeedStorageDummyImpl::pubDate(const QString& guid)
{
    return contains(guid) ? d->entries[guid].pubDate : 0;
}

int FeedStorageDummyImpl::status(const QString& guid)
{
    return contains(guid) ? d->entries[guid].status : 0;
}

void FeedStorageDummyImpl::setStatus(const QString& guid, int status)
{
    if (contains(guid))
        d->entries[guid].status = status;
}

QString FeedStorageDummyImpl::title(const QString& guid)
{
    return contains(guid) ? d->entries[guid].title : "";
}

QString FeedStorageDummyImpl::description(const QString& guid)
{
    return contains(guid) ? d->entries[guid].description : "";
}


void FeedStorageDummyImpl::setPubDate(const QString& guid, uint pubdate)
{
    if (contains(guid))
        d->entries[guid].pubDate = pubdate;
}

void FeedStorageDummyImpl::setGuidIsHash(const QString& guid, bool isHash)
{
    if (contains(guid))
        d->entries[guid].guidIsHash = isHash;
}

void FeedStorageDummyImpl::setLink(const QString& guid, const QString& link)
{
    if (contains(guid))
        d->entries[guid].link = link;
}

void FeedStorageDummyImpl::setHash(const QString& guid, uint hash)
{
    if (contains(guid))
        d->entries[guid].hash = hash;
}

void FeedStorageDummyImpl::setTitle(const QString& guid, const QString& title)
{
    if (contains(guid))
        d->entries[guid].title = title;
}

void FeedStorageDummyImpl::setDescription(const QString& guid, const QString& description)
{
    if (contains(guid))
        d->entries[guid].description = description;
}

void FeedStorageDummyImpl::setCommentsLink(const QString& guid, const QString& commentsLink)
{
    if (contains(guid))
        d->entries[guid].commentsLink = commentsLink;
}

void FeedStorageDummyImpl::setComments(const QString& guid, int comments)
{
    if (contains(guid))
        d->entries[guid].comments = comments;
}


void FeedStorageDummyImpl::setGuidIsPermaLink(const QString& guid, bool isPermaLink)
{
    if (contains(guid))
        d->entries[guid].guidIsPermaLink = isPermaLink;
}

void FeedStorageDummyImpl::addTag(const QString& guid, const QString& tag)
{
    if (contains(guid))
    {
        d->entries[guid].tags.append(tag);
        if (!d->taggedArticles[tag].contains(guid))
            d->taggedArticles[tag].append(guid);
        if (!d->tags.contains(tag))
            d->tags.append(tag);
    }

}

void FeedStorageDummyImpl::addCategory(const QString& guid, const Category& cat)
{
    if (!contains(guid))
        return;

    d->entries[guid].categories.append(cat);

    if (d->categorizedArticles[cat].count() == 0)
        d->categories.append(cat);
    d->categorizedArticles[cat].append(guid);
}

QValueList<Category> FeedStorageDummyImpl::categories(const QString& guid)
{
  if (!guid.isNull())
        return contains(guid) ? d->entries[guid].categories : QValueList<Category>();
    else
        return d->categories;
}


void FeedStorageDummyImpl::removeTag(const QString& guid, const QString& tag)
{
    if (contains(guid))
    {
        d->entries[guid].tags.remove(tag);
        d->taggedArticles[tag].remove(guid);
        if (d->taggedArticles[tag].count() == 0)
            d->tags.remove(tag);
    }
}

QStringList FeedStorageDummyImpl::tags(const QString& guid)
{
    if (!guid.isNull())
        return contains(guid) ? d->entries[guid].tags : QStringList();
    else
    {
        return d->tags;
    }
}

void FeedStorageDummyImpl::add(FeedStorage* source)
{
    QStringList articles = source->articles();
    for (QStringList::ConstIterator it = articles.begin(); it != articles.end(); ++it)
        copyArticle(*it, source);
    setUnread(source->unread());
    setLastFetch(source->lastFetch());
    setTotalCount(source->totalCount());
}

void FeedStorageDummyImpl::copyArticle(const QString& guid, FeedStorage* source)
{
    if (!contains(guid))
        addEntry(guid);

    setComments(guid, source->comments(guid));
    setCommentsLink(guid, source->commentsLink(guid));
    setDescription(guid, source->description(guid));
    setGuidIsHash(guid, source->guidIsHash(guid));
    setGuidIsPermaLink(guid, source->guidIsPermaLink(guid));
    setHash(guid, source->hash(guid));
    setLink(guid, source->link(guid));
    setPubDate(guid, source->pubDate(guid));
    setStatus(guid, source->status(guid));
    setTitle(guid, source->title(guid));
    QStringList tags = source->tags(guid);
    
    for (QStringList::ConstIterator it = tags.begin(); it != tags.end(); ++it)
        addTag(guid, *it);
}

void FeedStorageDummyImpl::clear()
{
    d->entries.clear();
    setUnread(0);
    setTotalCount(0);
}

void FeedStorageDummyImpl::setEnclosure(const QString& guid, const QString& url, const QString& type, int length)
{
    if (contains(guid))
    {
        FeedStorageDummyImplPrivate::Entry entry = d->entries[guid];
        entry.hasEnclosure = true;
        entry.enclosureUrl = url;
        entry.enclosureType = type;
        entry.enclosureLength = length;
    }
}

void FeedStorageDummyImpl::removeEnclosure(const QString& guid)
{
    if (contains(guid))
    {
        FeedStorageDummyImplPrivate::Entry entry = d->entries[guid];
        entry.hasEnclosure = false;
        entry.enclosureUrl = QString::null;
        entry.enclosureType = QString::null;
        entry.enclosureLength = -1;
    }
}

void FeedStorageDummyImpl::enclosure(const QString& guid, bool& hasEnclosure, QString& url, QString& type, int& length)
{
    if (contains(guid))
    {
        FeedStorageDummyImplPrivate::Entry entry = d->entries[guid];
        hasEnclosure = entry.hasEnclosure;
        url = entry.enclosureUrl;
        type = entry.enclosureType;
        length = entry.enclosureLength;
    }
    else
    {
        hasEnclosure = false;
        url = QString::null;
        type = QString::null;
        length = -1;
    }
}

} // namespace Backend
} // namespace Akregator
