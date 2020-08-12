/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef AKREGATOR_PROGRESSMANAGER_H
#define AKREGATOR_PROGRESSMANAGER_H

#include "feedlist.h"
#include <QObject>
#include <QSharedPointer>
#include <QHash>

namespace KPIM {
class ProgressItem;
}

namespace Akregator {
class Feed;
class ProgressItemHandler;
class TreeNode;

/** This class manages the progress items for all feeds */

class ProgressManager : public QObject
{
    Q_OBJECT
public:

    static ProgressManager *self();

    ProgressManager();
    ~ProgressManager();

    /** sets the feed list to be managed */
    void setFeedList(const QSharedPointer<FeedList> &feedList);

protected Q_SLOTS:

    void slotNodeAdded(Akregator::TreeNode *node);
    void slotNodeRemoved(Akregator::TreeNode *node);
    void slotNodeDestroyed(Akregator::TreeNode *node);

private:

    static ProgressManager *m_self;
    QSharedPointer<FeedList> m_feedList;
    QHash<Feed *, ProgressItemHandler *> m_handlers;
};

/** this class handles the creation and deletion of progress items for one feed.
    This is an internal class intended to be used in ProgressManager only */

class ProgressItemHandler : public QObject
{
    Q_OBJECT
public:
    explicit ProgressItemHandler(Feed *feed);
    ~ProgressItemHandler();

public Q_SLOTS:

    void slotFetchStarted();
    void slotFetchCompleted();
    void slotFetchAborted();
    void slotFetchError();

private:
    Feed *m_feed = nullptr;
    KPIM::ProgressItem *m_progressItem = nullptr;
};
} // namespace Akregator

#endif // AKREGATOR_PROGRESSMANAGER_H
