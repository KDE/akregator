/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#include "minifluxclient.h"

#include "miniflux_debug.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>

using namespace Akregator;

MinifluxClient::MinifluxClient(const QUrl &serverUrl, const QString &apiToken, QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
    , m_serverUrl(serverUrl)
    , m_apiToken(apiToken)
{
}

MinifluxClient::~MinifluxClient() = default;

void MinifluxClient::setCredentials(const QUrl &serverUrl, const QString &apiToken)
{
    m_serverUrl = serverUrl;
    m_apiToken = apiToken;
}

void MinifluxClient::setAuthHeader(QNetworkRequest &request) const
{
    request.setRawHeader("X-Auth-Token", m_apiToken.toUtf8());
}

QNetworkReply *MinifluxClient::makeGet(const QString &path, const QUrlQuery &query)
{
    QUrl url = m_serverUrl;
    url.setPath(url.path() + path);
    if (!query.isEmpty()) {
        url.setQuery(query);
    }
    QNetworkRequest request(url);
    setAuthHeader(request);
    return m_nam->get(request);
}

QNetworkReply *MinifluxClient::makePut(const QString &path, const QByteArray &body)
{
    QUrl url = m_serverUrl;
    url.setPath(url.path() + path);
    QNetworkRequest request(url);
    setAuthHeader(request);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    return m_nam->put(request, body);
}

void MinifluxClient::verifyCredentials()
{
    auto reply = makeGet(QStringLiteral("/v1/me"));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            Q_EMIT credentialsVerified(false, reply->errorString());
            return;
        }
        const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        Q_EMIT credentialsVerified(statusCode == 200, statusCode == 200 ? QString() : QStringLiteral("HTTP %1").arg(statusCode));
    });
}

void MinifluxClient::fetchCategories()
{
    auto reply = makeGet(QStringLiteral("/v1/categories"));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            Q_EMIT networkError(reply->errorString());
            return;
        }
        const QJsonArray arr = QJsonDocument::fromJson(reply->readAll()).array();
        QList<MinifluxCategory> categories;
        categories.reserve(arr.size());
        for (const QJsonValue &v : arr) {
            const QJsonObject obj = v.toObject();
            MinifluxCategory cat;
            cat.id = obj.value(QLatin1StringView("id")).toInt();
            cat.title = obj.value(QLatin1StringView("title")).toString();
            categories.append(cat);
        }
        Q_EMIT categoriesFetched(categories);
    });
}

void MinifluxClient::fetchFeeds()
{
    auto reply = makeGet(QStringLiteral("/v1/feeds"));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            Q_EMIT networkError(reply->errorString());
            return;
        }
        const QJsonArray arr = QJsonDocument::fromJson(reply->readAll()).array();
        QList<MinifluxFeedData> feeds;
        feeds.reserve(arr.size());
        for (const QJsonValue &v : arr) {
            const QJsonObject obj = v.toObject();
            MinifluxFeedData feed;
            feed.id = obj.value(QLatin1StringView("id")).toInt();
            feed.categoryId = obj.value(QLatin1StringView("category")).toObject().value(QLatin1StringView("id")).toInt();
            feed.title = obj.value(QLatin1StringView("title")).toString();
            feed.feedUrl = obj.value(QLatin1StringView("feed_url")).toString();
            feed.siteUrl = obj.value(QLatin1StringView("site_url")).toString();
            feed.unreadCount = obj.value(QLatin1StringView("unread_count")).toInt();
            feeds.append(feed);
        }
        Q_EMIT feedsFetched(feeds);
    });
}

static MinifluxEntry parseEntry(const QJsonObject &obj)
{
    MinifluxEntry entry;
    entry.id = obj.value(QLatin1StringView("id")).toVariant().toLongLong();
    entry.feedId = obj.value(QLatin1StringView("feed_id")).toInt();
    entry.title = obj.value(QLatin1StringView("title")).toString();
    entry.url = obj.value(QLatin1StringView("url")).toString();
    entry.author = obj.value(QLatin1StringView("author")).toString();
    entry.content = obj.value(QLatin1StringView("content")).toString();
    entry.publishedAt = QDateTime::fromString(obj.value(QLatin1StringView("published_at")).toString(), Qt::ISODate);
    entry.status = obj.value(QLatin1StringView("status")).toString();
    entry.starred = obj.value(QLatin1StringView("starred")).toBool();
    return entry;
}

