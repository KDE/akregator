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
#include <QList>

#include <klocale.h>

#include <libkdepim/progressmanager.h>

using namespace boost;

namespace Akregator {

class ProgressManager::ProgressManagerPrivate
{
    public:
        ProgressManagerPrivate() : feedList() {}
        shared_ptr<FeedList> feedList;
        QHash<Feed*, ProgressItemHandler*> handlers;

};

ProgressManager* ProgressManager::m_self = 0;

ProgressManager* ProgressManager::self()
{
    static ProgressManager sself;
    if (!m_self)
        m_self = &sself;
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

void ProgressManager::setFeedList(const shared_ptr<FeedList>& feedList)
{
    if ( feedList == d->feedList )
        return;

    if ( d->feedList )
    {
        qDeleteAll( d->handlers );
        d->handlers.clear();
        d->feedList->disconnect( this );
    }

    d->feedList = feedList;

    if ( d->feedList )
    {
        QVector<Feed*> list = feedList->feeds();

        foreach( TreeNode* i, list )
            slotNodeAdded( i );
        connect(feedList.get(), SIGNAL(signalNodeAdded(Akregator::TreeNode*)), this, SLOT(slotNodeAdded(Akregator::TreeNode*)));
        connect(feedList.get(), SIGNAL(signalNodeRemoved(Akregator::TreeNode*)), this, SLOT(slotNodeRemoved(Akregator::TreeNode*)));
    }
}

void ProgressManager::slotNodeAdded(TreeNode* node)
{
    Feed* const feed = qobject_cast<Feed*>(node);
    if (!feed)
        return;

    if ( d->handlers.contains( feed ) )
        return;

    d->handlers[feed] = new ProgressItemHandler(feed);
    connect(feed, SIGNAL(signalDestroyed(Akregator::TreeNode*)), this, SLOT(slotNodeDestroyed(Akregator::TreeNode*)));
}

void ProgressManager::slotNodeRemoved(TreeNode* node)
{
    Feed* feed = qobject_cast<Feed*>(node);
    if (feed)
    {
        feed->disconnect( this );
        delete d->handlers[feed];
        d->handlers.remove(feed);
    }
}

void ProgressManager::slotNodeDestroyed(TreeNode* node)
{
    Feed* feed = qobject_cast<Feed*>(node);
    if (feed)
    {
        delete d->handlers[feed];
        d->handlers.remove(feed);
    }
}

class ProgressItemHandler::ProgressItemHandlerPrivate
{
    public:

        Feed* feed;
        KPIM::ProgressItem* progressItem;
};

ProgressItemHandler::ProgressItemHandler(Feed* feed) : d(new ProgressItemHandlerPrivate)
{
    d->feed = feed;
    d->progressItem = 0;

    connect(feed, SIGNAL(fetchStarted(Akregator::Feed*)), this, SLOT(slotFetchStarted()));
    connect(feed, SIGNAL(fetched(Akregator::Feed*)), this, SLOT(slotFetchCompleted()));
    connect(feed, SIGNAL(fetchError(Akregator::Feed*)), this, SLOT(slotFetchError()));
    connect(feed, SIGNAL(fetchAborted(Akregator::Feed*)), this, SLOT(slotFetchAborted()));
}

ProgressItemHandler::~ProgressItemHandler()
{
    if (d->progressItem)
    {
        d->progressItem->setComplete();
        d->progressItem = 0;
    }

    delete d;
    d = 0;
}

void ProgressItemHandler::slotFetchStarted()
{
    if (d->progressItem)
    {
        d->progressItem->setComplete();
        d->progressItem = 0;
    }

    d->progressItem = KPIM::ProgressManager::createProgressItem(KPIM::ProgressManager::getUniqueID(), d->feed->title(), QString(), true);

    connect(d->progressItem, SIGNAL(progressItemCanceled(KPIM::ProgressItem*)), d->feed, SLOT(slotAbortFetch()));
}


void ProgressItemHandler::slotFetchCompleted()
{
    if (d->progressItem)
    {
        d->progressItem->setStatus(i18n("Fetch completed"));
        d->progressItem->setComplete();
        d->progressItem = 0;
    }
}

void ProgressItemHandler::slotFetchError()
{
    if (d->progressItem)
    {
        d->progressItem->setStatus(i18n("Fetch error"));
        d->progressItem->setComplete();
        d->progressItem = 0;
    }
}

void ProgressItemHandler::slotFetchAborted()
{
    if (d->progressItem)
    {
        d->progressItem->setStatus(i18n("Fetch aborted"));
        d->progressItem->setComplete();
        d->progressItem = 0;
    }
}

} // namespace Akregator

#include "progressmanager.moc"
