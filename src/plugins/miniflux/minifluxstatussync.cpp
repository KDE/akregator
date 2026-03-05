/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#include "minifluxstatussync.h"
#include "minifluxclient.h"

#include "article.h"
#include "feed.h"
#include "types.h"

#include "miniflux_debug.h"
#include <QUrl>

using namespace Akregator;

MinifluxStatusSync::MinifluxStatusSync(MinifluxClient *client, QObject *parent)
    : QObject(parent)
    , m_client(client)
{
    m_debounceTimer.setInterval(2000);
    m_debounceTimer.setSingleShot(true);
    connect(&m_debounceTimer, &QTimer::timeout, this, &MinifluxStatusSync::flushPendingChanges);
}

MinifluxStatusSync::~MinifluxStatusSync() = default;

void MinifluxStatusSync::watchFeed(Feed *feed)
{
    connect(feed, &TreeNode::signalArticlesUpdated, this, &MinifluxStatusSync::onArticlesUpdated, Qt::UniqueConnection);
}

void MinifluxStatusSync::unwatchFeed(Feed *feed)
{
    disconnect(feed, &TreeNode::signalArticlesUpdated, this, &MinifluxStatusSync::onArticlesUpdated);
}

void MinifluxStatusSync::onArticlesUpdated(TreeNode * /*node*/, const QList<Article> &articles)
{
    qCDebug(MINIFLUX_LOG) << "MinifluxStatusSync::onArticlesUpdated called with" << articles.size() << "articles";
    for (const Article &article : articles) {
        const QString guid = article.guid();
        if (!guid.startsWith(QLatin1StringView("miniflux://entry/"))) {
            continue;
        }
        const qint64 entryId = entryIdFromGuid(guid);
        if (entryId <= 0) {
            qCWarning(MINIFLUX_LOG) << "MinifluxStatusSync: failed to parse entry ID from GUID:" << guid;
            continue;
        }

        if (article.status() == Read) {
            qCDebug(MINIFLUX_LOG) << "MinifluxStatusSync: queuing entry" << entryId << "as read";
            m_pendingRead.insert(entryId);
            m_pendingUnread.remove(entryId);
        } else if (article.status() == Unread) {
            qCDebug(MINIFLUX_LOG) << "MinifluxStatusSync: queuing entry" << entryId << "as unread";
            m_pendingUnread.insert(entryId);
            m_pendingRead.remove(entryId);
        }

        if (article.keep()) {
            m_pendingBookmark.insert(entryId);
        }
    }

    m_debounceTimer.start();
}

void MinifluxStatusSync::flushPendingChanges()
{
    if (!m_pendingRead.isEmpty()) {
        qCDebug(MINIFLUX_LOG) << "MinifluxStatusSync: flushing" << m_pendingRead.size() << "read entries to server";
        m_client->markEntriesRead(m_pendingRead.values());
        m_pendingRead.clear();
    }
    if (!m_pendingUnread.isEmpty()) {
        qCDebug(MINIFLUX_LOG) << "MinifluxStatusSync: flushing" << m_pendingUnread.size() << "unread entries to server";
        m_client->markEntriesUnread(m_pendingUnread.values());
        m_pendingUnread.clear();
    }
    for (qint64 entryId : std::as_const(m_pendingBookmark)) {
        m_client->toggleBookmark(entryId);
    }
    m_pendingBookmark.clear();
}

qint64 MinifluxStatusSync::entryIdFromGuid(const QString &guid)
{
    // guid format: "miniflux://entry/12345"
    const QUrl url(guid);
    if (url.scheme() != QLatin1StringView("miniflux") || url.host() != QLatin1StringView("entry")) {
        return -1;
    }
    bool ok = false;
    const qint64 id = url.path().mid(1).toLongLong(&ok); // remove leading '/'
    return ok ? id : -1;
}

#include "moc_minifluxstatussync.cpp"
