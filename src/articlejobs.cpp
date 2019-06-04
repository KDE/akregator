/*
    This file is part of Akregator.

    Copyright (C) 2007 Frank Osterfeld <osterfeld@kde.org>

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

void ArticleDeleteJob::appendArticleIds(const QList<ArticleId> &ids)
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

ArticleModifyJob::ArticleModifyJob(QObject *parent) : KJob(parent)
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

CompositeJob::CompositeJob(QObject *parent) : KCompositeJob(parent)
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

ArticleListJob::ArticleListJob(TreeNode *p) : KJob(p)
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
