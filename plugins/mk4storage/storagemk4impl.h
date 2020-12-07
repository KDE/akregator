/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef STORAGEMK4IMPL_H
#define STORAGEMK4IMPL_H

#include "storage.h"

namespace Akregator {
namespace Backend {
/**
 * Metakit implementation of Storage interface
 */
class StorageMK4Impl : public Storage
{
    Q_OBJECT
public:

    StorageMK4Impl();
    StorageMK4Impl(const StorageMK4Impl &);
    StorageMK4Impl &operator =(const StorageMK4Impl &);
    ~StorageMK4Impl() override;

    /** KGlobal::dirs()->saveLocation("data", "akregator")+"/Archive" */
    static QString defaultArchivePath();

    /** sets the directory where the metakit files will be stored.

        @param archivePath the path to the archive, or QString() to reset it to the default.
     */
    void setArchivePath(const QString &archivePath);

    /** returns the path to the metakit archives */
    QString archivePath() const;

    void initialize(const QStringList &params) override;
    /**
     * Open storage and prepare it for work.
     * @return true on success.
     */
    bool open(bool autoCommit = false) override;

    /**
     * Commit changes made in feeds and articles, making them persistent.
     * @return true on success.
     */
    bool commit() override;

    /**
     * Rollback changes made in feeds and articles, reverting to last committed values.
     * @returns true on success.
     */
    bool rollback() override;

    /**
     * Closes storage, freeing all allocated resources. Called from destructor, so you don't need to call it directly.
     * @return true on success.
     */
    void close() override;

    /**
     * @return Article archive for feed at given url.
     */
    FeedStorage *archiveFor(const QString &url) override;
    const FeedStorage *archiveFor(const QString &url) const override;

    bool autoCommit() const override;

    // API for FeedStorage to alter the feed index 'view'
    int unreadFor(const QString &url) const;
    void setUnreadFor(const QString &url, int unread);
    int totalCountFor(const QString &url) const;
    void setTotalCountFor(const QString &url, int total);
    QDateTime lastFetchFor(const QString &url) const;
    void setLastFetchFor(const QString &url, const QDateTime &lastFetch);

    QStringList feeds() const override;

    void storeFeedList(const QString &opmlStr) override;
    QString restoreFeedList() const override;

    void markDirty();

protected Q_SLOTS:
    void slotCommit();

private:
    class StorageMK4ImplPrivate;
    StorageMK4ImplPrivate *d;
};
} // namespace Backend
} // namespace Akregator

#endif // STORAGEMK4IMPL_H
