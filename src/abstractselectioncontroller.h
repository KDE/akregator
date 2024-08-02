/*
    This file is part of Akregator.

        SPDX-FileCopyrightText: 2007 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#pragma once
#include "article.h"
#include "articlematcher.h"

#include <QObject>

#include <vector>

class QAbstractItemView;
class QItemSelectionModel;
class QPoint;
class QModelIndex;

namespace Akregator
{
class ArticleModel;
class FeedList;
class FolderExpansionHandler;
class SubscriptionListModel;
class TreeNode;

namespace Filters
{
class AbstractMatcher;
}

class ArticleLister
{
public:
    virtual ~ArticleLister() = default;

    virtual void setArticleModel(Akregator::ArticleModel *model) = 0;

    virtual QItemSelectionModel *articleSelectionModel() const = 0;

    virtual void setIsAggregation(bool isAggregation) = 0;

    virtual void setFilters(const std::vector<QSharedPointer<const Filters::AbstractMatcher>> &) = 0;

    virtual void forceFilterUpdate() = 0;

    virtual QPoint scrollBarPositions() const = 0;

    virtual void setScrollBarPositions(const QPoint &p) = 0;

    virtual const QAbstractItemView *itemView() const = 0;

    virtual QAbstractItemView *itemView() = 0;
};

class SingleArticleDisplay
{
public:
    virtual ~SingleArticleDisplay() = default;

    virtual void showArticle(const Akregator::Article &article) = 0;
};

class AbstractSelectionController : public QObject
{
    Q_OBJECT

public:
    explicit AbstractSelectionController(QObject *parent = nullptr);
    ~AbstractSelectionController() override;

    virtual void setFeedList(const QSharedPointer<FeedList> &list) = 0;

    virtual void setFeedSelector(QAbstractItemView *feedSelector) = 0;

    virtual void setArticleLister(Akregator::ArticleLister *lister) = 0;

    virtual void setFolderExpansionHandler(Akregator::FolderExpansionHandler *handler) = 0;

    virtual void setSingleArticleDisplay(Akregator::SingleArticleDisplay *display) = 0;

    virtual Akregator::Article currentArticle() const = 0;

    virtual QModelIndex currentArticleIndex() const = 0;

    virtual QList<Akregator::Article> selectedArticles() const = 0;

    virtual Akregator::TreeNode *selectedSubscription() const = 0;

public Q_SLOTS:

    virtual void settingsChanged() = 0;
    virtual void activitiesChanged() = 0;

    virtual void setFilters(const std::vector<QSharedPointer<const Akregator::Filters::AbstractMatcher>> &) = 0;

    virtual void forceFilterUpdate() = 0;

Q_SIGNALS:
    void currentSubscriptionChanged(Akregator::TreeNode *node);

    void currentArticleChanged(const Akregator::Article &);

    void articleDoubleClicked(const Akregator::Article &);
};
} // namespace Akregator
