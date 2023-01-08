/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#include "storage.h"

#include <mk4.h>

#include <QMap>
#include <QString>
#include <QStringList>
#include <QTimer>

#include <QDateTime>
#include <QDir>
#include <QStandardPaths>
#include <chrono>

using namespace std::chrono_literals;

class Akregator::Backend::Storage::StoragePrivate
{
public:
    StoragePrivate()
        : purl("url")
        , pFeedList("feedList")
        , punread("unread")
        , ptotalCount("totalCount")
        , plastFetch("lastFetch")
    {
    }

    c4_Storage *storage;
    Akregator::Backend::Storage *q;
    c4_View archiveView;
    bool autoCommit = false;
    bool modified = false;
    mutable QMap<QString, Akregator::Backend::FeedStorage *> feeds;
    QStringList feedURLs;
    c4_StringProp purl, pFeedList;
    c4_IntProp punread, ptotalCount, plastFetch;
    QString archivePath;

    c4_Storage *feedListStorage;
    c4_View feedListView;

    Akregator::Backend::FeedStorage *createFeedStorage(const QString &url);
};

Akregator::Backend::Storage::Storage()
    : d(new StoragePrivate)
{
    d->q = this;
    setArchivePath(QString());
}

Akregator::Backend::FeedStorage *Akregator::Backend::Storage::StoragePrivate::createFeedStorage(const QString &url)
{
    if (!feeds.contains(url)) {
        auto fs = new Akregator::Backend::FeedStorage(url, q);
        feeds[url] = fs;
        c4_Row findrow;
        purl(findrow) = url.toLatin1().constData();
        int findidx = archiveView.Find(findrow);
        if (findidx == -1) {
            punread(findrow) = 0;
            ptotalCount(findrow) = 0;
            plastFetch(findrow) = 0;
            archiveView.Add(findrow);
            modified = true;
        }
    }
    return feeds[url];
}

Akregator::Backend::FeedStorage *Akregator::Backend::Storage::archiveFor(const QString &url)
{
    return d->createFeedStorage(url);
}

const Akregator::Backend::FeedStorage *Akregator::Backend::Storage::archiveFor(const QString &url) const
{
    return d->createFeedStorage(url);
}

void Akregator::Backend::Storage::setArchivePath(const QString &archivePath)
{
    if (archivePath.isNull()) { // if isNull, reset to default
        d->archivePath = defaultArchivePath();
    } else {
        d->archivePath = archivePath;
    }
}

QString Akregator::Backend::Storage::archivePath() const
{
    return d->archivePath;
}

QString Akregator::Backend::Storage::defaultArchivePath()
{
    const QString ret = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/akregator/Archive");
    QDir().mkpath(ret);
    return ret;
}

Akregator::Backend::Storage::~Storage()
{
    close();
}

bool Akregator::Backend::Storage::open(bool autoCommit)
{
    QString filePath = d->archivePath + QLatin1String("/archiveindex.mk4");
    d->storage = new c4_Storage(filePath.toLocal8Bit().constData(), true);
    d->archiveView = d->storage->GetAs("archive[url:S,unread:I,totalCount:I,lastFetch:I]");
    c4_View hash = d->storage->GetAs("archiveHash[_H:I,_R:I]");
    d->archiveView = d->archiveView.Hash(hash, 1); // hash on url
    d->autoCommit = autoCommit;

    filePath = d->archivePath + QLatin1String("/feedlistbackup.mk4");
    d->feedListStorage = new c4_Storage(filePath.toLocal8Bit().constData(), true);
    d->feedListView = d->feedListStorage->GetAs("archive[feedList:S,tagSet:S]");
    return true;
}

bool Akregator::Backend::Storage::autoCommit() const
{
    return d->autoCommit;
}

void Akregator::Backend::Storage::close()
{
    QMap<QString, FeedStorage *>::Iterator it;
    QMap<QString, FeedStorage *>::Iterator end(d->feeds.end());
    for (it = d->feeds.begin(); it != end; ++it) {
        it.value()->close();
        delete it.value();
    }
    if (d->autoCommit) {
        d->storage->Commit();
    }

    delete d->storage;
    d->storage = nullptr;

    d->feedListStorage->Commit();
    delete d->feedListStorage;
    d->feedListStorage = nullptr;
}

bool Akregator::Backend::Storage::commit()
{
    QMap<QString, FeedStorage *>::Iterator it;
    QMap<QString, FeedStorage *>::Iterator end(d->feeds.end());
    for (it = d->feeds.begin(); it != end; ++it) {
        it.value()->commit();
    }

    if (d->storage) {
        d->storage->Commit();
        return true;
    }

    return false;
}

