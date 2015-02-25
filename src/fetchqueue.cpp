/*
    This file is part of Akregator.

    Copyright (C) 2004 Sashmit Bhaduri <smt@vfemail.net>
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

#include "fetchqueue.h"
#include "akregatorconfig.h"
#include "feed.h"
#include "treenode.h"

#include <QList>

#include <cassert>

using namespace Akregator;

class FetchQueue::FetchQueuePrivate
{
public:

    QList<Feed *> queuedFeeds;
    QList<Feed *> fetchingFeeds;
};

FetchQueue::FetchQueue(QObject *parent): QObject(parent), d(new FetchQueuePrivate)
{
}

FetchQueue::~FetchQueue()
{
    slotAbort();
    delete d;
    d = 0;
}

void FetchQueue::slotAbort()
{
    foreach (Feed *const i, d->fetchingFeeds) {
        disconnectFromFeed(i);
        i->slotAbortFetch();
    }
    d->fetchingFeeds.clear();

    foreach (Feed *const i, d->queuedFeeds) {
        disconnectFromFeed(i);
    }
    d->queuedFeeds.clear();

    emit signalStopped();
}

void FetchQueue::addFeed(Feed *f)
{
    if (!d->queuedFeeds.contains(f) && !d->fetchingFeeds.contains(f)) {
        connectToFeed(f);
        d->queuedFeeds.append(f);
        fetchNextFeed();
    }
}

void FetchQueue::fetchNextFeed()
{
    if (!d->queuedFeeds.isEmpty() && d->fetchingFeeds.count() < Settings::concurrentFetches()) {
        if (d->fetchingFeeds.isEmpty() && d->queuedFeeds.count() == 1) {
            emit signalStarted();
        }
        Feed *f = *(d->queuedFeeds.begin());
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
    if (isEmpty()) {
        emit signalStopped();
    } else {
        fetchNextFeed();
    }
}

void FetchQueue::connectToFeed(Feed *feed)
{
    connect(feed, SIGNAL(fetched(Akregator::Feed*)), this, SLOT(slotFeedFetched(Akregator::Feed*)));
    connect(feed, SIGNAL(fetchError(Akregator::Feed*)), this, SLOT(slotFetchError(Akregator::Feed*)));
    connect(feed, SIGNAL(fetchAborted(Akregator::Feed*)), this, SLOT(slotFetchAborted(Akregator::Feed*)));
    connect(feed, SIGNAL(signalDestroyed(Akregator::TreeNode*)), this, SLOT(slotNodeDestroyed(Akregator::TreeNode*)));
}

void FetchQueue::disconnectFromFeed(Feed *feed)
{
    feed->disconnect(this);
}

void FetchQueue::slotNodeDestroyed(TreeNode *node)
{
    Feed *const feed = qobject_cast<Feed *>(node);
    Q_ASSERT(feed);

    d->fetchingFeeds.removeAll(feed);
    d->queuedFeeds.removeAll(feed);
}

