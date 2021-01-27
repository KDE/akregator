/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2007 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef AKREGATOR_ARTICLE_JOBS_H
#define AKREGATOR_ARTICLE_JOBS_H

#include <KCompositeJob>

#include <QMap>
#include <QPointer>
#include <QString>
#include <QVector>

#include "akregator_export.h"

// transitional job classes
namespace Akregator
{
class Article;
class FeedList;
class TreeNode;

struct ArticleId {
    QString feedUrl;
    QString guid;
    Q_REQUIRED_RESULT bool operator<(const ArticleId &other) const
    {
        return feedUrl < other.feedUrl || (feedUrl == other.feedUrl && guid < other.guid);
    }
};

typedef QVector<Akregator::ArticleId> ArticleIdList;

class AKREGATOR_EXPORT CompositeJob : public KCompositeJob
{
    Q_OBJECT
public:
    explicit CompositeJob(QObject *parent = nullptr);
    Q_REQUIRED_RESULT bool addSubjob(KJob *job) override;
    void start() override;
};

class AKREGATOR_EXPORT ArticleDeleteJob : public KJob
{
    Q_OBJECT
public:
    explicit ArticleDeleteJob(QObject *parent = nullptr);

    void appendArticleIds(const Akregator::ArticleIdList &ids);
    void appendArticleId(const Akregator::ArticleId &id);

    void start() override;

private Q_SLOTS:
    void doStart();

private:
    QSharedPointer<FeedList> m_feedList;
    ArticleIdList m_ids;
};

class AKREGATOR_EXPORT ArticleModifyJob : public KJob
{
    Q_OBJECT
public:
    explicit ArticleModifyJob(QObject *parent = nullptr);

    // TODO replace this by passing modified item later
    void setStatus(const ArticleId &id, int status);
    void setKeep(const ArticleId &id, bool keep);

    void start() override;

private Q_SLOTS:
    void doStart();

private:
    QSharedPointer<FeedList> m_feedList;
    QMap<ArticleId, bool> m_keepFlags;
    QMap<ArticleId, int> m_status;
};

class AKREGATOR_EXPORT ArticleListJob : public KJob
{
    Q_OBJECT
public:
    explicit ArticleListJob(TreeNode *parent = nullptr);

    QVector<Article> articles() const;
    TreeNode *node() const;

    void start() override;

    enum Error { ListingFailed = KJob::UserDefinedError };

private Q_SLOTS:
    void doList();

private:
    const QPointer<TreeNode> m_node;
    QVector<Article> m_articles;
};
} // namespace akregator

#endif // AKREGATOR_ARTICLE_JOBS_H
