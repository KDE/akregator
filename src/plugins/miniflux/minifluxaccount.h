/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#pragma once

#include "minifluxdata.h"
#include <KJob>
#include <QObject>
#include <QTimer>
#include <QUrl>

namespace Akregator
{
class FeedList;
class Folder;
class MinifluxClient;
class MinifluxStatusSync;
class TreeNode;

namespace Backend
{
class Storage;
}

/**
 * Manages one Miniflux server account: owns the client, the top-level
 * "Miniflux" folder in the feed tree, and the periodic sync timer.
 */
class MinifluxAccount : public QObject
{
    Q_OBJECT
public:
    explicit MinifluxAccount(const QString &name,
                             const QUrl &serverUrl,
                             const QString &apiToken,
                             FeedList *feedList,
                             Backend::Storage *storage,
                             QObject *parent = nullptr);
    ~MinifluxAccount() override;

    /** Start initial sync and set up polling timer. */
    void initialize();

    [[nodiscard]] QString accountName() const;
    [[nodiscard]] QUrl serverUrl() const;
    [[nodiscard]] QString apiToken() const;

    /** Update the server connection settings of a live account and resync. */
    void setCredentials(const QUrl &serverUrl, const QString &apiToken);

    /** Remove the account folder from the feed tree, which also removes the
     *  account from the configuration and emits accountDeleted(). */
    void removeFromTree();

Q_SIGNALS:
    void syncFinished();
    void syncError(const QString &message);
    /** Emitted when the user deletes the account's root folder from the tree. */
    void accountDeleted(MinifluxAccount *account);

private:
    void onSyncJobFinished(KJob *job);
    void onPollTimer();
    void onRootFolderDeleted();
    void onChildRemoved(Akregator::Folder *parent, Akregator::TreeNode *node);
    void onFeedListDestroyed();
    void populateFeedTree(const QList<MinifluxCategory> &categories, const QList<MinifluxFeedData> &feeds);
    void watchExistingFeeds(Folder *folder);
    void removeFromConfig();
    void startSyncJob();
    void detachFromFeedList();
    [[nodiscard]] QString folderTitle() const;

    QString m_accountName;
    QUrl m_serverUrl;
    QString m_apiToken;
    FeedList *m_feedList = nullptr;
    Backend::Storage *m_storage = nullptr;
    MinifluxClient *m_client = nullptr;
    MinifluxStatusSync *m_statusSync = nullptr;
    Folder *m_rootFolder = nullptr;
    QTimer m_pollTimer;
};

} // namespace Akregator
