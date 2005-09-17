/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

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

#include <qmap.h>
#include <QList>

#include <klocale.h>
#include <kstaticdeleter.h>

#include <libkdepim/progressmanager.h>

#include "feedlist.h"
#include "feed.h"
#include "treenode.h"

#include "progressmanager.h"

//#include <kdebug.h>

namespace Akregator {

class ProgressManager::ProgressManagerPrivate
{
    public:
        FeedList* feedList;
        QMap<Feed*, ProgressItemHandler*> handlers;
    
};

static KStaticDeleter<ProgressManager> progressmanagersd;
ProgressManager* ProgressManager::m_self = 0;

ProgressManager* ProgressManager::self()
{
    if (!m_self)
        m_self = progressmanagersd.setObject(m_self, new ProgressManager);
    return m_self;
}

ProgressManager::ProgressManager() : d(new ProgressManagerPrivate)
{
    d->feedList = 0;
}

ProgressManager::~ProgressManager()
{
    delete d; 
    d = 0;
}

void ProgressManager::setFeedList(FeedList* feedList)
{
    if (feedList == d->feedList)
        return;

    if (d->feedList != 0)
    {
        for (QMap<Feed*, ProgressItemHandler*>::ConstIterator it = d->handlers.begin(); it != d->handlers.end(); ++it)
            delete *it;
        d->handlers.clear();
        
        disconnect(feedList, SIGNAL(signalNodeAdded(TreeNode*)), this, SLOT(slotNodeAdded(TreeNode*)));
        disconnect(feedList, SIGNAL(signalNodeRemoved(TreeNode*)), this, SLOT(slotNodeRemoved(TreeNode*)));
    }

    d->feedList = feedList;
    
    if (feedList != 0)
    {
        QList<TreeNode*> list = feedList->asFlatList();
    
        for (QList<TreeNode*>::ConstIterator it = list.begin(); it != list.end(); ++it)
        slotNodeAdded(*it);
        connect(feedList, SIGNAL(signalNodeAdded(TreeNode*)), this, SLOT(slotNodeAdded(TreeNode*)));
        connect(feedList, SIGNAL(signalNodeRemoved(TreeNode*)), this, SLOT(slotNodeRemoved(TreeNode*)));
    }
}
     
void ProgressManager::slotNodeAdded(TreeNode* node)
{
    Feed* feed = dynamic_cast<Feed*>(node);
    if (feed)
    {
        if (!d->handlers.contains(feed))
        d->handlers[feed] = new ProgressItemHandler(feed);
        connect(feed, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotNodeDestroyed(TreeNode*)));
    }
}

void ProgressManager::slotNodeRemoved(TreeNode* node)
{
    Feed* feed = dynamic_cast<Feed*>(node);
    if (feed)
    {
        disconnect(feed, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotNodeDestroyed(TreeNode*)));
        delete d->handlers[feed];
        d->handlers.remove(feed);
    }
}

void ProgressManager::slotNodeDestroyed(TreeNode* node)
{
    Feed* feed = dynamic_cast<Feed*>(node);
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
    
    connect(feed, SIGNAL(fetchStarted(Feed*)), this, SLOT(slotFetchStarted()));
    connect(feed, SIGNAL(fetched(Feed*)), this, SLOT(slotFetchCompleted()));
    connect(feed, SIGNAL(fetchError(Feed*)), this, SLOT(slotFetchError()));
    connect(feed, SIGNAL(fetchAborted(Feed*)), this, SLOT(slotFetchAborted()));
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
    
    d->progressItem = KPIM::ProgressManager::createProgressItem(KPIM::ProgressManager::getUniqueID(), d->feed->title(), QString::null, true);

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
