/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#pragma once

#include "feed.h"

namespace Akregator
{
class MinifluxClient;

/**
 * A Feed subclass that fetches articles via the Miniflux API
 * instead of directly downloading the RSS/Atom file.
 */
class MinifluxFeed : public Feed
{
    Q_OBJECT
public:
    explicit MinifluxFeed(int minifluxFeedId, const QString &feedTitle, MinifluxClient *client, Akregator::Backend::Storage *storage);
    ~MinifluxFeed() override;

    [[nodiscard]] int minifluxFeedId() const;

    /** Miniflux feeds are managed entirely by the plugin; skip OPML persistence. */
    QDomElement toOPML(QDomElement parent, QDomDocument document) const override;

protected:
    void tryFetch() override;

private:
    int m_minifluxFeedId;
    MinifluxClient *m_client; // not owned
};

} // namespace Akregator
