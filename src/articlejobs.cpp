/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2007 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "articlejobs.h"
#include "article.h"
#include "feed.h"
#include "feedlist.h"
#include "kernel.h"

#include "akregator_debug.h"
#include <KLocalizedString>

#include <QTimer>

#include <vector>

#include <cassert>

using namespace Akregator;

ArticleDeleteJob::ArticleDeleteJob(QObject *parent)
    : KJob(parent)
    , m_feedList(Kernel::self()->feedList())
{
    Q_ASSERT(m_feedList);
}

void ArticleDeleteJob::appendArticleIds(const Akregator::ArticleIdList &ids)
{
    m_ids += ids;
}

void ArticleDeleteJob::appendArticleId(const ArticleId &id)
{
    m_ids += id;
}

void ArticleDeleteJob::start()
{
    QTimer::singleShot(20, this, &ArticleDeleteJob::doStart);
}

void ArticleDeleteJob::doStart()
{
    if (!m_feedList) {
        qCWarning(AKREGATOR_LOG) << "Feedlist object was deleted, items not deleted";
        emitResult();
        return;
    }
    std::vector<Feed *> feeds;

    for (const ArticleId &id : qAsConst(m_ids)) {
        Article article = m_feedList->findArticle(id.feedUrl, id.guid);
        if (article.isNull()) {
            continue;
        }

        if (Feed *const feed = m_feedList->findByURL(id.feedUrl)) {
            feeds.push_back(feed);
            feed->setNotificationMode(false);
        }
        article.setDeleted();
    }

    for (Feed *const i : qAsConst(feeds)) {
        i->setNotificationMode(true);
    }

    emitResult();
}

ArticleModifyJob::ArticleModifyJob(QObject *parent)
    : KJob(parent)
    , m_feedList(Kernel::self()->feedList())
{
    Q_ASSERT(m_feedList);
}

void ArticleModifyJob::setStatus(const ArticleId &id, int status)
{
    m_status[id] = status;
}

void ArticleModifyJob::setKeep(const ArticleId &id, bool keep)
{
    m_keepFlags[id] = keep;
}

void ArticleModifyJob::start()
{
    QTimer::singleShot(20, this, &ArticleModifyJob::doStart);
}

void ArticleModifyJob::doStart()
{
    if (!m_feedList) {
        qCWarning(AKREGATOR_LOG) << "Feedlist object was deleted, items not modified";
        emitResult();
        return;
    }
    std::vector<Feed *> feeds;

    for (auto it = m_keepFlags.cbegin(), end = m_keepFlags.cend(); it != end; ++it) {
        const ArticleId &id = it.key();
        Feed *feed = m_feedList->findByURL(id.feedUrl);
        if (!feed) {
            continue;
        }
        feed->setNotificationMode(false);
        feeds.push_back(feed);
        Article article = feed->findArticle(id.guid);
        if (!article.isNull()) {
            article.setKeep(it.value());
        }
    }

    for (auto it = m_status.cbegin(), end = m_status.cend(); it != end; ++it) {
        const ArticleId &id = it.key();
        Feed *feed = m_feedList->findByURL(id.feedUrl);
        if (!feed) {
            continue;
        }
        feed->setNotificationMode(false);
        feeds.push_back(feed);
        Article article = feed->findArticle(id.guid);
        if (!article.isNull()) {
            article.setStatus(it.value());
        }
    }

    for (Feed *const i : qAsConst(feeds)) {
        i->setNotificationMode(true);
    }
    emitResult();
}

CompositeJob::CompositeJob(QObject *parent)
    : KCompositeJob(parent)
{
}

bool CompositeJob::addSubjob(KJob *job)
{
    return KCompositeJob::addSubjob(job);
}

void CompositeJob::start()
{
    if (subjobs().isEmpty()) {
        emitResult();
        return;
    }
    const auto jobs = subjobs();
    for (KJob *const i : jobs) {
        i->start();
    }
}

ArticleListJob::ArticleListJob(TreeNode *p)
    : KJob(p)
    , m_node(p)
{
}

void ArticleListJob::start()
{
    QTimer::singleShot(20, this, &ArticleListJob::doList);
}

void ArticleListJob::doList()
{
    if (m_node) {
        m_articles = m_node->articles();
    } else {
        setError(ListingFailed);
        setErrorText(i18n("The feed to be listed was already removed."));
    }
    emitResult();
}

TreeNode *ArticleListJob::node() const
{
    return m_node;
}

QVector<Article> ArticleListJob::articles() const
{
    return m_articles;
}
