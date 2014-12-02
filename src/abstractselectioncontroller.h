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
#ifndef AKREGATOR_ABSTRACTSELECTIONCONTROLLER_H
#define AKREGATOR_ABSTRACTSELECTIONCONTROLLER_H

#include <QObject>

#include <boost/shared_ptr.hpp>
#include <vector>

class QAbstractItemView;
class QItemSelectionModel;
class QPoint;
class QModelIndex;

namespace Akregator
{

class Article;
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

    virtual ~ArticleLister() {}

    virtual void setArticleModel(Akregator::ArticleModel *model) = 0;

    virtual QItemSelectionModel *articleSelectionModel() const = 0;

    virtual void setIsAggregation(bool isAggregation) = 0;

    virtual void setFilters(const std::vector<boost::shared_ptr<const Filters::AbstractMatcher> > &) = 0;

    virtual void forceFilterUpdate() = 0;

    virtual QPoint scrollBarPositions() const = 0;

    virtual void setScrollBarPositions(const QPoint &p) = 0;

    virtual const QAbstractItemView *itemView() const = 0;

    virtual QAbstractItemView *itemView() = 0;
};

class SingleArticleDisplay
{
public:
    virtual ~SingleArticleDisplay() {}

    virtual void showArticle(const Akregator::Article &article) = 0;
};

class SubscriptionLister
{
public:
    virtual ~SubscriptionLister() {}

    virtual void setSubscriptionListModel(Akregator::SubscriptionListModel *model) = 0;
    virtual QItemSelectionModel *subscriptionSelectionModel() const = 0;
};

class AbstractSelectionController : public QObject
{
    Q_OBJECT

public:
    explicit AbstractSelectionController(QObject *parent = Q_NULLPTR);
    virtual ~AbstractSelectionController();

    virtual void setFeedList(const boost::shared_ptr<FeedList> &list) = 0;

    virtual void setFeedSelector(QAbstractItemView *feedSelector) = 0;

    virtual void setArticleLister(Akregator::ArticleLister *lister) = 0;

    virtual void setFolderExpansionHandler(Akregator::FolderExpansionHandler *handler) = 0;

    virtual void setSingleArticleDisplay(Akregator::SingleArticleDisplay *display) = 0;

    virtual Akregator::Article currentArticle() const = 0;

    virtual QModelIndex currentArticleIndex() const = 0;

    virtual QList<Akregator::Article> selectedArticles() const = 0;

    virtual Akregator::TreeNode *selectedSubscription() const = 0;

public Q_SLOTS:

    virtual void setFilters(const std::vector<boost::shared_ptr<const Akregator::Filters::AbstractMatcher> > &) = 0;

    virtual void forceFilterUpdate() = 0;

Q_SIGNALS:
    void currentSubscriptionChanged(Akregator::TreeNode *node);

    void currentArticleChanged(const Akregator::Article &);

    void articleDoubleClicked(const Akregator::Article &);
};

} // namespace Akregator

#endif // AKREGATOR_ABSTRACTSELECTIONCONTROLLER_H
