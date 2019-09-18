/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <osterfeld@kde.org>

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
