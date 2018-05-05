/*
    This file is part of Akregator.

    Copyright (C) 2018 Pierre Ducroquet <pinaraf@pinaraf.info>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will b useful,
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

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QTimer>
#include <QVariant>

class Akregator::Backend::StorageSqlImpl::StorageSqlImplPrivate
{
public:
    StorageSqlImplPrivate()
    {
    }

    StorageSqlImpl *parent;
    QSqlDatabase db;
    bool autoCommit;
    QTimer *autoCommitTimer;
    Akregator::Backend::FeedStorageSqlImpl *createFeedStorage(const QString &url);
};

Akregator::Backend::FeedStorageSqlImpl * Akregator::Backend::StorageSqlImpl::StorageSqlImplPrivate::createFeedStorage(const QString &url)
{
    int feed_id;
    
    // Try to find the current feed_id. If it does not exists, insert a new row in feed.
    QSqlQuery q(db);
    q.prepare(QLatin1String("SELECT id FROM feed WHERE url = ?"));
    q.addBindValue(url);
    if (!q.exec()) {
        // TODO : forward error somehow ?
        return nullptr;
    }
    if (q.next()) {
        feed_id = q.value(0).toInt();
    } else {
        q.prepare(QLatin1String("INSERT INTO feed(url, last_fetch) VALUES (?, datetime('now'));"));
        q.addBindValue(url);
        if (!q.exec()) {
            // TODO : forward error somehow ?
            return nullptr;
        }
        feed_id = q.lastInsertId().toInt();
    }
    return new FeedStorageSqlImpl(feed_id, parent);
}

Akregator::Backend::StorageSqlImpl::StorageSqlImpl() : d(new StorageSqlImplPrivate)
{
    d->parent = this;
    d->autoCommitTimer = new QTimer(this);
    d->autoCommitTimer->setInterval(3000);      // Commit each 3s. at most
    d->autoCommitTimer->setSingleShot(true);    // markDirty will start this timer
    QObject::connect(d->autoCommitTimer, &QTimer::timeout, this, [this] () {
        this->commit();
        this->d->db.transaction();
    });
}

Akregator::Backend::StorageSqlImpl::~StorageSqlImpl()
{
    if (d->autoCommit)
        this->commit();
    d->autoCommitTimer->deleteLater();
    delete(d);
    d = nullptr;
}

Akregator::Backend::FeedStorage *Akregator::Backend::StorageSqlImpl::archiveFor(const QString &url)
{
    return d->createFeedStorage(url);
}

const Akregator::Backend::FeedStorage *Akregator::Backend::StorageSqlImpl::archiveFor(const QString &url) const
{
    return d->createFeedStorage(url);
}

bool Akregator::Backend::StorageSqlImpl::autoCommit() const
{
    return d->autoCommit;
}

void Akregator::Backend::StorageSqlImpl::close()
{
    d->db.close();
}

bool Akregator::Backend::StorageSqlImpl::commit()
{
    bool result = d->db.commit();
    this->d->db.transaction();
    return result;
}

QStringList Akregator::Backend::StorageSqlImpl::feeds() const
{
    return QStringList();
}

QString Akregator::Backend::StorageSqlImpl::defaultArchivePath()
{
    const QString ret = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/akregator");
    QDir().mkpath(ret);
    return ret;
}

void Akregator::Backend::StorageSqlImpl::initialize(const QStringList &params)
{
    qDebug() << "Initializing StorageSqlImpl with params = {" << params << "}";
    d->db = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"));
    d->db.setDatabaseName(defaultArchivePath() + QStringLiteral("/archive.sqlite"));
}

bool Akregator::Backend::StorageSqlImpl::open(bool autoCommit)
{
    d->autoCommit = autoCommit;
    if (!d->db.open( /* TODO : user, password */ )) {
        return false;
    }
    // TODO Versioning ?
    QStringList tables = d->db.tables(QSql::TableType::Tables);
    if (!tables.contains(QStringLiteral("opml"))) {
        d->db.exec(QStringLiteral("CREATE TABLE opml(data text);"));
        markDirty();
    }
    if (!tables.contains(QStringLiteral("feed"))) {
        d->db.exec(QStringLiteral("CREATE TABLE feed(id integer primary key, url text unique not null, unread integer not null default 0, last_fetch timestamp not null);"));
        markDirty();
    }
    if (!tables.contains(QStringLiteral("article"))) {
        d->db.exec(QStringLiteral("CREATE TABLE article(feed_id integer not null, guid text not null, title text, hash integer, guid_is_hash boolean, guid_is_permalink boolean, description text, link text, status integer, publication_date timestamp, enclosure_url text, enclosure_type text, enclosure_length integer, author_name text, author_url text, author_email text, content text);"));
        d->db.exec(QStringLiteral("CREATE UNIQUE INDEX article_feed_guid ON article(feed_id, guid);"));
        d->db.exec(QStringLiteral("CREATE INDEX article_feed_unread ON article(feed_id, guid) WHERE (NOT(status & 8));"));
        markDirty();
    }
    
    d->db.transaction();
    return true;
}

bool Akregator::Backend::StorageSqlImpl::rollback()
{
    return d->db.rollback();
}

QString Akregator::Backend::StorageSqlImpl::restoreFeedList() const
{
    QSqlQuery q = d->db.exec(QStringLiteral("SELECT data FROM opml;"));
    if (q.next())
        return q.value(0).toString();
    else
        return QString::null;
}

void Akregator::Backend::StorageSqlImpl::storeFeedList(const QString &opmlStr)
{
    d->db.exec(QStringLiteral("DELETE FROM opml;"));
    QSqlQuery q(d->db);
    q.prepare(QLatin1String("INSERT INTO opml(data) VALUES (?);"));
    q.addBindValue(opmlStr);
    q.exec();
    // This asserts it worked.
}

QSqlDatabase Akregator::Backend::StorageSqlImpl::database()
{
    return d->db;
}

void Akregator::Backend::StorageSqlImpl::markDirty()
{
    if (d->autoCommit) {
        d->autoCommitTimer->stop();
        d->autoCommitTimer->start();
    }
}

