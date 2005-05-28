/*
    This file is part of Akregator.

    2005 Frank Osterfeld <frank.osterfeld@kdemail.net>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#include "storagedummyimpl.h"
#include "feedstoragedummyimpl.h"

#include <qmap.h>
#include <qstring.h>

namespace Akregator {
namespace Backend {

class StorageDummyImpl::StorageDummyImplPrivate
{
    public:
    class Entry
    {
        public:
        int unread;
        int totalCount;
        int lastFetch;
	FeedStorage* feedStorage;
    };

    void addEntry(const QString& url, int unread, int totalCount, int lastFetch)
    {
        Entry entry;
        entry.unread = unread;
        entry.totalCount = totalCount;
        entry.lastFetch = lastFetch;
        entry.feedStorage = 0;
        feeds[url] = entry;
	
    }

    QMap<QString, Entry> feeds;
};

StorageDummyImpl::StorageDummyImpl() : d(new StorageDummyImplPrivate)
{
}

StorageDummyImpl::~StorageDummyImpl()
{
    delete d; d = 0;
}
void StorageDummyImpl::initialize(const QStringList&) {}

bool StorageDummyImpl::open(bool /*autoCommit*/)
{
    return true;
}

bool StorageDummyImpl::autoCommit() const
{
    return false;
}

bool StorageDummyImpl::close()
{
    for (QMap<QString, StorageDummyImplPrivate::Entry>::ConstIterator it = d->feeds.begin(); it != d->feeds.end(); ++it)
    {
        (*it).feedStorage->close();
        delete (*it).feedStorage;
    }
    return true;
}

bool StorageDummyImpl::commit()
{
    return true;
}

bool StorageDummyImpl::rollback()
{
    return true;
}

int StorageDummyImpl::unreadFor(const QString &url)
{
    return d->feeds.contains(url) ? d->feeds[url].unread : 0;
}

void StorageDummyImpl::setUnreadFor(const QString &url, int unread)
{
    if (!d->feeds.contains(url))
       d->addEntry(url, unread, unread, 0);
    else
       d->feeds[url].unread = unread;
}

int StorageDummyImpl::totalCountFor(const QString &url)
{
    return d->feeds.contains(url) ? d->feeds[url].totalCount : 0;
}

void StorageDummyImpl::setTotalCountFor(const QString &url, int total)
{
    if (!d->feeds.contains(url))
       d->addEntry(url, 0, total, 0);
    else
       d->feeds[url].totalCount = total;
}

int StorageDummyImpl::lastFetchFor(const QString& url)
{
    return d->feeds.contains(url) ? d->feeds[url].lastFetch : 0;
}

void StorageDummyImpl::setLastFetchFor(const QString& url, int lastFetch)
{
    if (!d->feeds.contains(url))
       d->addEntry(url, 0, 0, lastFetch);
    else 
       d->feeds[url].lastFetch = lastFetch;
}
        
void StorageDummyImpl::slotCommit()
{
}

FeedStorage* StorageDummyImpl::archiveFor(const QString& url)
{
    if (!d->feeds.contains(url))
        d->feeds[url].feedStorage = new FeedStorageDummyImpl(url, this);

    return d->feeds[url].feedStorage;
}

}
}

#include "storagedummyimpl.moc"
