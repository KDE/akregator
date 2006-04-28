/*
    This file is part of Akregator.

    Copyright (C) 2004 Sashmit Bhaduri <smt@vfemail.net>
                  2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

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

#include <QList>

#include "akregatorconfig.h"
#include "fetchqueue.h"
#include "feed.h"
#include "treenode.h"


namespace Akregator {

class FetchQueue::FetchQueuePrivate
{
    public:
    
        QList<Feed*> queuedFeeds;
        QList<Feed*> fetchingFeeds;
};


FetchQueue::FetchQueue(QObject* parent, const char* name): QObject(parent), d(new FetchQueuePrivate) {
  setObjectName( name );
}


FetchQueue::~FetchQueue()
{
    slotAbort();
    delete d;
    d = 0;
}

void FetchQueue::slotAbort()
{
    for (QList<Feed*>::Iterator it = d->fetchingFeeds.begin(); it != d->fetchingFeeds.end(); ++it)
    {
        disconnectFromFeed(*it);
        (*it)->slotAbortFetch();
    }
    d->fetchingFeeds.clear();

    for (QList<Feed*>::Iterator it = d->queuedFeeds.begin(); it != d->queuedFeeds.end(); ++it)
    {
        disconnectFromFeed(*it);
    }
    d->queuedFeeds.clear();
    
    emit signalStopped();
}

void FetchQueue::addFeed(Feed *f)
{
    if (!d->queuedFeeds.contains(f) && !d->fetchingFeeds.contains(f))
    {
        connectToFeed(f);
        d->queuedFeeds.append(f);
        fetchNextFeed();
    }
}

void FetchQueue::fetchNextFeed()
{
    if (!d->queuedFeeds.isEmpty() && d->fetchingFeeds.count() < Settings::concurrentFetches())
    {
        if (d->fetchingFeeds.isEmpty() && d->queuedFeeds.count() == 1)
            emit signalStarted();
        Feed* f = *(d->queuedFeeds.begin());
        d->queuedFeeds.pop_front();
        d->fetchingFeeds.append(f);
        f->fetch(false);
        
    }
}

void FetchQueue::slotFeedFetched(Feed *f)
{
    emit fetched(f);
    feedDone(f);
}

void FetchQueue::slotFetchError(Feed *f)
{
    emit fetchError(f);
    feedDone(f);
}

void FetchQueue::slotFetchAborted(Feed *f)
{
    emit fetched(f); // FIXME: better use a signal like signalAborted(Feed*)
    feedDone(f);
}

bool FetchQueue::isEmpty() const
{
    return d->queuedFeeds.isEmpty() && d->fetchingFeeds.isEmpty();
}

void FetchQueue::feedDone(Feed *f)
{
    disconnectFromFeed(f);
    d->fetchingFeeds.removeAll(f);
    if (isEmpty())
        emit signalStopped();
    else    
        fetchNextFeed();
}

void FetchQueue::connectToFeed(Feed* feed)
{
    connect (feed, SIGNAL(fetched(Feed*)), this, SLOT(slotFeedFetched(Feed*)));
    connect (feed, SIGNAL(fetchError(Feed*)), this, SLOT(slotFetchError(Feed*)));
    connect (feed, SIGNAL(fetchAborted(Feed*)), this, SLOT(slotFetchAborted(Feed*)));
    connect (feed, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotNodeDestroyed(TreeNode*)));
}

void FetchQueue::disconnectFromFeed(Feed* feed)
{
    disconnect (feed, SIGNAL(fetched(Feed*)), this, SLOT(slotFeedFetched(Feed*)));
    disconnect (feed, SIGNAL(fetchError(Feed*)), this, SLOT(slotFetchError(Feed*)));
    disconnect (feed, SIGNAL(fetchAborted(Feed*)), this, SLOT(slotFetchAborted(Feed*)));
    disconnect (feed, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotNodeDestroyed(TreeNode*)));
}


void FetchQueue::slotNodeDestroyed(TreeNode* node)
{
    Feed* feed = dynamic_cast<Feed*> (node);

    if (feed)
    {
        d->fetchingFeeds.removeAll(feed);
        d->queuedFeeds.removeAll(feed);
    }
}

} // namespace Akregator

#include "fetchqueue.moc"
