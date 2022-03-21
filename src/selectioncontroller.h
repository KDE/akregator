/*
    This file is part of Akregator.

        SPDX-FileCopyrightText: 2007 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#pragma once

#include "abstractselectioncontroller.h"

#include <QAbstractItemModel>
#include <QPointer>

class QModelIndex;
class QPoint;

class KJob;

namespace Akregator
{
class ArticleListJob;
class FilterUnreadProxyModel;

class SelectionController : public AbstractSelectionController
{
    Q_OBJECT

public:
    explicit SelectionController(QObject *parent = nullptr);
    ~SelectionController() override;

    // impl
    void setFeedSelector(QAbstractItemView *feedSelector) override;

    // impl
    void setArticleLister(Akregator::ArticleLister *lister) override;

    // impl
    Q_REQUIRED_RESULT Akregator::Article currentArticle() const override;

    // impl
    Q_REQUIRED_RESULT QModelIndex currentArticleIndex() const override;

    // impl
    Q_REQUIRED_RESULT QVector<Akregator::Article> selectedArticles() const override;

    // impl
    void setSingleArticleDisplay(Akregator::SingleArticleDisplay *display) override;

    // impl
    Akregator::TreeNode *selectedSubscription() const override;

    // impl
    void setFeedList(const QSharedPointer<FeedList> &list) override;

    // impl
    void setFolderExpansionHandler(Akregator::FolderExpansionHandler *handler) override;

public Q_SLOTS:

    // impl
    void settingsChanged() override;

    // impl
    void setFilters(const std::vector<QSharedPointer<const Akregator::Filters::AbstractMatcher>> &) override;

    // impl
    void forceFilterUpdate() override;

private Q_SLOTS:

    void subscriptionDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void selectedSubscriptionChanged(const QModelIndex &index);
    void articleSelectionChanged();
    void articleIndexDoubleClicked(const QModelIndex &index);
    void subscriptionContextMenuRequested(const QPoint &point);
    void articleHeadersAvailable(KJob *);

private:
    QSharedPointer<FeedList> m_feedList;
    QPointer<QAbstractItemView> m_feedSelector;
    Akregator::ArticleLister *m_articleLister = nullptr;
    Akregator::SingleArticleDisplay *m_singleDisplay = nullptr;
    Akregator::FilterUnreadProxyModel *const m_subscriptionModel;
    QAbstractItemModel *m_currentModel = nullptr;
    Akregator::FolderExpansionHandler *m_folderExpansionHandler = nullptr;
    Akregator::ArticleModel *m_articleModel = nullptr;
    QPointer<TreeNode> m_selectedSubscription;
    QPointer<ArticleListJob> m_listJob;
};
} // namespace Akregator
