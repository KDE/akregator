/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#include "minifluxaccount.h"
#include "akregatorconfig.h"
#include "feedlist.h"
#include "folder.h"
#include "minifluxclient.h"
#include "minifluxfeed.h"
#include "minifluxstatussync.h"
#include "minifluxsyncjob.h"

#include <KConfigGroup>
#include <KJob>
#include <KSharedConfig>

using namespace Qt::Literals::StringLiterals;
using namespace Akregator;

MinifluxAccount::MinifluxAccount(const QString &name,
                                 const QUrl &serverUrl,
                                 const QString &apiToken,
                                 FeedList *feedList,
                                 Backend::Storage *storage,
                                 QObject *parent)
    : QObject(parent)
    , m_accountName(name)
    , m_serverUrl(serverUrl)
    , m_apiToken(apiToken)
    , m_feedList(feedList)
    , m_storage(storage)
    , m_client(new MinifluxClient(serverUrl, apiToken, this))
    , m_statusSync(new MinifluxStatusSync(m_client, this))
{
    const int intervalMinutes = Settings::autoFetchInterval();
    if (intervalMinutes > 0) {
        m_pollTimer.setInterval(intervalMinutes * 60 * 1000);
        m_pollTimer.setSingleShot(false);
        connect(&m_pollTimer, &QTimer::timeout, this, &MinifluxAccount::onPollTimer);
    }
}

MinifluxAccount::~MinifluxAccount() = default;

void MinifluxAccount::initialize()
{
    if (!m_rootFolder) {
        // Reuse an existing folder left over from a previous session (loaded from OPML)
        const auto children = m_feedList->allFeedsFolder()->children();
        for (TreeNode *child : children) {
            if (auto *f = qobject_cast<Folder *>(child); f && f->title() == folderTitle()) {
                m_rootFolder = f;
                break;
            }
        }
        if (!m_rootFolder) {
            m_rootFolder = new Folder(folderTitle());
            m_feedList->allFeedsFolder()->appendChild(m_rootFolder);
        }
        connect(m_rootFolder, &QObject::destroyed, this, &MinifluxAccount::onRootFolderDeleted);
    }

    // Watch any feeds already in the tree (plain Feed objects from OPML) so that
    // status changes made before the initial sync job finishes are still captured.
    watchExistingFeeds(m_rootFolder);

    auto *job = new MinifluxSyncJob(m_client, this);
    connect(job, &KJob::finished, this, &MinifluxAccount::onSyncJobFinished);
    job->start();
}

void MinifluxAccount::watchExistingFeeds(Folder *folder)
{
    for (TreeNode *child : folder->children()) {
        if (auto *f = qobject_cast<Feed *>(child)) {
            m_statusSync->watchFeed(f);
        } else if (auto *sub = qobject_cast<Folder *>(child)) {
            watchExistingFeeds(sub);
        }
    }
}

QString MinifluxAccount::accountName() const
{
    return m_accountName;
}

void MinifluxAccount::onSyncJobFinished(KJob *job)
{
    if (job->error()) {
        Q_EMIT syncError(job->errorText());
        return;
    }
    auto *syncJob = qobject_cast<MinifluxSyncJob *>(job);
    populateFeedTree(syncJob->categories(), syncJob->feeds());
    if (m_pollTimer.interval() > 0) {
        m_pollTimer.start();
    }
    Q_EMIT syncFinished();
}

void MinifluxAccount::onRootFolderDeleted()
{
    m_rootFolder = nullptr;
    m_pollTimer.stop();
    removeFromConfig();
    Q_EMIT accountDeleted(this);
}

QString MinifluxAccount::folderTitle() const
{
    return m_accountName + u" (Miniflux)"_s;
}

void MinifluxAccount::removeFromConfig()
{
    KConfigGroup group = KSharedConfig::openConfig()->group(u"MinifluxAccounts"_s);
    QStringList accounts = group.readEntry("accounts", QStringList());
    accounts.removeAll(m_accountName);
    group.writeEntry("accounts", accounts);
    KSharedConfig::openConfig()->deleteGroup(u"MinifluxAccount-"_s + m_accountName);
    KSharedConfig::openConfig()->sync();
}

void MinifluxAccount::onPollTimer()
{
    auto *job = new MinifluxSyncJob(m_client, this);
    connect(job, &KJob::finished, this, &MinifluxAccount::onSyncJobFinished);
    job->start();
}

void MinifluxAccount::populateFeedTree(const QList<MinifluxCategory> &categories, const QList<MinifluxFeedData> &feeds)
{
    // Map category ID -> Folder
    QMap<int, Folder *> categoryFolders;
    for (const MinifluxCategory &cat : categories) {
        Folder *catFolder = nullptr;
        const auto children = m_rootFolder->children();
        for (TreeNode *child : children) {
            if (auto *f = qobject_cast<Folder *>(child)) {
                if (f->title() == cat.title) {
                    catFolder = f;
                    break;
                }
            }
        }
        if (!catFolder) {
            catFolder = new Folder(cat.title);
            m_rootFolder->appendChild(catFolder);
        }
        categoryFolders[cat.id] = catFolder;
    }

    // Add feeds to category folders
    for (const MinifluxFeedData &mfFeed : feeds) {
        Folder *parentFolder = categoryFolders.value(mfFeed.categoryId, m_rootFolder);

        bool exists = false;
        const QString expectedUrl = QStringLiteral("miniflux://feed/%1").arg(mfFeed.id);
        const auto children = parentFolder->children();
        for (TreeNode *child : children) {
            if (auto *mf = qobject_cast<MinifluxFeed *>(child)) {
                if (mf->minifluxFeedId() == mfFeed.id) {
                    exists = true;
                    m_statusSync->watchFeed(mf); // re-watch in case of reconnect
                    break;
                }
            } else if (auto *f = qobject_cast<Feed *>(child); f && f->xmlUrl() == expectedUrl) {
                // Stale plain Feed left over from OPML — replace with a proper MinifluxFeed
                m_statusSync->unwatchFeed(f);
                parentFolder->removeChild(f);
                f->deleteLater();
                break;
            }
        }
        if (!exists) {
            auto *feed = new MinifluxFeed(mfFeed.id, mfFeed.title, m_client, m_storage);
            feed->setHtmlUrl(mfFeed.siteUrl);
            parentFolder->appendChild(feed);
            m_statusSync->watchFeed(feed);
        }
    }
}
