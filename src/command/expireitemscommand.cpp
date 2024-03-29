/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "expireitemscommand.h"

#include "articlejobs.h"
#include "feed.h"
#include "feedlist.h"

#include "akregator_debug.h"

#include <QSet>
#include <QTimer>

#include <QSharedPointer>

#include <cassert>

using namespace Akregator;

class Akregator::ExpireItemsCommandPrivate
{
    ExpireItemsCommand *const q;

public:
    explicit ExpireItemsCommandPrivate(ExpireItemsCommand *qq);

    void createDeleteJobs();
    void addDeleteJobForFeed(Feed *feed);
    void jobFinished(KJob *);

    QWeakPointer<FeedList> m_feedList;
    QList<uint> m_feeds;
    QSet<KJob *> m_jobs;
};

ExpireItemsCommandPrivate::ExpireItemsCommandPrivate(ExpireItemsCommand *qq)
    : q(qq)
    , m_feedList()
{
}

void ExpireItemsCommandPrivate::addDeleteJobForFeed(Feed *feed)
{
    Q_ASSERT(feed);
    auto job = new ArticleDeleteJob(q);
    QObject::connect(job, &ArticleDeleteJob::finished, q, [this](KJob *job) {
        jobFinished(job);
    });
    m_jobs.insert(job);
    feed->deleteExpiredArticles(job);
    job->start();
}

void ExpireItemsCommandPrivate::jobFinished(KJob *job)
{
    Q_ASSERT(!m_jobs.isEmpty());
    m_jobs.remove(job);
    Q_EMIT q->progress(((m_feeds.count() - m_jobs.count()) * 100) / m_feeds.count(), QString());
    if (m_jobs.isEmpty()) {
        q->done();
    }
}

void ExpireItemsCommandPrivate::createDeleteJobs()
{
    Q_ASSERT(m_jobs.isEmpty());
    const QSharedPointer<FeedList> feedList = m_feedList.lock();

    if (m_feeds.isEmpty() || !feedList) {
        if (!feedList) {
            qCWarning(AKREGATOR_LOG) << "Associated feed list was deleted, could not expire items";
        }
        q->done();
        return;
    }

    for (const uint i : std::as_const(m_feeds)) {
        Feed *const feed = qobject_cast<Feed *>(feedList->findByID(i));
        if (feed) {
            addDeleteJobForFeed(feed);
        }
    }
}

ExpireItemsCommand::ExpireItemsCommand(QObject *parent)
    : Command(parent)
    , d(new ExpireItemsCommandPrivate(this))
{
}

ExpireItemsCommand::~ExpireItemsCommand() = default;

void ExpireItemsCommand::setFeedList(const QWeakPointer<FeedList> &feedList)
{
    d->m_feedList = feedList;
}

QWeakPointer<FeedList> ExpireItemsCommand::feedList() const
{
    return d->m_feedList;
}

void ExpireItemsCommand::setFeeds(const QList<uint> &feeds)
{
    d->m_feeds = feeds;
}

QList<uint> ExpireItemsCommand::feeds() const
{
    return d->m_feeds;
}

void ExpireItemsCommand::doAbort()
{
    for (KJob *const i : std::as_const(d->m_jobs)) {
        i->kill();
    }
}

void ExpireItemsCommand::doStart()
{
    QTimer::singleShot(0, this, [this]() {
        d->createDeleteJobs();
    });
}

#include "moc_expireitemscommand.cpp"
