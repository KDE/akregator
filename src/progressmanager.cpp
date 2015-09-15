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

#include "progressmanager.h"
#include "feed.h"
#include "treenode.h"

#include <QHash>

#include <KLocalizedString>

#include <Libkdepim/ProgressManager>

namespace Akregator
{

class ProgressManager::ProgressManagerPrivate
{
public:
    ProgressManagerPrivate() : feedList() {}
    QSharedPointer<FeedList> feedList;
    QHash<Feed *, ProgressItemHandler *> handlers;

};

ProgressManager *ProgressManager::m_self = 0;

ProgressManager *ProgressManager::self()
{
    static ProgressManager sself;
    if (!m_self) {
        m_self = &sself;
    }
    return m_self;
}

ProgressManager::ProgressManager() : d(new ProgressManagerPrivate)
{
}

ProgressManager::~ProgressManager()
{
    delete d;
    d = 0;
}

void ProgressManager::setFeedList(const QSharedPointer<FeedList> &feedList)
{
    if (feedList == d->feedList) {
        return;
    }

    if (d->feedList) {
        qDeleteAll(d->handlers);
        d->handlers.clear();
        d->feedList->disconnect(this);
    }

    d->feedList = feedList;

    if (d->feedList) {
        QVector<Feed *> list = feedList->feeds();

        foreach (TreeNode *i, list) {
            slotNodeAdded(i);
        }
        connect(feedList.data(), &FeedList::signalNodeAdded, this, &ProgressManager::slotNodeAdded);
        connect(feedList.data(), &FeedList::signalNodeRemoved, this, &ProgressManager::slotNodeRemoved);
    }
}

void ProgressManager::slotNodeAdded(TreeNode *node)
{
    Feed *const feed = qobject_cast<Feed *>(node);
    if (!feed) {
        return;
    }

    if (d->handlers.contains(feed)) {
        return;
    }

    d->handlers[feed] = new ProgressItemHandler(feed);
    connect(feed, &TreeNode::signalDestroyed, this, &ProgressManager::slotNodeDestroyed);
}

void ProgressManager::slotNodeRemoved(TreeNode *node)
{
    Feed *feed = qobject_cast<Feed *>(node);
    if (feed) {
        feed->disconnect(this);
        delete d->handlers[feed];
        d->handlers.remove(feed);
    }
}

void ProgressManager::slotNodeDestroyed(TreeNode *node)
{
    Feed *feed = qobject_cast<Feed *>(node);
    if (feed) {
        delete d->handlers[feed];
        d->handlers.remove(feed);
    }
}

class ProgressItemHandler::ProgressItemHandlerPrivate
{
public:

    Feed *feed;
    KPIM::ProgressItem *progressItem;
};

ProgressItemHandler::ProgressItemHandler(Feed *feed) : d(new ProgressItemHandlerPrivate)
{
    d->feed = feed;
    d->progressItem = 0;

    connect(feed, &Feed::fetchStarted, this, &ProgressItemHandler::slotFetchStarted);
    connect(feed, &Feed::fetched, this, &ProgressItemHandler::slotFetchCompleted);
    connect(feed, &Feed::fetchError, this, &ProgressItemHandler::slotFetchError);
    connect(feed, &Feed::fetchAborted, this, &ProgressItemHandler::slotFetchAborted);
}

ProgressItemHandler::~ProgressItemHandler()
{
    if (d->progressItem) {
        d->progressItem->setComplete();
        d->progressItem = 0;
    }

    delete d;
    d = 0;
}

void ProgressItemHandler::slotFetchStarted()
{
    if (d->progressItem) {
        d->progressItem->setComplete();
        d->progressItem = 0;
    }

    d->progressItem = KPIM::ProgressManager::createProgressItem(KPIM::ProgressManager::getUniqueID(), d->feed->title(), QString(), true);

    connect(d->progressItem, &KPIM::ProgressItem::progressItemCanceled, d->feed, &Feed::slotAbortFetch);
}

void ProgressItemHandler::slotFetchCompleted()
{
    if (d->progressItem) {
        d->progressItem->setStatus(i18n("Fetch completed"));
        d->progressItem->setComplete();
        d->progressItem = 0;
    }
}

void ProgressItemHandler::slotFetchError()
{
    if (d->progressItem) {
        d->progressItem->setStatus(i18n("Fetch error"));
        d->progressItem->setComplete();
        d->progressItem = 0;
    }
}

void ProgressItemHandler::slotFetchAborted()
{
    if (d->progressItem) {
        d->progressItem->setStatus(i18n("Fetch aborted"));
        d->progressItem->setComplete();
        d->progressItem = 0;
    }
}

} // namespace Akregator

