/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#include "progressmanager.h"
#include "feed.h"
#include "treenode.h"

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

ProgressManager::ProgressManager() = default;

ProgressManager::~ProgressManager() = default;

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
        const QList<Feed *> list = feedList->feeds();

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
    : m_feed(feed)
{
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

#include "moc_progressmanager.cpp"
