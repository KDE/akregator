/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#include "minifluxsyncjob.h"
#include "minifluxclient.h"

using namespace Akregator;

MinifluxSyncJob::MinifluxSyncJob(MinifluxClient *client, QObject *parent)
    : KJob(parent)
    , m_client(client)
{
}

MinifluxSyncJob::~MinifluxSyncJob() = default;

void MinifluxSyncJob::start()
{
    connect(m_client, &MinifluxClient::categoriesFetched, this, &MinifluxSyncJob::onCategoriesFetched);
    connect(m_client, &MinifluxClient::feedsFetched, this, &MinifluxSyncJob::onFeedsFetched);
    connect(m_client, &MinifluxClient::networkError, this, &MinifluxSyncJob::onNetworkError);
    m_client->fetchCategories();
    m_client->fetchFeeds();
}

void MinifluxSyncJob::onCategoriesFetched(const QList<MinifluxCategory> &categories)
{
    m_categories = categories;
    m_categoriesDone = true;
    if (m_feedsDone) {
        emitResult();
    }
}

void MinifluxSyncJob::onFeedsFetched(const QList<MinifluxFeedData> &feeds)
{
    m_feeds = feeds;
    m_feedsDone = true;
    if (m_categoriesDone) {
        emitResult();
    }
}

void MinifluxSyncJob::onNetworkError(const QString &message)
{
    setError(KJob::UserDefinedError);
    setErrorText(message);
    emitResult();
}

QList<MinifluxCategory> MinifluxSyncJob::categories() const
{
    return m_categories;
}

QList<MinifluxFeedData> MinifluxSyncJob::feeds() const
{
    return m_feeds;
}

#include "moc_minifluxsyncjob.cpp"
