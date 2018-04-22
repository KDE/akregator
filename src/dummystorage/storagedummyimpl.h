/*
    This file is part of Akregator.

    Copyright (C) 2005 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
                  2005 Frank Osterfeld <osterfeld@kde.org>

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

#ifndef AKREGATOR_STORAGEDUMMYIMPL_H
#define AKREGATOR_STORAGEDUMMYIMPL_H

#include "storage.h"

namespace Akregator {
namespace Backend {
/**
 * Metakit implementation of Storage interface
 */
class StorageDummyImpl : public Storage
{
    Q_OBJECT
public:

    StorageDummyImpl();
    StorageDummyImpl(const StorageDummyImpl &);
    StorageDummyImpl &operator =(const StorageDummyImpl &);
    ~StorageDummyImpl() override;
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
    bool close() override;

    /**
     * @return Article archive for feed at given url.
     */
    FeedStorage *archiveFor(const QString &url) override;
    const FeedStorage *archiveFor(const QString &url) const override;
    bool autoCommit() const override;
    int unreadFor(const QString &url) const override;
    void setUnreadFor(const QString &url, int unread) override;
    int totalCountFor(const QString &url) const override;
    void setTotalCountFor(const QString &url, int total) override;
    QDateTime lastFetchFor(const QString &url) const override;
    void setLastFetchFor(const QString &url, const QDateTime &lastFetch) override;
    QStringList feeds() const override;

    void storeFeedList(const QString &opmlStr) override;
    QString restoreFeedList() const override;

    /** adds all feed storages from a source to this storage
        existing articles are replaced
    */
    void add(Storage *source) override;

    /** deletes all feed storages in this archive */
    void clear() override;

protected Q_SLOTS:
    void slotCommit();

private:
    class StorageDummyImplPrivate;
    StorageDummyImplPrivate *d;
};
} // namespace Backend
} // namespace Akregator

#endif // AKREGATOR_STORAGEDUMMYIMPL_H
