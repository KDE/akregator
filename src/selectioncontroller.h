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
#ifndef AKREGATOR_SELECTIONCONTROLLER_H
#define AKREGATOR_SELECTIONCONTROLLER_H

#include "abstractselectioncontroller.h"

#include <QPointer>
#include <QAbstractItemModel>

class QModelIndex;
class QPoint;

class KJob;

namespace Akregator {
class ArticleListJob;
class FilterUnreadProxyModel;

class SelectionController : public AbstractSelectionController
{
    Q_OBJECT

public:

    explicit SelectionController(QObject *parent = nullptr);
    ~SelectionController() override;

    //impl
    void setFeedSelector(QAbstractItemView *feedSelector)  override;

    //impl
    void setArticleLister(Akregator::ArticleLister *lister) override;

    //impl
    Q_REQUIRED_RESULT Akregator::Article currentArticle() const override;

    //impl
    Q_REQUIRED_RESULT QModelIndex currentArticleIndex() const override;

    //impl
    Q_REQUIRED_RESULT QVector<Akregator::Article> selectedArticles() const override;

    //impl
    void setSingleArticleDisplay(Akregator::SingleArticleDisplay *display) override;

    //impl
    Akregator::TreeNode *selectedSubscription() const override;

    //impl
    void setFeedList(const QSharedPointer<FeedList> &list) override;

    //impl
    void setFolderExpansionHandler(Akregator::FolderExpansionHandler *handler) override;

public Q_SLOTS:

    //impl
    void settingsChanged() override;

    //impl
    void setFilters(const std::vector<QSharedPointer<const Akregator::Filters::AbstractMatcher> > &) override;

    //impl
    void forceFilterUpdate() override;

private Q_SLOTS:

    void subscriptionDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void selectedSubscriptionChanged(const QModelIndex &index);
    void articleSelectionChanged();
    void articleIndexDoubleClicked(const QModelIndex &index);
    void subscriptionContextMenuRequested(const QPoint &point);
    void articleHeadersAvailable(KJob *);

private:

    void setCurrentSubscriptionModel();

    QSharedPointer<FeedList> m_feedList;
    QPointer<QAbstractItemView> m_feedSelector;
    Akregator::ArticleLister *m_articleLister = nullptr;
    Akregator::SingleArticleDisplay *m_singleDisplay = nullptr;
    Akregator::FilterUnreadProxyModel *m_subscriptionModel = nullptr;
    QAbstractItemModel *m_currentModel = nullptr;
    Akregator::FolderExpansionHandler *m_folderExpansionHandler = nullptr;
    Akregator::ArticleModel *m_articleModel = nullptr;
    QPointer<TreeNode> m_selectedSubscription;
    QPointer<ArticleListJob> m_listJob;
};
} // namespace Akregator

#endif // AKREGATOR_SELECTIONCONTROLLER_H
