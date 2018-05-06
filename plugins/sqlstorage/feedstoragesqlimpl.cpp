/*
    This file is part of Akregator.

    Copyright (C) 2018 Pierre Ducroquet <pinaraf@pinaraf.info>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "storagesqlimpl.h"
#include "feedstoragesqlimpl.h"

#include <QDebug>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

/// Simple helpers to use the Qt SQL database API

#define DUMP_SQL 1

template<typename T>
T simpleQuery(const QSqlDatabase &db, const char *query, const T &default_value, const std::initializer_list<QVariant> &parameters)
{
    QSqlQuery q(db);
    q.prepare(QLatin1String(query));
    for (auto &value: parameters) {
        q.addBindValue(value);
    }
    
    if (DUMP_SQL) {
    if (strcmp(query, "SELECT COUNT(*) FROM article WHERE feed_id = ? AND NOT(status  &8)") != 0) {
        qDebug() << "Executing query '" << query << "' with ... {";
        for (auto &value: parameters) {
            qDebug() << "    - " << value;
        }
        qDebug() << "} default " << default_value;
    }
    }
    
    if (!q.exec()) {
        qCritical() << "SQL error occured !";
        qCritical() << q.lastError().databaseText();
        qCritical() << q.lastError().driverText();
        return default_value;
    }
    if (!q.next()) {
        return default_value;
    }
    return q.value(0).value<T>();
}

bool simpleQuery(const QSqlDatabase &db, const char *query, const std::initializer_list<QVariant> &parameters)
{
    if (DUMP_SQL) {
        qDebug() << "Executing query '" << query << "' with ... {";
        for (auto &value: parameters) {
            qDebug() << "    - " << value;
        }
        qDebug() << "}";
    }
    QSqlQuery q(db);
    q.prepare(QLatin1String(query));
    for (auto &value: parameters) {
        q.addBindValue(value);
    }
    if (!q.exec()) {
        qCritical() << "SQL error occured !";
        qCritical() << q.lastError().databaseText();
        qCritical() << q.lastError().driverText();
        return false;
    }
    return true;
}


class Akregator::Backend::FeedStorageSqlImpl::FeedStorageSqlImplPrivate
{
public:
    FeedStorageSqlImplPrivate()
    {
    }

    int feed_id;
    Akregator::Backend::StorageSqlImpl *mainStorage;
};


Akregator::Backend::FeedStorageSqlImpl::FeedStorageSqlImpl(int feed_id, Akregator::Backend::StorageSqlImpl *main)
    : d(new FeedStorageSqlImplPrivate)
{
    d->feed_id = feed_id;
    d->mainStorage = main;
}

Akregator::Backend::FeedStorageSqlImpl::~FeedStorageSqlImpl()
{
}

void Akregator::Backend::FeedStorageSqlImpl::addEntry(const QString &guid)
{
    if (contains(guid))
        return;
    simpleQuery(d->mainStorage->database(),
                "INSERT INTO article (feed_id, guid, status) VALUES (?, ?, 0)",
                { d->feed_id, guid });
    d->mainStorage->markDirty();
}

bool Akregator::Backend::FeedStorageSqlImpl::contains(const QString &guid) const
{
    return simpleQuery(d->mainStorage->database(),
                       "SELECT true FROM article WHERE feed_id = ? AND guid = ?",
                       false,
                       { d->feed_id, guid });
}

QString Akregator::Backend::FeedStorageSqlImpl::content(const QString &guid) const
{
    return simpleQuery(d->mainStorage->database(),
                       "SELECT content FROM article WHERE feed_id = ? AND guid = ?",
                       QStringLiteral(""),
                       { d->feed_id, guid });
}

QStringList Akregator::Backend::FeedStorageSqlImpl::articles() const
{
    QStringList result;
    QSqlQuery q(d->mainStorage->database());
    q.prepare(QLatin1String("SELECT guid FROM article WHERE feed_id = ? ORDER BY publication_date DESC"));
    q.addBindValue(d->feed_id);
    if (q.exec()) {
        // Not all SQL backend handle returning size, sadly
        if (q.size() > 0)
            result.reserve(q.size());
        
        while (q.next()) {
            result << q.value(0).toString();
        }
    }
    return result;
}

void Akregator::Backend::FeedStorageSqlImpl::article(const QString &guid, uint &hash, QString &title, int &status, QDateTime &pubDate) const
{
    qDebug() << "Fetching article with guid " << guid;
    QStringList result;
    QSqlQuery q(d->mainStorage->database());
    q.prepare(QLatin1String("SELECT hash, title, status, publication_date FROM article WHERE feed_id = ? AND guid = ?"));
    q.addBindValue(d->feed_id);
    q.addBindValue(guid);
    if (!q.exec())
        return;
    
    if (!q.next())
        return;
    
    hash = q.value(0).toInt();
    title = q.value(1).toString();
    status = q.value(2).toInt();
    pubDate = q.value(3).toDateTime();
}

QString Akregator::Backend::FeedStorageSqlImpl::description(const QString &guid) const
{
    return simpleQuery(d->mainStorage->database(),
                       "SELECT description FROM article WHERE feed_id = ? AND guid = ?",
                       QStringLiteral(""),
                       { d->feed_id, guid });
}


QString Akregator::Backend::FeedStorageSqlImpl::authorEMail(const QString &guid) const
{
    return simpleQuery(d->mainStorage->database(),
                       "SELECT author_email FROM article WHERE feed_id = ? AND guid = ?",
                       QStringLiteral(""),
                       { d->feed_id, guid });
}

QString Akregator::Backend::FeedStorageSqlImpl::authorName(const QString &guid) const
{
    return simpleQuery(d->mainStorage->database(),
                       "SELECT author_name FROM article WHERE feed_id = ? AND guid = ?",
                       QStringLiteral(""),
                       { d->feed_id, guid });
}

QString Akregator::Backend::FeedStorageSqlImpl::authorUri(const QString &guid) const
{
    return simpleQuery(d->mainStorage->database(),
                       "SELECT author_url FROM article WHERE feed_id = ? AND guid = ?",
                       QStringLiteral(""),
                       { d->feed_id, guid });
}


void Akregator::Backend::FeedStorageSqlImpl::enclosure(const QString &guid, bool &hasEnclosure, QString &url, QString &type, int &length) const
{
    QSqlQuery q(d->mainStorage->database());
    q.prepare(QLatin1String("SELECT enclosure_url, enclosure_type, enclosure_length FROM article WHERE guid = ? AND feed_id = ? AND enclosure_url IS NOT NULL"));
    q.addBindValue(guid);
    q.addBindValue(d->feed_id);
    if (q.exec()) {
        if (!q.next()) {
            hasEnclosure = false;
            return;
        }
        hasEnclosure    = true;
        url             = q.value(0).toString();
        type            = q.value(1).toString();
        length          = q.value(2).toInt();
    }
}

bool Akregator::Backend::FeedStorageSqlImpl::guidIsHash(const QString &guid) const
{
    return simpleQuery(d->mainStorage->database(),
                       "SELECT guid_is_hash FROM article WHERE feed_id = ? AND guid = ?",
                       false,
                       { d->feed_id, guid });
}

bool Akregator::Backend::FeedStorageSqlImpl::guidIsPermaLink(const QString &guid) const
{
    return simpleQuery(d->mainStorage->database(),
                       "SELECT guid_is_permalink FROM article WHERE feed_id = ? AND guid = ?",
                       false,
                       { d->feed_id, guid });
}

uint Akregator::Backend::FeedStorageSqlImpl::hash(const QString &guid) const
{
    return simpleQuery(d->mainStorage->database(),
                       "SELECT hash FROM article WHERE feed_id = ? AND guid = ?",
                       0,
                       { d->feed_id, guid });
}

QString Akregator::Backend::FeedStorageSqlImpl::link(const QString &guid) const
{
    return simpleQuery(d->mainStorage->database(),
                       "SELECT link FROM article WHERE feed_id = ? AND guid = ?",
                       QStringLiteral(""),
                       { d->feed_id, guid });
}

QDateTime Akregator::Backend::FeedStorageSqlImpl::pubDate(const QString &guid) const
{
    return simpleQuery(d->mainStorage->database(),
                       "SELECT publication_date FROM article WHERE feed_id = ? AND guid = ?",
                       QDateTime(),
                       { d->feed_id, guid });
}

int Akregator::Backend::FeedStorageSqlImpl::status(const QString &guid) const
{
    return simpleQuery(d->mainStorage->database(),
                       "SELECT status FROM article WHERE feed_id = ? AND guid = ?",
                       0,
                       { d->feed_id, guid });
}

QString Akregator::Backend::FeedStorageSqlImpl::title(const QString &guid) const
{
    return simpleQuery(d->mainStorage->database(),
                       "SELECT title FROM article WHERE feed_id = ? AND guid = ?",
                       QStringLiteral(""),
                       { d->feed_id, guid });
}

void Akregator::Backend::FeedStorageSqlImpl::removeEnclosure(const QString &guid)
{
    simpleQuery(d->mainStorage->database(),
            "UPDATE article SET enclosure_url = NULL, enclosure_type = NULL, enclosure_length = NULL WHERE guid = ? AND feed_id = ?",
            { guid, d->feed_id });
    d->mainStorage->markDirty();
}

void Akregator::Backend::FeedStorageSqlImpl::deleteArticle(const QString &guid)
{
    simpleQuery(d->mainStorage->database(),
                "DELETE FROM article WHERE guid = ? AND feed_id = ?",
                { guid, d->feed_id });
    d->mainStorage->markDirty();
}

void Akregator::Backend::FeedStorageSqlImpl::setAuthorEMail(const QString &guid, const QString &email)
{
    simpleQuery(d->mainStorage->database(),
                "UPDATE article SET author_email = ? WHERE guid = ? AND feed_id = ?",
                { email, guid, d->feed_id });
    d->mainStorage->markDirty();
}

void Akregator::Backend::FeedStorageSqlImpl::setAuthorName(const QString &guid, const QString &name)
{
    simpleQuery(d->mainStorage->database(),
                "UPDATE article SET author_name = ? WHERE guid = ? AND feed_id = ?",
                { name, guid, d->feed_id });
    d->mainStorage->markDirty();
}

void Akregator::Backend::FeedStorageSqlImpl::setAuthorUri(const QString &guid, const QString &uri)
{
    simpleQuery(d->mainStorage->database(),
                "UPDATE article SET author_url = ? WHERE guid = ? AND feed_id = ?",
                { uri, guid, d->feed_id });
    d->mainStorage->markDirty();
}

void Akregator::Backend::FeedStorageSqlImpl::setContent(const QString &guid, const QString &content)
{
    simpleQuery(d->mainStorage->database(),
                "UPDATE article SET content = ? WHERE guid = ? AND feed_id = ?",
                { content, guid, d->feed_id });
    d->mainStorage->markDirty();
}

void Akregator::Backend::FeedStorageSqlImpl::setDeleted(const QString &guid)
{
    simpleQuery(d->mainStorage->database(),
                "UPDATE article SET content = '', description = '', title = '', link = '', author_name = '', author_url = '', author_email = '' WHERE guid = ? AND feed_id = ?",
                { guid, d->feed_id });
    d->mainStorage->markDirty();
}

void Akregator::Backend::FeedStorageSqlImpl::setDescription(const QString &guid, const QString &description)
{
    simpleQuery(d->mainStorage->database(),
                "UPDATE article SET description = ? WHERE guid = ? AND feed_id = ?",
                { description, guid, d->feed_id });
    d->mainStorage->markDirty();
}

void Akregator::Backend::FeedStorageSqlImpl::setEnclosure(const QString &guid, const QString &url, const QString &type, int length)
{
    simpleQuery(d->mainStorage->database(),
                "UPDATE article SET enclosure_url = ?, enclosure_type = ?, enclosure_length = ? WHERE guid = ? AND feed_id = ?",
                { url, type, length, guid, d->feed_id });
    d->mainStorage->markDirty();
}

void Akregator::Backend::FeedStorageSqlImpl::setGuidIsHash(const QString &guid, bool isHash)
{
    simpleQuery(d->mainStorage->database(),
                "UPDATE article SET guid_is_hash = ? WHERE guid = ? AND feed_id = ?",
                { isHash, guid, d->feed_id });
    d->mainStorage->markDirty();
}

void Akregator::Backend::FeedStorageSqlImpl::setGuidIsPermaLink(const QString &guid, bool isPermaLink)
{
    simpleQuery(d->mainStorage->database(),
                "UPDATE article SET guid_is_permalink = ? WHERE guid = ? AND feed_id = ?",
                { isPermaLink, guid, d->feed_id });
    d->mainStorage->markDirty();
}

void Akregator::Backend::FeedStorageSqlImpl::setHash(const QString &guid, uint hash)
{
    simpleQuery(d->mainStorage->database(),
                "UPDATE article SET hash = ? WHERE guid = ? AND feed_id = ?",
                { hash, guid, d->feed_id });
    d->mainStorage->markDirty();
}

void Akregator::Backend::FeedStorageSqlImpl::setLink(const QString &guid, const QString &link)
{
    simpleQuery(d->mainStorage->database(),
                "UPDATE article SET link = ? WHERE guid = ? AND feed_id = ?",
                { link, guid, d->feed_id });
    d->mainStorage->markDirty();
}

void Akregator::Backend::FeedStorageSqlImpl::setPubDate(const QString &guid, const QDateTime &pubdate)
{
    simpleQuery(d->mainStorage->database(),
                "UPDATE article SET publication_date = ? WHERE guid = ? AND feed_id = ?",
                { pubdate, guid, d->feed_id });
    d->mainStorage->markDirty();
}

void Akregator::Backend::FeedStorageSqlImpl::setStatus(const QString &guid, int status)
{
    simpleQuery(d->mainStorage->database(),
                "UPDATE article SET status = ? WHERE guid = ? AND feed_id = ?",
                { status, guid, d->feed_id });
    d->mainStorage->markDirty();
}

void Akregator::Backend::FeedStorageSqlImpl::setTitle(const QString &guid, const QString &title)
{
    simpleQuery(d->mainStorage->database(),
                "UPDATE article SET title= ? WHERE guid = ? AND feed_id = ?",
                { title, guid, d->feed_id });
    d->mainStorage->markDirty();
}


// Queries with no guid, aka feed table or statistics

QDateTime Akregator::Backend::FeedStorageSqlImpl::lastFetch() const
{
    return simpleQuery(d->mainStorage->database(),
                       "SELECT last_fetch FROM feed WHERE id = ?",
                       QDateTime(),
                       { d->feed_id });
    return QDateTime();
}

int Akregator::Backend::FeedStorageSqlImpl::totalCount() const
{
    return simpleQuery(d->mainStorage->database(),
                       "SELECT COUNT(*) FROM article WHERE feed_id = ?",
                       0,
                       { d->feed_id });
}

int Akregator::Backend::FeedStorageSqlImpl::unread() const
{
    return simpleQuery(d->mainStorage->database(),
                       "SELECT COUNT(*) FROM article WHERE feed_id = ? AND NOT(status  &8)",
                       0,
                       { d->feed_id });
}

void Akregator::Backend::FeedStorageSqlImpl::setLastFetch(const QDateTime &lastFetch)
{
    simpleQuery(d->mainStorage->database(),
                "UPDATE feed SET last_fetch = ? WHERE id = ?",
                { lastFetch, d->feed_id });
    d->mainStorage->markDirty();
}

void Akregator::Backend::FeedStorageSqlImpl::setUnread(int unread)
{
    Q_UNUSED(unread);
    // So far the SQL backend uses an index to maintain unread by itself.
    
    /* IGNORE
    simpleQuery(d->mainStorage->database(),
                "UPDATE feed SET last_fetch = ? WHERE id = ?",
                { unread, d->feed_id });
    d->mainStorage->markDirty();
    */
}
