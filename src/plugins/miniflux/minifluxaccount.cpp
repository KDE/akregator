/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#include "minifluxaccount.h"
#include "akregatorconfig.h"
#include "feedlist.h"
#include "folder.h"
#include "miniflux_debug.h"
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
    static constexpr int msPerMinute = 60 * 1000;
    const int intervalMinutes = Settings::autoFetchInterval();
    if (intervalMinutes > 0) {
        m_pollTimer.setInterval(intervalMinutes * msPerMinute);
        m_pollTimer.setSingleShot(false);
        connect(&m_pollTimer, &QTimer::timeout, this, &MinifluxAccount::onPollTimer);
    }
}

MinifluxAccount::~MinifluxAccount() = default;

void MinifluxAccount::initialize()
{
    if (!m_feedList) {
        return;
    }
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
        // Deleting a node always routes through Folder::removeChild(), even from the
        // node's own destructor, so signalChildRemoved alone cannot distinguish the
        // user deleting the account folder from the tree teardown on shutdown.
        // FeedList emits signalDestroyed() before destroying its folders, so
        // onFeedListDestroyed() detaches first and the shutdown cascade never
        // reaches onChildRemoved().
        connect(m_feedList->allFeedsFolder(), &Folder::signalChildRemoved, this, &MinifluxAccount::onChildRemoved);
        connect(m_feedList, &FeedList::signalDestroyed, this, &MinifluxAccount::onFeedListDestroyed);
        connect(m_rootFolder, &QObject::destroyed, this, &MinifluxAccount::onRootFolderDeleted);
    }

    // Watch any feeds already in the tree (plain Feed objects from OPML) so that
    // status changes made before the initial sync job finishes are still captured.
    watchExistingFeeds(m_rootFolder);
    startSyncJob();
}

void MinifluxAccount::onChildRemoved(Folder * /*parent*/, TreeNode *node)
{
    if (node != m_rootFolder) {
        return;
    }
    // The user deleted the account folder from the feed tree: forget the account.
    detachFromFeedList();
    removeFromConfig();
    Q_EMIT accountDeleted(this);
}

void MinifluxAccount::onFeedListDestroyed()
{
    detachFromFeedList();
    m_feedList = nullptr;
}

void MinifluxAccount::detachFromFeedList()
{
    m_pollTimer.stop();
    if (m_feedList) {
        disconnect(m_feedList, nullptr, this, nullptr);
        if (Folder *allFeeds = m_feedList->allFeedsFolder()) {
            disconnect(allFeeds, nullptr, this, nullptr);
        }
    }
    if (m_rootFolder) {
        disconnect(m_rootFolder, nullptr, this, nullptr);
        m_rootFolder = nullptr;
    }
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
        qCWarning(MINIFLUX_LOG) << "Miniflux sync failed for account" << m_accountName << ":" << job->errorText();
        Q_EMIT syncError(job->errorText());
        return;
    }
    if (!m_rootFolder) {
        // The account folder was deleted while the sync was in flight.
        return;
    }
    auto *syncJob = qobject_cast<MinifluxSyncJob *>(job);
    qCDebug(MINIFLUX_LOG) << "Miniflux sync succeeded for account" << m_accountName << "- categories:" << syncJob->categories().size()
                          << "feeds:" << syncJob->feeds().size();
    populateFeedTree(syncJob->categories(), syncJob->feeds());
    if (m_pollTimer.interval() > 0) {
        m_pollTimer.start();
    }
    Q_EMIT syncFinished();
}

void MinifluxAccount::onRootFolderDeleted()
{
    // Safety net for folder destruction that did not route through
    // onChildRemoved()/onFeedListDestroyed(); in-memory cleanup only.
    m_rootFolder = nullptr;
    m_pollTimer.stop();
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
    startSyncJob();
}

void MinifluxAccount::startSyncJob()
{
    if (!m_rootFolder) {
        return;
    }
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

#include "moc_minifluxaccount.cpp"
