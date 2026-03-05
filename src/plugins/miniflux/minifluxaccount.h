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

Q_SIGNALS:
    void syncFinished();
    void syncError(const QString &message);
    /** Emitted when the user deletes the account's root folder from the tree. */
    void accountDeleted(MinifluxAccount *account);

private Q_SLOTS:
    void onSyncJobFinished(KJob *job);
    void onPollTimer();
    void onRootFolderDeleted();

private:
    void populateFeedTree(const QList<MinifluxCategory> &categories, const QList<MinifluxFeedData> &feeds);
    void watchExistingFeeds(Folder *folder);
    void removeFromConfig();
    [[nodiscard]] QString folderTitle() const;

    QString m_accountName;
    QUrl m_serverUrl;
    QString m_apiToken;
    FeedList *m_feedList;
    Backend::Storage *m_storage;
    MinifluxClient *m_client;
    MinifluxStatusSync *m_statusSync;
    Folder *m_rootFolder = nullptr;
    QTimer m_pollTimer;
};

} // namespace Akregator