bool Akregator::Backend::Storage::rollback()
{
    QMap<QString, FeedStorage *>::Iterator it;
    QMap<QString, FeedStorage *>::Iterator end(d->feeds.end());
    for (it = d->feeds.begin(); it != end; ++it) {
        it.value()->rollback();
    }

    if (d->storage) {
        d->storage->Rollback();
        return true;
    }
    return false;
}

int Akregator::Backend::Storage::unreadFor(const QString &url) const
{
    c4_Row findrow;
    d->purl(findrow) = url.toLatin1().constData();
    int findidx = d->archiveView.Find(findrow);

    return findidx != -1 ? d->punread(d->archiveView.GetAt(findidx)) : 0;
}

void Akregator::Backend::Storage::setUnreadFor(const QString &url, int unread)
{
    c4_Row findrow;
    d->purl(findrow) = url.toLatin1().constData();
    int findidx = d->archiveView.Find(findrow);
    if (findidx == -1) {
        return;
    }
    findrow = d->archiveView.GetAt(findidx);
    d->punread(findrow) = unread;
    d->archiveView.SetAt(findidx, findrow);
    markDirty();
}

int Akregator::Backend::Storage::totalCountFor(const QString &url) const
{
    c4_Row findrow;
    d->purl(findrow) = url.toLatin1().constData();
    int findidx = d->archiveView.Find(findrow);

    return findidx != -1 ? d->ptotalCount(d->archiveView.GetAt(findidx)) : 0;
}

void Akregator::Backend::Storage::setTotalCountFor(const QString &url, int total)
{
    c4_Row findrow;
    d->purl(findrow) = url.toLatin1().constData();
    int findidx = d->archiveView.Find(findrow);
    if (findidx == -1) {
        return;
    }
    findrow = d->archiveView.GetAt(findidx);
    d->ptotalCount(findrow) = total;
    d->archiveView.SetAt(findidx, findrow);
    markDirty();
}

QDateTime Akregator::Backend::Storage::lastFetchFor(const QString &url) const
{
    c4_Row findrow;
    d->purl(findrow) = url.toLatin1().constData();
    int findidx = d->archiveView.Find(findrow);

    return findidx != -1 ? QDateTime::fromSecsSinceEpoch(d->plastFetch(d->archiveView.GetAt(findidx))) : QDateTime();
}

void Akregator::Backend::Storage::setLastFetchFor(const QString &url, const QDateTime &lastFetch)
{
    c4_Row findrow;
    d->purl(findrow) = url.toLatin1().constData();
    int findidx = d->archiveView.Find(findrow);
    if (findidx == -1) {
        return;
    }
    findrow = d->archiveView.GetAt(findidx);
    d->plastFetch(findrow) = lastFetch.toSecsSinceEpoch();
    d->archiveView.SetAt(findidx, findrow);
    markDirty();
}

void Akregator::Backend::Storage::markDirty()
{
    if (!d->modified) {
        d->modified = true;
        // commit changes after 3 seconds
        QTimer::singleShot(3s, this, &Storage::slotCommit);
    }
}

void Akregator::Backend::Storage::slotCommit()
{
    if (d->modified) {
        commit();
    }
    d->modified = false;
}

QStringList Akregator::Backend::Storage::feeds() const
{
    // TODO: cache list
    QStringList list;
    const int size = d->archiveView.GetSize();
    list.reserve(size);
    for (int i = 0; i < size; ++i) {
        list += QString::fromLatin1(QByteArray(d->purl(d->archiveView.GetAt(i))));
    }
    // fill with urls
    return list;
}

void Akregator::Backend::Storage::storeFeedList(const QString &opmlStr)
{
    if (d->feedListView.GetSize() == 0) {
        c4_Row row;
        d->pFeedList(row) = !opmlStr.isEmpty() ? opmlStr.toUtf8().data() : "";
        d->feedListView.Add(row);
    } else {
        c4_Row row = d->feedListView.GetAt(0);
        d->pFeedList(row) = !opmlStr.isEmpty() ? opmlStr.toUtf8().data() : "";
        d->feedListView.SetAt(0, row);
    }
    markDirty();
}

QString Akregator::Backend::Storage::restoreFeedList() const
{
    if (d->feedListView.GetSize() == 0) {
        return {};
    }

    c4_Row row = d->feedListView.GetAt(0);
    return QString::fromUtf8(QByteArray(d->pFeedList(row)));
}
