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

FetchQueue::FetchQueue(QObject *parent) : QObject(parent)
{
}

FetchQueue::~FetchQueue()
{
    slotAbort();
}

void FetchQueue::slotAbort()
{
    for (Feed *const i : qAsConst(m_fetchingFeeds)) {
        disconnectFromFeed(i);
        i->slotAbortFetch();
    }
    m_fetchingFeeds.clear();

    for (Feed *const i : qAsConst(m_queuedFeeds)) {
        disconnectFromFeed(i);
    }
    m_queuedFeeds.clear();

    Q_EMIT signalStopped();
}

void FetchQueue::addFeed(Feed *f)
{
    if (!m_queuedFeeds.contains(f) && !m_fetchingFeeds.contains(f)) {
        connectToFeed(f);
        m_queuedFeeds.append(f);
        fetchNextFeed();
    }
}

void FetchQueue::fetchNextFeed()
{
    if (!m_queuedFeeds.isEmpty() && m_fetchingFeeds.count() < Settings::concurrentFetches()) {
        if (m_fetchingFeeds.isEmpty() && m_queuedFeeds.count() == 1) {
            Q_EMIT signalStarted();
        }
        Feed *f = *(m_queuedFeeds.begin());
        m_queuedFeeds.pop_front();
        m_fetchingFeeds.append(f);
        f->fetch(false);
    }
}

void FetchQueue::slotFeedFetched(Feed *f)
{
    Q_EMIT fetched(f);
    feedDone(f);
}

void FetchQueue::slotFetchError(Feed *f)
{
    Q_EMIT fetchError(f);
    feedDone(f);
}

void FetchQueue::slotFetchAborted(Feed *f)
{
    Q_EMIT fetched(f); // FIXME: better use a signal like signalAborted(Feed*)
    feedDone(f);
}

bool FetchQueue::isEmpty() const
{
    return m_queuedFeeds.isEmpty() && m_fetchingFeeds.isEmpty();
}

void FetchQueue::feedDone(Feed *f)
{
    disconnectFromFeed(f);
    m_fetchingFeeds.removeAll(f);
    if (isEmpty()) {
        Q_EMIT signalStopped();
    } else {
        fetchNextFeed();
    }
}

void FetchQueue::connectToFeed(Feed *feed)
{
    connect(feed, &Feed::fetched, this, &FetchQueue::slotFeedFetched);
    connect(feed, &Feed::fetchError, this, &FetchQueue::slotFetchError);
    connect(feed, &Feed::fetchAborted, this, &FetchQueue::slotFetchAborted);
    connect(feed, &TreeNode::signalDestroyed, this, &FetchQueue::slotNodeDestroyed);
}

void FetchQueue::disconnectFromFeed(Feed *feed)
{
    feed->disconnect(this);
}

void FetchQueue::slotNodeDestroyed(TreeNode *node)
{
    Feed *const feed = qobject_cast<Feed *>(node);
    Q_ASSERT(feed);

    m_fetchingFeeds.removeAll(feed);
    m_queuedFeeds.removeAll(feed);
}
