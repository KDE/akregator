/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#include "minifluxfeed.h"
#include "minifluxretriever.h"

#include <QDomDocument>

using namespace Akregator;

MinifluxFeed::MinifluxFeed(int minifluxFeedId, const QString &feedTitle, MinifluxClient *client, Backend::Storage *storage)
    : Feed(storage)
    , m_minifluxFeedId(minifluxFeedId)
    , m_client(client)
{
    setTitle(feedTitle);
    setXmlUrl(QStringLiteral("miniflux://feed/%1").arg(minifluxFeedId));
}

MinifluxFeed::~MinifluxFeed() = default;

int MinifluxFeed::minifluxFeedId() const
{
    return m_minifluxFeedId;
}

QDomElement MinifluxFeed::toOPML(QDomElement /*parent*/, QDomDocument /*document*/) const
{
    // Miniflux feeds are repopulated from the server on each start; skip OPML persistence.
    return {};
}

void MinifluxFeed::tryFetch()
{
    startFeedLoader(new MinifluxRetriever(m_client, m_minifluxFeedId, title()));
}

#include "moc_minifluxfeed.cpp"
