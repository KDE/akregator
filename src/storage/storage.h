/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#pragma once

#include "akregator_export.h"

#include <QObject>
#include <memory>

#include "feedstorage.h"

namespace Akregator
{
namespace Backend
{
class AKREGATOR_EXPORT Storage : public QObject
{
    Q_OBJECT
public:
    Storage();
    virtual ~Storage();

    /** KGlobal::dirs()->saveLocation("data", "akregator")+"/Archive" */
    static QString defaultArchivePath();

    /** sets the directory where the metakit files will be stored.

        @param archivePath the path to the archive, or QString() to reset it to the default.
     */
    void setArchivePath(const QString &archivePath);

    /** returns the path to the metakit archives */
    QString archivePath() const;

    /**
     * Open storage and prepare it for work.
     * @return true on success.
     */
    bool open(bool autoCommit = false);

    /**
     * Commit changes made in feeds and articles, making them persistent.
     * @return true on success.
     */
    bool commit();

    /**
     * Rollback changes made in feeds and articles, reverting to last committed values.
     * @returns true on success.
     */
    bool rollback();

    /**
     * Closes storage, freeing all allocated resources. Called from destructor, so you don't need to call it directly.
     * @return true on success.
     */
    void close();

    /**
     * @return Article archive for feed at given url.
     */
    FeedStorage *archiveFor(const QString &url);
    const FeedStorage *archiveFor(const QString &url) const;

    bool autoCommit() const;

    // API for FeedStorage to alter the feed index 'view'
    int unreadFor(const QString &url) const;
    void setUnreadFor(const QString &url, int unread);
    int totalCountFor(const QString &url) const;
    void setTotalCountFor(const QString &url, int total);
    QDateTime lastFetchFor(const QString &url) const;
    void setLastFetchFor(const QString &url, const QDateTime &lastFetch);

    QStringList feeds() const;

    void storeFeedList(const QString &opmlStr);
    QString restoreFeedList() const;

    void markDirty();

protected Q_SLOTS:
    void slotCommit();

private:
    class StoragePrivate;
    std::unique_ptr<StoragePrivate> const d;
};
} // namespace Backend
} // namespace Akregator
