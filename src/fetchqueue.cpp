/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Sashmit Bhaduri <smt@vfemail.net>
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#include "fetchqueue.h"
#include "akregatorconfig.h"
#include "feed.h"
#include "treenode.h"

using namespace Akregator;

FetchQueue::FetchQueue(QObject *parent)
    : QObject(parent)
{
}

FetchQueue::~FetchQueue()
{
    slotAbort();
}

void FetchQueue::slotAbort()
{
    for (Feed *const i : std::as_const(m_fetchingFeeds)) {
        disconnectFromFeed(i);
        i->slotAbortFetch();
    }
    m_fetchingFeeds.clear();

    for (Feed *const i : std::as_const(m_queuedFeeds)) {
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

#include "moc_fetchqueue.cpp"
