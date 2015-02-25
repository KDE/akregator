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

#include "selectioncontroller.h"

#include "actionmanager.h"
#include "article.h"
#include "articlejobs.h"
#include "articlemodel.h"
#include "feedlist.h"
#include "subscriptionlistmodel.h"
#include "treenode.h"

#include "akregator_debug.h"

#include <QAbstractItemView>
#include <QMenu>

using namespace Akregator;

namespace
{
static Akregator::Article articleForIndex(const QModelIndex &index, FeedList *feedList)
{
    if (!index.isValid()) {
        return Akregator::Article();
    }

    const QString guid = index.data(ArticleModel::GuidRole).toString();
    const QString feedId = index.data(ArticleModel::FeedIdRole).toString();
    return feedList->findArticle(feedId, guid);
}

static QList<Akregator::Article> articlesForIndexes(const QModelIndexList &indexes, FeedList *feedList)
{
    QList<Akregator::Article> articles;
    Q_FOREACH (const QModelIndex &i, indexes) {
        const Article a = articleForIndex(i, feedList);
        if (a.isNull()) {
            continue;
        }
        articles.append(articleForIndex(i, feedList));
    }

    return articles;
}

static Akregator::TreeNode *subscriptionForIndex(const QModelIndex &index, Akregator::FeedList *feedList)
{
    if (!index.isValid()) {
        return Q_NULLPTR;
    }

    return feedList->findByID(index.data(Akregator::SubscriptionListModel::SubscriptionIdRole).toInt());
}
} // anon namespace

Akregator::SelectionController::SelectionController(QObject *parent)
    : AbstractSelectionController(parent),
      m_feedList(),
      m_feedSelector(),
      m_articleLister(0),
      m_singleDisplay(0),
      m_subscriptionModel(new SubscriptionListModel(QSharedPointer<FeedList>(), this)),
      m_folderExpansionHandler(0),
      m_articleModel(0),
      m_selectedSubscription()
{
}

Akregator::SelectionController::~SelectionController()
{
    delete m_articleModel;
}

void Akregator::SelectionController::setFeedSelector(QAbstractItemView *feedSelector)
{
    if (m_feedSelector == feedSelector) {
        return;
    }

    if (m_feedSelector) {
        m_feedSelector->disconnect(this);
        m_feedSelector->selectionModel()->disconnect(this);
    }

    m_feedSelector = feedSelector;

    if (!m_feedSelector) {
        return;
    }

    m_feedSelector->setModel(m_subscriptionModel);

    connect(m_feedSelector.data(), &QAbstractItemView::customContextMenuRequested, this, &SelectionController::subscriptionContextMenuRequested);
    connect(m_feedSelector->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),  this, SLOT(selectedSubscriptionChanged(QModelIndex)));
    connect(m_feedSelector.data(), &QAbstractItemView::activated, this, &SelectionController::selectedSubscriptionChanged);

}

void Akregator::SelectionController::setArticleLister(Akregator::ArticleLister *lister)
{
    if (m_articleLister == lister) {
        return;
    }

    if (m_articleLister) {
        m_articleLister->articleSelectionModel()->disconnect(this);
    }
    if (m_articleLister && m_articleLister->itemView()) {
        m_articleLister->itemView()->disconnect(this);
    }

    m_articleLister = lister;

    if (m_articleLister && m_articleLister->itemView()) {
        connect(m_articleLister->itemView(), SIGNAL(doubleClicked(QModelIndex)), this, SLOT(articleIndexDoubleClicked(QModelIndex)));
    }
}

void Akregator::SelectionController::setSingleArticleDisplay(Akregator::SingleArticleDisplay *display)
{
    m_singleDisplay = display;
}

Akregator::Article Akregator::SelectionController::currentArticle() const
{
    if (!m_articleLister || !m_articleLister->articleSelectionModel()) {
        return Article();
    }
    return ::articleForIndex(m_articleLister->articleSelectionModel()->currentIndex(), m_feedList.data());
}

QModelIndex SelectionController::currentArticleIndex() const
{
    return m_articleLister->articleSelectionModel()->currentIndex();
}

QList<Akregator::Article> Akregator::SelectionController::selectedArticles() const
{
    if (!m_articleLister || !m_articleLister->articleSelectionModel()) {
        return QList<Akregator::Article>();
    }
    return ::articlesForIndexes(m_articleLister->articleSelectionModel()->selectedRows(), m_feedList.data());
}

Akregator::TreeNode *Akregator::SelectionController::selectedSubscription() const
{
    return ::subscriptionForIndex(m_feedSelector->selectionModel()->currentIndex(), m_feedList.data());
}

