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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

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
        StorageMK4ImplPrivate() : modified(false), purl("url"), punread("unread"), ptotalCount("totalCount"), plastFetch("lastFetch") {}
        c4_Storage* m_storage;
        c4_View m_archiveView;
        bool m_autoCommit;
	bool modified;
        QMap<QString, FeedStorage*> feeds;
        c4_StringProp purl;
        c4_IntProp punread, ptotalCount, plastFetch;
        QTimer* commitTimer;
};

StorageMK4Impl::StorageMK4Impl() : d(new StorageMK4ImplPrivate)
{
    // commit changes every 3 seconds
    d->commitTimer = new QTimer(this);
    connect(d->commitTimer, SIGNAL(timeout()), this, SLOT(slotCommit()));
    d->commitTimer->start(3000); 
    open(true);
}

StorageMK4Impl::~StorageMK4Impl()
{
    close();
    delete d; d = 0;
}
void StorageMK4Impl::initialize(const QStringList&) {}

bool StorageMK4Impl::open(bool autoCommit)
{
    QString filePath = KGlobal::dirs()->saveLocation("data", "akregator")+"/Archive/archiveindex.mk4";
    d->m_storage = new c4_Storage(filePath.local8Bit(), true);
    d->m_archiveView = d->m_storage->GetAs("archive[url:S,unread:I,totalCount:I,lastFetch:I]");
    c4_View hash = d->m_storage->GetAs("archiveHash[_H:I,_R:I]");
    d->m_archiveView = d->m_archiveView.Hash(hash, 1); // hash on url
    d->m_autoCommit = autoCommit;
    return true;
}

bool StorageMK4Impl::autoCommit() const
{
    return d->m_autoCommit;
}

bool StorageMK4Impl::close()
{
    d->commitTimer->stop();

    QMap<QString, FeedStorage*>::Iterator it;
    for (it = d->feeds.begin(); it != d->feeds.end(); ++it)
    {
        it.data()->close();
        delete it.data();
    }
    if(d->m_autoCommit)
        d->m_storage->Commit();
    delete d->m_storage; d->m_storage = 0;
    return true;
}

bool StorageMK4Impl::commit()
{
    QMap<QString, FeedStorage*>::Iterator it;
    for ( it = d->feeds.begin(); it != d->feeds.end(); ++it )
        it.data()->commit();

    if(d->m_storage)
    {
        d->m_storage->Commit();
        return true;
    }

    return false;
}

bool StorageMK4Impl::rollback()
{
    QMap<QString, FeedStorage*>::Iterator it;
    for ( it = d->feeds.begin(); it != d->feeds.end(); ++it )
        it.data()->rollback();

    if(d->m_storage)
    {
        d->m_storage->Rollback();
        return true;
    }
    return false;
}

int StorageMK4Impl::unreadFor(const QString &url)
{
    c4_Row findrow;
    d->purl(findrow) = url.ascii();
    int findidx = d->m_archiveView.Find(findrow);
    
    return findidx != -1 ? d->punread(d->m_archiveView.GetAt(findidx)) : 0;
}

void StorageMK4Impl::setUnreadFor(const QString &url, int unread)
{
    c4_Row findrow;
    d->purl(findrow) = url.ascii();
    int findidx = d->m_archiveView.Find(findrow);
    if (findidx == -1)
        return;
    findrow = d->m_archiveView.GetAt(findidx);
    d->punread(findrow) = unread;
    d->m_archiveView.SetAt(findidx, findrow);
    d->modified = true;
}

int StorageMK4Impl::totalCountFor(const QString &url)
{
    c4_Row findrow;
    d->purl(findrow) = url.ascii();
    int findidx = d->m_archiveView.Find(findrow);
    
    return findidx != -1 ? d->ptotalCount(d->m_archiveView.GetAt(findidx)) : 0;
}

void StorageMK4Impl::setTotalCountFor(const QString &url, int total)
{
    c4_Row findrow;
    d->purl(findrow) = url.ascii();
    int findidx = d->m_archiveView.Find(findrow);
    if (findidx == -1)
        return;
    findrow = d->m_archiveView.GetAt(findidx);
    d->ptotalCount(findrow) = total;
    d->m_archiveView.SetAt(findidx, findrow);
    d->modified = true;
}

int StorageMK4Impl::lastFetchFor(const QString& url)
{
    c4_Row findrow;
    d->purl(findrow) = url.ascii();
    int findidx = d->m_archiveView.Find(findrow);
    
    return (findidx != -1 ? d->plastFetch(d->m_archiveView.GetAt(findidx)) : 0);
}

void StorageMK4Impl::setLastFetchFor(const QString& url, int lastFetch)
{
    c4_Row findrow;
    d->purl(findrow) = url.ascii();
    int findidx = d->m_archiveView.Find(findrow);
    if (findidx == -1)
        return;
    findrow = d->m_archiveView.GetAt(findidx);
    d->plastFetch(findrow) = lastFetch;
    d->m_archiveView.SetAt(findidx, findrow);
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
        d->purl(findrow) = url.ascii();
        int findidx = d->m_archiveView.Find(findrow);
        if (findidx == -1)
        {
            d->punread(findrow) = 0;
	    d->ptotalCount(findrow) = 0;
	    d->plastFetch(findrow) = 0;
            d->m_archiveView.Add(findrow);
            d->modified = true;
        }
        fs->convertOldArchive();
    }
    return d->feeds[url];
}

}
}

#include "storagemk4impl.moc"
