/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#pragma once

#include "minifluxclient.h"
#include <Syndication/DataRetriever>

namespace Akregator
{

/**
 * Syndication::DataRetriever that fetches articles from the Miniflux API
 * and returns them as RSS 2.0 XML. The feed URL passed to retrieveData()
 * is ignored; the feedId is used instead.
 *
 * Article GUIDs are encoded as miniflux://entry/{entryId} so that
 * MinifluxStatusSync can extract the entry ID for sync-back.
 */
class MinifluxRetriever : public Syndication::DataRetriever
{
    Q_OBJECT
public:
    explicit MinifluxRetriever(MinifluxClient *client, int feedId, const QString &feedTitle);
    ~MinifluxRetriever() override;

    void retrieveData(const QUrl &url) override;
    [[nodiscard]] int errorCode() const override;
    void abort() override;

private Q_SLOTS:
    void onEntriesFetched(int feedId, const QList<MinifluxEntry> &entries, int total);

private:
    static QByteArray entriesToRss(const QString &feedTitle, const QString &feedUrl, const QList<MinifluxEntry> &entries);

    MinifluxClient *m_client;
    int m_feedId;
    QString m_feedTitle;
    int m_errorCode = 0;
    bool m_aborted = false;
};

} // namespace Akregator
