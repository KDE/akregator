/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "akregatorinterfaces_export.h"
#include <QObject>

class QString;
#include <QStringList>

namespace Akregator
{
namespace Backend
{
class FeedStorage;

/** \brief Storage is the main interface to the article archive. It creates and manages FeedStorage objects handling the article list for a feed.

    An archive implementation must implement Storage, FeedStorage and StorageFactory. See mk4storage for an example.
*/
class Storage : public QObject // krazy:exclude=qobject
{
public:
    ~Storage() override = default;

    /** initializes the storage object with given parameters */

    virtual void initialize(const QStringList &params) = 0;

    /**
     * Open storage and prepare it for work.
     * @return true on success.
     */
    virtual bool open(bool autoCommit = false) = 0;

    /**
     * Commit changes made in feeds and articles, making them persistent.
     * @return true on success.
     */
    virtual bool commit() = 0;

    /**
     * Rollback changes made in feeds and articles, reverting to last committed values.
     * @returns true on success.
     */
    virtual bool rollback() = 0;

    /**
     * Closes storage, freeing all allocated resources. Called from destructor, so you don't need to call it directly.
     * @return true on success.
     */
    virtual void close() = 0;

    /**
     * @return Article archive for feed at given url.
     */
    virtual FeedStorage *archiveFor(const QString &url) = 0;
    virtual const FeedStorage *archiveFor(const QString &url) const = 0;
    virtual bool autoCommit() const = 0;

    /** stores the feed list in the storage backend. This is a fallback for the case that the
        feeds.opml file gets corrupted
        @param opmlStr the feed list in OPML format
     */
    virtual void storeFeedList(const QString &opmlStr) = 0;
    virtual QString restoreFeedList() const = 0;

    /** returns a list of all feeds (URLs) stored in this archive */

    virtual QStringList feeds() const = 0;
};
} // namespace Backend
} // namespace Akregator
