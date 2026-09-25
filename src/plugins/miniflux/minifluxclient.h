/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#pragma once

#include "minifluxdata.h"
#include <QObject>
#include <QUrl>
#include <QUrlQuery>
class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;
namespace Akregator
{

/**
 * REST client for the Miniflux API.
 * All methods are async; results are delivered via signals.
 */
class MinifluxClient : public QObject
{
    Q_OBJECT
public:
    explicit MinifluxClient(const QUrl &serverUrl, const QString &apiToken, QObject *parent = nullptr);
    ~MinifluxClient() override;

    void setCredentials(const QUrl &serverUrl, const QString &apiToken);

    /** GET /v1/me — verify credentials */
    void verifyCredentials();

    /** GET /v1/categories */
    void fetchCategories();

    /** GET /v1/feeds */
    void fetchFeeds();

    /** GET /v1/feeds/{feedId}/entries?status=unread&limit=N&offset=N, newest first.
     *  Emits entriesFetched() on success and entriesFetchError() on failure. */
    void fetchEntriesForFeed(int feedId, const QString &status = QStringLiteral("unread"), int offset = 0, int limit = 1000);

    /** GET /v1/entries?status=unread&limit=100&offset=N (paginated) */
    void fetchAllEntries(const QString &status = QStringLiteral("unread"), int offset = 0, qint64 sinceId = 0);

    /** PUT /v1/entries {"entry_ids":[…],"status":"read"} */
    void markEntriesRead(const QList<qint64> &entryIds);

    /** PUT /v1/entries {"entry_ids":[…],"status":"unread"} */
    void markEntriesUnread(const QList<qint64> &entryIds);

    /** PUT /v1/entries/{id}/bookmark */
    void toggleBookmark(qint64 entryId);

    /** PUT /v1/feeds/{id}/refresh */
    void refreshFeed(int feedId);

Q_SIGNALS:
    void credentialsVerified(bool ok, const QString &errorMessage);
    void categoriesFetched(const QList<MinifluxCategory> &categories);
    void feedsFetched(const QList<MinifluxFeedData> &feeds);
    void entriesFetched(int feedId, const QList<MinifluxEntry> &entries, int total);
    /** Emitted instead of networkError() when fetchEntriesForFeed() fails, so that
     *  concurrent per-feed fetches on the shared client do not abort each other. */
    void entriesFetchError(int feedId, const QString &message);
    void allEntriesFetched(const QList<MinifluxEntry> &entries, int total, int offset);
    void entriesStatusUpdated(bool ok);
    void bookmarkToggled(bool ok);
    void feedRefreshed(bool ok);
    void networkError(const QString &message);

private:
    QNetworkReply *makeGet(const QString &path, const QUrlQuery &query = QUrlQuery());
    QNetworkReply *makePut(const QString &path, const QByteArray &body);
    void setAuthHeader(QNetworkRequest &request) const;

    QNetworkAccessManager *const m_nam;
    QUrl m_serverUrl;
    QString m_apiToken;
};

} // namespace Akregator
