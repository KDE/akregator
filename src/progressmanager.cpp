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

using namespace Akregator;

ProgressManager *ProgressManager::m_self = nullptr;

ProgressManager *ProgressManager::self()
{
    static ProgressManager sself;
    if (!m_self) {
        m_self = &sself;
    }
    return m_self;
}

ProgressManager::ProgressManager()
{
}

ProgressManager::~ProgressManager()
{
}

void ProgressManager::setFeedList(const QSharedPointer<FeedList> &feedList)
{
    if (feedList == m_feedList) {
        return;
    }

    if (m_feedList) {
        qDeleteAll(m_handlers);
        m_handlers.clear();
        m_feedList->disconnect(this);
    }

    m_feedList = feedList;

    if (m_feedList) {
        const QVector<Feed *> list = feedList->feeds();

        for (TreeNode *i : list) {
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

    if (m_handlers.contains(feed)) {
        return;
    }

    m_handlers[feed] = new ProgressItemHandler(feed);
    connect(feed, &TreeNode::signalDestroyed, this, &ProgressManager::slotNodeDestroyed);
}

void ProgressManager::slotNodeRemoved(TreeNode *node)
{
    Feed *feed = qobject_cast<Feed *>(node);
    if (feed) {
        feed->disconnect(this);
        delete m_handlers[feed];
        m_handlers.remove(feed);
    }
}

void ProgressManager::slotNodeDestroyed(TreeNode *node)
{
    Feed *feed = qobject_cast<Feed *>(node);
    if (feed) {
        delete m_handlers[feed];
        m_handlers.remove(feed);
    }
}

ProgressItemHandler::ProgressItemHandler(Feed *feed)
{
    m_feed = feed;
    m_progressItem = nullptr;

    connect(feed, &Feed::fetchStarted, this, &ProgressItemHandler::slotFetchStarted);
    connect(feed, &Feed::fetched, this, &ProgressItemHandler::slotFetchCompleted);
    connect(feed, &Feed::fetchError, this, &ProgressItemHandler::slotFetchError);
    connect(feed, &Feed::fetchAborted, this, &ProgressItemHandler::slotFetchAborted);
}

ProgressItemHandler::~ProgressItemHandler()
{
    if (m_progressItem) {
        m_progressItem->setComplete();
        m_progressItem = nullptr;
    }
}

void ProgressItemHandler::slotFetchStarted()
{
    if (m_progressItem) {
        m_progressItem->setComplete();
        m_progressItem = nullptr;
    }

    m_progressItem = KPIM::ProgressManager::createProgressItem(KPIM::ProgressManager::getUniqueID(), m_feed->title(), QString(), true);

    connect(m_progressItem, &KPIM::ProgressItem::progressItemCanceled, m_feed, &Feed::slotAbortFetch);
}

void ProgressItemHandler::slotFetchCompleted()
{
    if (m_progressItem) {
        m_progressItem->setStatus(i18n("Fetch completed"));
        m_progressItem->setComplete();
        m_progressItem = nullptr;
    }
}

void ProgressItemHandler::slotFetchError()
{
    if (m_progressItem) {
        m_progressItem->setStatus(i18n("Fetch error"));
        m_progressItem->setComplete();
        m_progressItem = nullptr;
    }
}

void ProgressItemHandler::slotFetchAborted()
{
    if (m_progressItem) {
        m_progressItem->setStatus(i18n("Fetch aborted"));
        m_progressItem->setComplete();
        m_progressItem = nullptr;
    }
}
