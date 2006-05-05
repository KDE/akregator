/*
    This file is part of Akregator.

    Copyright (C) 2005 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#include "storagemk4impl.h"
#include "feedstoragemk4impl.h"

#include <mk4.h>

#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtimer.h>

#include <kdebug.h>
#include <kstandarddirs.h>

namespace Akregator {
namespace Backend {

class StorageMK4Impl::StorageMK4ImplPrivate
{
    public:
        StorageMK4ImplPrivate() : modified(false),
            purl("url"),
            pFeedList("feedList"),
            pTagSet("tagSet"),
            punread("unread"),
            ptotalCount("totalCount"),
            plastFetch("lastFetch") {}

        c4_Storage* storage;
        c4_View archiveView;
        bool autoCommit;
        bool modified;
        QMap<QString, FeedStorage*> feeds;
        QStringList feedURLs;
        c4_StringProp purl, pFeedList, pTagSet;
        c4_IntProp punread, ptotalCount, plastFetch;
        QTimer* commitTimer;
        QString archivePath;

        c4_Storage* feedListStorage;
        c4_View feedListView;
};

void StorageMK4Impl::setArchivePath(const QString& archivePath)
{
    if (archivePath.isNull()) // if isNull, reset to default
        d->archivePath = defaultArchivePath();
    else
        d->archivePath = archivePath;
}

QString StorageMK4Impl::archivePath() const
{
    return d->archivePath;
}

StorageMK4Impl::StorageMK4Impl() : d(new StorageMK4ImplPrivate)
{
    setArchivePath(QString::null); // set path to default
    // commit changes every 3 seconds
    d->commitTimer = new QTimer(this);
    connect(d->commitTimer, SIGNAL(timeout()), this, SLOT(slotCommit()));
    d->commitTimer->start(3000);
}

QString StorageMK4Impl::defaultArchivePath()
{
    return KGlobal::dirs()->saveLocation("data", "akregator")+"Archive";
}

StorageMK4Impl::~StorageMK4Impl()
{
    close();
    delete d;
    d = 0;
}
void StorageMK4Impl::initialize(const QStringList&) {}

bool StorageMK4Impl::open(bool autoCommit)
{
    QString filePath = d->archivePath +"/archiveindex.mk4";
    d->storage = new c4_Storage(filePath.toLocal8Bit(), true);
    d->archiveView = d->storage->GetAs("archive[url:S,unread:I,totalCount:I,lastFetch:I]");
    c4_View hash = d->storage->GetAs("archiveHash[_H:I,_R:I]");
    d->archiveView = d->archiveView.Hash(hash, 1); // hash on url
    d->autoCommit = autoCommit;

    filePath = d->archivePath +"/feedlistbackup.mk4";
    d->feedListStorage = new c4_Storage(filePath.toLocal8Bit(), true);
    d->feedListView = d->feedListStorage->GetAs("archive[feedList:S,tagSet:S]");
    return true;
}

bool StorageMK4Impl::autoCommit() const
{
    return d->autoCommit;
}

bool StorageMK4Impl::close()
{
    d->commitTimer->stop();

    QMap<QString, FeedStorage*>::Iterator it;
    QMap<QString, FeedStorage*>::Iterator end(d->feeds.end() ) ;
    for (it = d->feeds.begin(); it != end; ++it)
    {
        it.value()->close();
        delete it.value();
    }
    if(d->autoCommit)
        d->storage->Commit();

    delete d->storage; 
    d->storage = 0;
    
    d->feedListStorage->Commit();
    delete d->feedListStorage;
    d->feedListStorage = 0;

    return true;
}

bool StorageMK4Impl::commit()
{
    QMap<QString, FeedStorage*>::Iterator it;
    QMap<QString, FeedStorage*>::Iterator end(d->feeds.end() ) ;
    for ( it = d->feeds.begin(); it != end; ++it )
        it.value()->commit();

    if(d->storage)
    {
        d->storage->Commit();
        return true;
    }

    return false;
}

bool StorageMK4Impl::rollback()
{
    QMap<QString, FeedStorage*>::Iterator it;
    QMap<QString, FeedStorage*>::Iterator end(d->feeds.end() ) ;
    for ( it = d->feeds.begin(); it != end; ++it )
        it.value()->rollback();

    if(d->storage)
    {
        d->storage->Rollback();
        return true;
    }
    return false;
}

int StorageMK4Impl::unreadFor(const QString &url)
{
    c4_Row findrow;
    d->purl(findrow) = url.toAscii();
    int findidx = d->archiveView.Find(findrow);

    return findidx != -1 ? d->punread(d->archiveView.GetAt(findidx)) : 0;
}

void StorageMK4Impl::setUnreadFor(const QString &url, int unread)
{
    c4_Row findrow;
    d->purl(findrow) = url.toAscii();
    int findidx = d->archiveView.Find(findrow);
    if (findidx == -1)
        return;
    findrow = d->archiveView.GetAt(findidx);
    d->punread(findrow) = unread;
    d->archiveView.SetAt(findidx, findrow);
    d->modified = true;
}

int StorageMK4Impl::totalCountFor(const QString &url)
{
    c4_Row findrow;
    d->purl(findrow) = url.toAscii();
    int findidx = d->archiveView.Find(findrow);

    return findidx != -1 ? d->ptotalCount(d->archiveView.GetAt(findidx)) : 0;
}

void StorageMK4Impl::setTotalCountFor(const QString &url, int total)
{
    c4_Row findrow;
    d->purl(findrow) = url.toAscii();
    int findidx = d->archiveView.Find(findrow);
    if (findidx == -1)
        return;
    findrow = d->archiveView.GetAt(findidx);
    d->ptotalCount(findrow) = total;
    d->archiveView.SetAt(findidx, findrow);
    d->modified = true;
}

int StorageMK4Impl::lastFetchFor(const QString& url)
{
    c4_Row findrow;
    d->purl(findrow) = url.toAscii();
    int findidx = d->archiveView.Find(findrow);

    return (findidx != -1 ? d->plastFetch(d->archiveView.GetAt(findidx)) : 0);
}

void StorageMK4Impl::setLastFetchFor(const QString& url, int lastFetch)
{
    c4_Row findrow;
    d->purl(findrow) = url.toAscii();
    int findidx = d->archiveView.Find(findrow);
    if (findidx == -1)
        return;
    findrow = d->archiveView.GetAt(findidx);
    d->plastFetch(findrow) = lastFetch;
    d->archiveView.SetAt(findidx, findrow);
    d->modified = true;
}

void StorageMK4Impl::slotCommit()
{
    if (d->modified)
    	commit();
    d->modified = false;
}

FeedStorage* StorageMK4Impl::archiveFor(const QString& url)
{
    if (!d->feeds.contains(url))
    {
        FeedStorage* fs = new FeedStorageMK4Impl(url, this);
        d->feeds[url] = fs;
        c4_Row findrow;
        d->purl(findrow) = url.toAscii();
        int findidx = d->archiveView.Find(findrow);
        if (findidx == -1)
        {
            d->punread(findrow) = 0;
            d->ptotalCount(findrow) = 0;
            d->plastFetch(findrow) = 0;
            d->archiveView.Add(findrow);
            d->modified = true;
        }
        fs->convertOldArchive();
    }
    return d->feeds[url];
}

QStringList StorageMK4Impl::feeds() const
{
    // TODO: cache list
    QStringList list;
    int size = d->archiveView.GetSize();
    for (int i = 0; i < size; i++)
        list += QString(d->purl(d->archiveView.GetAt(i)));
    // fill with urls
    return list;

}

void StorageMK4Impl::add(Storage* source)
{
    QStringList feeds = source->feeds();
    QStringList::ConstIterator end(feeds.end() ) ;

    for (QStringList::ConstIterator it = feeds.begin(); it != end; ++it)
    {
        FeedStorage* fa = archiveFor(*it);
        fa->add(source->archiveFor(*it));
    }
}


void StorageMK4Impl::clear()
{
   QStringList feeds;
    int size = d->archiveView.GetSize();
    for (int i = 0; i < size; i++)
        feeds += QString(d->purl(d->archiveView.GetAt(i)));
    QStringList::ConstIterator end(feeds.end() ) ;

    for (QStringList::ConstIterator it = feeds.begin(); it != end; ++it)
    {
        FeedStorage* fa = archiveFor(*it);
        fa->clear();
        fa->commit();
        // FIXME: delete file (should be 0 in size now)
    }
    d->storage->RemoveAll();

}

void StorageMK4Impl::storeFeedList(const QString& opmlStr)
{
   
    if  (d->feedListView.GetSize() == 0)
    {
        c4_Row row;
        d->pFeedList(row) = !opmlStr.isEmpty() ? opmlStr.toUtf8().data() : "";
        d->pTagSet(row) = "";
        d->feedListView.Add(row);
    }
    else
    {
        c4_Row row = d->feedListView.GetAt(0);
        d->pFeedList(row) = !opmlStr.isEmpty() ? opmlStr.toUtf8().data() : "";
        d->feedListView.SetAt(0, row);
    }
    d->modified = true;
}

QString StorageMK4Impl::restoreFeedList() const
{
    if  (d->feedListView.GetSize() == 0)
        return "";

    c4_Row row = d->feedListView.GetAt(0);
    return QString::fromUtf8(d->pFeedList(row));
}

void StorageMK4Impl::storeTagSet(const QString& xmlStr)
{
   
    if  (d->feedListView.GetSize() == 0)
    {
        c4_Row row;
        d->pTagSet(row) = !xmlStr.isEmpty() ? xmlStr.toUtf8().data() : "";
        d->pFeedList(row) = "";
        d->feedListView.Add(row);
    }
    else
    {
        c4_Row row = d->feedListView.GetAt(0);
        d->pTagSet(row) = !xmlStr.isEmpty() ? xmlStr.toUtf8().data() : "";
        d->feedListView.SetAt(0, row);
    }
    d->modified = true;
}

QString StorageMK4Impl::restoreTagSet() const
{
    if  (d->feedListView.GetSize() == 0)
        return "";

    c4_Row row = d->feedListView.GetAt(0);
    return QString::fromUtf8(d->pTagSet(row));
}

} // namespace Backend
} // namespace Akregator

#include "storagemk4impl.moc"
