/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#pragma once

#include "article.h"
#include <QObject>
#include <QSet>
#include <QTimer>

namespace Akregator
{
class Feed;
class MinifluxClient;
class TreeNode;

/**
 * Watches Miniflux-managed feeds for article status changes
 * (read/unread) and syncs them back to the Miniflux server
 * using a 2-second debounce timer.
 */
class MinifluxStatusSync : public QObject
{
    Q_OBJECT
public:
    explicit MinifluxStatusSync(MinifluxClient *client, QObject *parent = nullptr);
    ~MinifluxStatusSync() override;

    /** Start watching this feed for status changes. */
    void watchFeed(Feed *feed);

    /** Stop watching this feed. */
    void unwatchFeed(Feed *feed);

private Q_SLOTS:
    void onArticlesUpdated(Akregator::TreeNode *node, const QList<Akregator::Article> &articles);
    void flushPendingChanges();

private:
    static qint64 entryIdFromGuid(const QString &guid);

    MinifluxClient *m_client = nullptr;
    QTimer m_debounceTimer;
    QSet<qint64> m_pendingRead;
    QSet<qint64> m_pendingUnread;
    QSet<qint64> m_pendingBookmark;
};

} // namespace Akregator
