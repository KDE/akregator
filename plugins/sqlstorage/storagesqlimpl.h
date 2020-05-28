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

#ifndef STORAGESQLIMPL_H
#define STORAGESQLIMPL_H

#include "storage.h"

class QSqlDatabase;

namespace Akregator {
namespace Backend {
/**
 * SQL implementation of Storage interface
 */
class StorageSqlImpl : public Storage
{
    Q_OBJECT
public:

    StorageSqlImpl();
    StorageSqlImpl(const StorageSqlImpl &);
    StorageSqlImpl &operator =(const StorageSqlImpl &);
    ~StorageSqlImpl();

    /// Archive path is used for SQLite mainly
    static QString defaultArchivePath();

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
    
    QStringList feeds() const override;

    void storeFeedList(const QString &opmlStr) override;
    QString restoreFeedList() const override;

    QSqlDatabase database();

    void markDirty();
private:
    class StorageSqlImplPrivate;
    StorageSqlImplPrivate *d;
};
} // namespace Backend
} // namespace Akregator

#endif // STORAGESQLIMPL_H