void MinifluxClient::fetchEntriesForFeed(int feedId, const QString &status, int offset, int limit)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("status"), status);
    query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    // Newest first, so that a capped fetch keeps the most recent entries.
    query.addQueryItem(QStringLiteral("order"), QStringLiteral("published_at"));
    query.addQueryItem(QStringLiteral("direction"), QStringLiteral("desc"));
    if (offset > 0) {
        query.addQueryItem(QStringLiteral("offset"), QString::number(offset));
    }
    auto reply = makeGet(QStringLiteral("/v1/feeds/%1/entries").arg(feedId), query);
    connect(reply, &QNetworkReply::finished, this, [this, reply, feedId]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            Q_EMIT entriesFetchError(feedId, reply->errorString());
            return;
        }
        const QJsonObject root = QJsonDocument::fromJson(reply->readAll()).object();
        const int total = root.value(QLatin1StringView("total")).toInt();
        const QJsonArray arr = root.value(QLatin1StringView("entries")).toArray();
        QList<MinifluxEntry> entries;
        entries.reserve(arr.size());
        for (const QJsonValue &v : arr) {
            entries.append(parseEntry(v.toObject()));
        }
        Q_EMIT entriesFetched(feedId, entries, total);
    });
}

void MinifluxClient::fetchAllEntries(const QString &status, int offset, qint64 sinceId)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("status"), status);
    query.addQueryItem(QStringLiteral("limit"), QStringLiteral("100"));
    if (offset > 0) {
        query.addQueryItem(QStringLiteral("offset"), QString::number(offset));
    }
    if (sinceId > 0) {
        query.addQueryItem(QStringLiteral("after_entry_id"), QString::number(sinceId));
    }
    auto reply = makeGet(QStringLiteral("/v1/entries"), query);
    connect(reply, &QNetworkReply::finished, this, [this, reply, offset]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            Q_EMIT networkError(reply->errorString());
            return;
        }
        const QJsonObject root = QJsonDocument::fromJson(reply->readAll()).object();
        const int total = root.value(QLatin1StringView("total")).toInt();
        const QJsonArray arr = root.value(QLatin1StringView("entries")).toArray();
        QList<MinifluxEntry> entries;
        entries.reserve(arr.size());
        for (const QJsonValue &v : arr) {
            entries.append(parseEntry(v.toObject()));
        }
        Q_EMIT allEntriesFetched(entries, total, offset);
    });
}

void MinifluxClient::markEntriesRead(const QList<qint64> &entryIds)
{
    QJsonArray ids;
    for (qint64 id : entryIds) {
        ids.append(id);
    }
    QJsonObject body;
    body[QStringLiteral("entry_ids")] = ids;
    body[QStringLiteral("status")] = QStringLiteral("read");
    const QByteArray payload = QJsonDocument(body).toJson(QJsonDocument::Compact);
    qCDebug(MINIFLUX_LOG) << "MinifluxClient::markEntriesRead: PUT /v1/entries" << payload;
    auto reply = makePut(QStringLiteral("/v1/entries"), payload);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        const bool ok = reply->error() == QNetworkReply::NoError;
        if (!ok) {
            qCWarning(MINIFLUX_LOG) << "MinifluxClient::markEntriesRead failed:" << reply->errorString();
        }
        Q_EMIT entriesStatusUpdated(ok);
    });
}

void MinifluxClient::markEntriesUnread(const QList<qint64> &entryIds)
{
    QJsonArray ids;
    for (qint64 id : entryIds) {
        ids.append(id);
    }
    QJsonObject body;
    body[QStringLiteral("entry_ids")] = ids;
    body[QStringLiteral("status")] = QStringLiteral("unread");
    auto reply = makePut(QStringLiteral("/v1/entries"), QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        Q_EMIT entriesStatusUpdated(reply->error() == QNetworkReply::NoError);
    });
}

void MinifluxClient::toggleBookmark(qint64 entryId)
{
    auto reply = makePut(QStringLiteral("/v1/entries/%1/bookmark").arg(entryId), {});
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        Q_EMIT bookmarkToggled(reply->error() == QNetworkReply::NoError);
    });
}

void MinifluxClient::refreshFeed(int feedId)
{
    auto reply = makePut(QStringLiteral("/v1/feeds/%1/refresh").arg(feedId), {});
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        Q_EMIT feedRefreshed(reply->error() == QNetworkReply::NoError);
    });
}

#include "moc_minifluxclient.cpp"
