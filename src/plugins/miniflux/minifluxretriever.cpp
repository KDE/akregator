/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#include "minifluxretriever.h"

#include <QDomDocument>

using namespace Akregator;

MinifluxRetriever::MinifluxRetriever(MinifluxClient *client, int feedId, const QString &feedTitle)
    : Syndication::DataRetriever()
    , m_client(client)
    , m_feedId(feedId)
    , m_feedTitle(feedTitle)
{
}

MinifluxRetriever::~MinifluxRetriever() = default;

void MinifluxRetriever::retrieveData(const QUrl & /*url*/)
{
    if (m_aborted) {
        return;
    }
    connect(m_client, &MinifluxClient::entriesFetched, this, &MinifluxRetriever::onEntriesFetched);
    connect(m_client, &MinifluxClient::entriesFetchError, this, &MinifluxRetriever::onEntriesFetchError);
    fetchPage(0);
}

void MinifluxRetriever::fetchPage(int offset)
{
    m_client->fetchEntriesForFeed(m_feedId, QStringLiteral("unread"), offset);
}

int MinifluxRetriever::errorCode() const
{
    return m_errorCode;
}

void MinifluxRetriever::abort()
{
    m_aborted = true;
}

void MinifluxRetriever::onEntriesFetched(int feedId, const QList<MinifluxEntry> &entries, int total)
{
    if (feedId != m_feedId || m_aborted) {
        return;
    }
    m_allEntries.append(entries);

    if (!entries.isEmpty() && m_allEntries.size() < total) {
        // More pages to fetch
        fetchPage(m_allEntries.size());
        return;
    }

    disconnect(m_client, &MinifluxClient::entriesFetched, this, &MinifluxRetriever::onEntriesFetched);
    disconnect(m_client, &MinifluxClient::entriesFetchError, this, &MinifluxRetriever::onEntriesFetchError);
    const QByteArray xml = entriesToRss(m_feedTitle, QString(), m_allEntries);
    Q_EMIT dataRetrieved(xml, true);
}

void MinifluxRetriever::onEntriesFetchError(int feedId, const QString & /*message*/)
{
    if (feedId != m_feedId || m_aborted) {
        return;
    }
    disconnect(m_client, &MinifluxClient::entriesFetched, this, &MinifluxRetriever::onEntriesFetched);
    disconnect(m_client, &MinifluxClient::entriesFetchError, this, &MinifluxRetriever::onEntriesFetchError);
    m_errorCode = 1;
    Q_EMIT dataRetrieved({}, false);
}

QByteArray MinifluxRetriever::entriesToRss(const QString &feedTitle, const QString &feedUrl, const QList<MinifluxEntry> &entries)
{
    QDomDocument doc;
    doc.appendChild(doc.createProcessingInstruction(QStringLiteral("xml"), QStringLiteral("version=\"1.0\" encoding=\"UTF-8\"")));

    QDomElement rss = doc.createElement(QStringLiteral("rss"));
    rss.setAttribute(QStringLiteral("version"), QStringLiteral("2.0"));
    doc.appendChild(rss);

    QDomElement channel = doc.createElement(QStringLiteral("channel"));
    rss.appendChild(channel);

    auto addElement = [&](QDomElement &parent, const QString &tag, const QString &text) {
        QDomElement el = doc.createElement(tag);
        el.appendChild(doc.createTextNode(text));
        parent.appendChild(el);
    };

    addElement(channel, QStringLiteral("title"), feedTitle);
    addElement(channel, QStringLiteral("link"), feedUrl);

    for (const MinifluxEntry &entry : entries) {
        QDomElement item = doc.createElement(QStringLiteral("item"));
        channel.appendChild(item);

        addElement(item, QStringLiteral("title"), entry.title);
        addElement(item, QStringLiteral("link"), entry.url);
        addElement(item, QStringLiteral("author"), entry.author);
        addElement(item, QStringLiteral("description"), entry.content);
        addElement(item, QStringLiteral("pubDate"), entry.publishedAt.toString(Qt::RFC2822Date));
        // GUID: miniflux://entry/{entryId} — used by MinifluxStatusSync
        addElement(item, QStringLiteral("guid"), QStringLiteral("miniflux://entry/%1").arg(entry.id));
    }

    return doc.toByteArray();
}

#include "moc_minifluxretriever.cpp"
