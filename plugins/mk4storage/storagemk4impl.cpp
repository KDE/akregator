/*
    This file is part of Akregator.

    Copyright (C) 2005 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
                  2005 Frank Osterfeld <osterfeld@kde.org>

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

#include <QMap>
#include <QString>
#include <QStringList>
#include <QTimer>

#include <kdebug.h>
#include <kstandarddirs.h>


class Akregator::Backend::StorageMK4Impl::StorageMK4ImplPrivate
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
        Akregator::Backend::StorageMK4Impl* q;
        c4_View archiveView;
        bool autoCommit;
        bool modified;
        mutable QMap<QString, Akregator::Backend::FeedStorageMK4Impl*> feeds;
        QStringList feedURLs;
        c4_StringProp purl, pFeedList, pTagSet;
        c4_IntProp punread, ptotalCount, plastFetch;
        QString archivePath;

        c4_Storage* feedListStorage;
        c4_View feedListView;

        Akregator::Backend::FeedStorageMK4Impl* createFeedStorage( const QString& url );
};

Akregator::Backend::StorageMK4Impl::StorageMK4Impl() : d(new StorageMK4ImplPrivate)
{
    d->q = this;
    setArchivePath(QString::null); // set path to default        //krazy:exclude=nullstrassign for old broken gcc
}

Akregator::Backend::FeedStorageMK4Impl* Akregator::Backend::StorageMK4Impl::StorageMK4ImplPrivate::createFeedStorage( const QString& url )
{
    if (!feeds.contains(url))
    {
        Akregator::Backend::FeedStorageMK4Impl* fs = new Akregator::Backend::FeedStorageMK4Impl(url, q);
        feeds[url] = fs;
        c4_Row findrow;
        purl(findrow) = url.toLatin1();
        int findidx = archiveView.Find(findrow);
        if (findidx == -1)
        {
            punread(findrow) = 0;
            ptotalCount(findrow) = 0;
            plastFetch(findrow) = 0;
            archiveView.Add(findrow);
            modified = true;
        }
        fs->convertOldArchive();
    }
    return feeds[url];
}

Akregator::Backend::FeedStorage* Akregator::Backend::StorageMK4Impl::archiveFor(const QString& url)
{
    return d->createFeedStorage( url );
}

const Akregator::Backend::FeedStorage* Akregator::Backend::StorageMK4Impl::archiveFor(const QString& url) const
{
    return d->createFeedStorage( url );
}

void Akregator::Backend::StorageMK4Impl::setArchivePath(const QString& archivePath)
{
    if (archivePath.isNull()) // if isNull, reset to default
        d->archivePath = defaultArchivePath();
    else
        d->archivePath = archivePath;
}

QString Akregator::Backend::StorageMK4Impl::archivePath() const
{
    return d->archivePath;
}

QString Akregator::Backend::StorageMK4Impl::defaultArchivePath()
{
    return KGlobal::dirs()->saveLocation("data", "akregator")+"Archive";
}

Akregator::Backend::StorageMK4Impl::~StorageMK4Impl()
{
    close();
    delete d;
    d = 0;
}
void Akregator::Backend::StorageMK4Impl::initialize(const QStringList&) {}

bool Akregator::Backend::StorageMK4Impl::open(bool autoCommit)
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

bool Akregator::Backend::StorageMK4Impl::autoCommit() const
{
    return d->autoCommit;
}

bool Akregator::Backend::StorageMK4Impl::close()
{
    QMap<QString, FeedStorageMK4Impl*>::Iterator it;
    QMap<QString, FeedStorageMK4Impl*>::Iterator end(d->feeds.end() ) ;
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

bool Akregator::Backend::StorageMK4Impl::commit()
{
    QMap<QString, FeedStorageMK4Impl*>::Iterator it;
    QMap<QString, FeedStorageMK4Impl*>::Iterator end(d->feeds.end() ) ;
    for ( it = d->feeds.begin(); it != end; ++it )
        it.value()->commit();

    if(d->storage)
    {
        d->storage->Commit();
        return true;
    }

    return false;
}

bool Akregator::Backend::StorageMK4Impl::rollback()
{
    QMap<QString, FeedStorageMK4Impl*>::Iterator it;
    QMap<QString, FeedStorageMK4Impl*>::Iterator end(d->feeds.end() ) ;
    for ( it = d->feeds.begin(); it != end; ++it )
        it.value()->rollback();

    if(d->storage)
    {
        d->storage->Rollback();
        return true;
    }
    return false;
}

int Akregator::Backend::StorageMK4Impl::unreadFor(const QString &url) const
{
    c4_Row findrow;
    d->purl(findrow) = url.toLatin1();
    int findidx = d->archiveView.Find(findrow);

    return findidx != -1 ? d->punread(d->archiveView.GetAt(findidx)) : 0;
}

void Akregator::Backend::StorageMK4Impl::setUnreadFor(const QString &url, int unread)
{
    c4_Row findrow;
    d->purl(findrow) = url.toLatin1();
    int findidx = d->archiveView.Find(findrow);
    if (findidx == -1)
        return;
    findrow = d->archiveView.GetAt(findidx);
    d->punread(findrow) = unread;
    d->archiveView.SetAt(findidx, findrow);
    markDirty();
}

int Akregator::Backend::StorageMK4Impl::totalCountFor(const QString &url) const
{
    c4_Row findrow;
    d->purl(findrow) = url.toLatin1();
    int findidx = d->archiveView.Find(findrow);

    return findidx != -1 ? d->ptotalCount(d->archiveView.GetAt(findidx)) : 0;
}

void Akregator::Backend::StorageMK4Impl::setTotalCountFor(const QString &url, int total)
{
    c4_Row findrow;
    d->purl(findrow) = url.toLatin1();
    int findidx = d->archiveView.Find(findrow);
    if (findidx == -1)
        return;
    findrow = d->archiveView.GetAt(findidx);
    d->ptotalCount(findrow) = total;
    d->archiveView.SetAt(findidx, findrow);
    markDirty();
}

int Akregator::Backend::StorageMK4Impl::lastFetchFor(const QString& url) const
{
    c4_Row findrow;
    d->purl(findrow) = url.toLatin1();
    int findidx = d->archiveView.Find(findrow);

    return (findidx != -1 ? d->plastFetch(d->archiveView.GetAt(findidx)) : 0);
}

void Akregator::Backend::StorageMK4Impl::setLastFetchFor(const QString& url, int lastFetch)
{
    c4_Row findrow;
    d->purl(findrow) = url.toLatin1();
    int findidx = d->archiveView.Find(findrow);
    if (findidx == -1)
        return;
    findrow = d->archiveView.GetAt(findidx);
    d->plastFetch(findrow) = lastFetch;
    d->archiveView.SetAt(findidx, findrow);
    markDirty();
}

void Akregator::Backend::StorageMK4Impl::markDirty()
{
    if (!d->modified)
    {
        d->modified = true;
        // commit changes after 3 seconds
        QTimer::singleShot(3000, this, SLOT(slotCommit()));
    }
}

void Akregator::Backend::StorageMK4Impl::slotCommit()
{
    if (d->modified)
        commit();
    d->modified = false;
}

QStringList Akregator::Backend::StorageMK4Impl::feeds() const
{
    // TODO: cache list
    QStringList list;
    int size = d->archiveView.GetSize();
    for (int i = 0; i < size; ++i)
        list += QString(d->purl(d->archiveView.GetAt(i)));
    // fill with urls
    return list;

}

void Akregator::Backend::StorageMK4Impl::add(Storage* source)
{
    QStringList feeds = source->feeds();
    QStringList::ConstIterator end(feeds.constEnd() ) ;

    for (QStringList::ConstIterator it = feeds.constBegin(); it != end; ++it)
    {
        FeedStorage* fa = archiveFor(*it);
        fa->add(source->archiveFor(*it));
    }
}


void Akregator::Backend::StorageMK4Impl::clear()
{
   QStringList feeds;
    int size = d->archiveView.GetSize();
    for (int i = 0; i < size; ++i)
        feeds += QString(d->purl(d->archiveView.GetAt(i)));
    QStringList::ConstIterator end(feeds.constEnd() ) ;

    for (QStringList::ConstIterator it = feeds.constBegin(); it != end; ++it)
    {
        FeedStorage* fa = archiveFor(*it);
        fa->clear();
        fa->commit();
        // FIXME: delete file (should be 0 in size now)
    }
    d->storage->RemoveAll();

}

void Akregator::Backend::StorageMK4Impl::storeFeedList(const QString& opmlStr)
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
    markDirty();
}

QString Akregator::Backend::StorageMK4Impl::restoreFeedList() const
{
    if  (d->feedListView.GetSize() == 0)
        return "";

    c4_Row row = d->feedListView.GetAt(0);
    return QString::fromUtf8(d->pFeedList(row));
}

void Akregator::Backend::StorageMK4Impl::storeTagSet(const QString& xmlStr)
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
    markDirty();
}

QString Akregator::Backend::StorageMK4Impl::restoreTagSet() const
{
    if  (d->feedListView.GetSize() == 0)
        return "";

    c4_Row row = d->feedListView.GetAt(0);
    return QString::fromUtf8(d->pTagSet(row));
}


#include "storagemk4impl.moc"
