/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#pragma once

#include "minifluxdata.h"
#include <KJob>
#include <QList>

namespace Akregator
{
class MinifluxClient;

/**
 * KJob that performs the initial sync from a Miniflux server:
 * fetches all categories and feeds, then emits the results.
 */
class MinifluxSyncJob : public KJob
{
    Q_OBJECT
public:
    explicit MinifluxSyncJob(MinifluxClient *client, QObject *parent = nullptr);
    ~MinifluxSyncJob() override;

    void start() override;

    [[nodiscard]] QList<MinifluxCategory> categories() const;
    [[nodiscard]] QList<MinifluxFeedData> feeds() const;

private:
    void onCategoriesFetched(const QList<MinifluxCategory> &categories);
    void onFeedsFetched(const QList<MinifluxFeedData> &feeds);
    void onNetworkError(const QString &message);

    MinifluxClient *const m_client;
    QList<MinifluxCategory> m_categories;
    QList<MinifluxFeedData> m_feeds;
    bool m_categoriesDone = false;
    bool m_feedsDone = false;
};

} // namespace Akregator
