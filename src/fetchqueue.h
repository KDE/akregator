/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Sashmit Bhaduri <smt@vfemail.net>
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "akregator_export.h"
#include <QObject>

namespace Akregator
{
class Feed;
class TreeNode;

class AKREGATOR_EXPORT FetchQueue : public QObject
{
    Q_OBJECT

public:
    explicit FetchQueue(QObject *parent = nullptr);
    ~FetchQueue();

    /** returns true when no feeds are neither fetching nor queued */
    Q_REQUIRED_RESULT bool isEmpty() const;

    /** adds a feed to the queue */
    void addFeed(Feed *f);

public Q_SLOTS:

    /** aborts currently fetching feeds and empties the queue */
    void slotAbort();

Q_SIGNALS:

    void signalStarted();
    void signalStopped();
    void fetched(Akregator::Feed *);
    void fetchError(Akregator::Feed *);

protected:
    /** fetches the next feed in the queue, unless the maximum of concurrent fetches is reached */
    void fetchNextFeed();

    void feedDone(Feed *f);
    void connectToFeed(Feed *feed);
    void disconnectFromFeed(Feed *feed);

protected Q_SLOTS:

    void slotNodeDestroyed(Akregator::TreeNode *node);
    void slotFeedFetched(Akregator::Feed *);
    void slotFetchError(Akregator::Feed *);
    void slotFetchAborted(Akregator::Feed *);

private:
    QList<Feed *> m_queuedFeeds;
    QList<Feed *> m_fetchingFeeds;
};
} // namespace Akregator