void Akregator::SelectionController::setFeedList(const QSharedPointer<FeedList> &list)
{
    if (m_feedList == list) {
        return;
    }

    m_feedList = list;
    QScopedPointer<SubscriptionListModel> oldModel(m_subscriptionModel);
    m_subscriptionModel = new SubscriptionListModel(m_feedList, this);

    if (m_folderExpansionHandler) {
        m_folderExpansionHandler->setFeedList(m_feedList);
        m_folderExpansionHandler->setModel(m_subscriptionModel);
    }

    if (m_feedSelector) {
        m_feedSelector->setModel(m_subscriptionModel);
        disconnect(m_feedSelector->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(selectedSubscriptionChanged(QModelIndex)));
        connect(m_feedSelector->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(selectedSubscriptionChanged(QModelIndex)));
    }
}

void Akregator::SelectionController::setFolderExpansionHandler(Akregator::FolderExpansionHandler *handler)
{
    if (handler == m_folderExpansionHandler) {
        return;
    }
    m_folderExpansionHandler = handler;
    if (!m_folderExpansionHandler) {
        return;
    }
    handler->setFeedList(m_feedList);
    handler->setModel(m_subscriptionModel);
}

void Akregator::SelectionController::articleHeadersAvailable(KJob *job)
{
    Q_ASSERT(job);
    Q_ASSERT(job == m_listJob);

    if (job->error()) {
        qCWarning(AKREGATOR_LOG) << job->errorText();
        return;
    }
    TreeNode *const node = m_listJob->node();

    Q_ASSERT(node);   // if there was no error, the node must still exist
    Q_ASSERT(node == m_selectedSubscription);   //...and equal the previously selected node

    ArticleModel *const newModel = new ArticleModel(m_listJob->articles());

    connect(node, SIGNAL(destroyed()), newModel, SLOT(clear()));
    connect(node, SIGNAL(signalArticlesAdded(Akregator::TreeNode*,QList<Akregator::Article>)), newModel, SLOT(articlesAdded(Akregator::TreeNode*,QList<Akregator::Article>)));
    connect(node, SIGNAL(signalArticlesRemoved(Akregator::TreeNode*,QList<Akregator::Article>)), newModel, SLOT(articlesRemoved(Akregator::TreeNode*,QList<Akregator::Article>)));
    connect(node, SIGNAL(signalArticlesUpdated(Akregator::TreeNode*,QList<Akregator::Article>)), newModel, SLOT(articlesUpdated(Akregator::TreeNode*,QList<Akregator::Article>)));

    m_articleLister->setIsAggregation(node->isAggregation());
    m_articleLister->setArticleModel(newModel);
    delete m_articleModel; //order is important: do not delete the old model before the new model is set in the view
    m_articleModel = newModel;

    disconnect(m_articleLister->articleSelectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(articleSelectionChanged()));
    connect(m_articleLister->articleSelectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(articleSelectionChanged()));

    if (node) {
        m_articleLister->setScrollBarPositions(node->listViewScrollBarPositions());
    }
}

void Akregator::SelectionController::selectedSubscriptionChanged(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    if (m_selectedSubscription && m_articleLister) {
        m_selectedSubscription->setListViewScrollBarPositions(m_articleLister->scrollBarPositions());
    }

    m_selectedSubscription = selectedSubscription();
    emit currentSubscriptionChanged(m_selectedSubscription);

    // using a timer here internally to simulate async data fetching (which is still synchronous),
    // to ensure the UI copes with async behavior later on

    if (m_listJob) {
        m_listJob->disconnect(this);   //Ignore if ~KJob() emits finished()
        delete m_listJob;
    }

    if (!m_selectedSubscription) {
        return;
    }

    ArticleListJob *const job(new ArticleListJob(m_selectedSubscription));
    connect(job, SIGNAL(finished(KJob*)),
            this, SLOT(articleHeadersAvailable(KJob*)));
    m_listJob = job;
    m_listJob->start();

}

void Akregator::SelectionController::subscriptionContextMenuRequested(const QPoint &point)
{
    Q_ASSERT(m_feedSelector);
    const TreeNode *const node = ::subscriptionForIndex(m_feedSelector->indexAt(point), m_feedList.data());
    if (!node) {
        return;
    }

    QWidget *w = ActionManager::getInstance()->container(node->isGroup() ? "feedgroup_popup" : "feeds_popup");
    QMenu *popup = qobject_cast<QMenu *>(w);
    if (popup) {
        const QPoint globalPos = m_feedSelector->viewport()->mapToGlobal(point);
        popup->exec(globalPos);
    }
}

void Akregator::SelectionController::articleSelectionChanged()
{
    const Akregator::Article article = currentArticle();
    if (m_singleDisplay) {
        m_singleDisplay->showArticle(article);
    }
    emit currentArticleChanged(article);
}

void Akregator::SelectionController::articleIndexDoubleClicked(const QModelIndex &index)
{
    const Akregator::Article article = ::articleForIndex(index, m_feedList.data());
    emit articleDoubleClicked(article);
}

void SelectionController::setFilters(const std::vector<QSharedPointer<const Filters::AbstractMatcher> > &matchers)
{
    Q_ASSERT(m_articleLister);
    m_articleLister->setFilters(matchers);
}

void SelectionController::forceFilterUpdate()
{
    Q_ASSERT(m_articleLister);
    m_articleLister->forceFilterUpdate();
}

