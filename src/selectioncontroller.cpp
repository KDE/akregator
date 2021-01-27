/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2007 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "selectioncontroller.h"

#include "actionmanager.h"
#include "akregatorconfig.h"
#include "article.h"
#include "articlejobs.h"
#include "articlemodel.h"
#include "feedlist.h"
#include "subscriptionlistmodel.h"
#include "treenode.h"

#include "akregator_debug.h"

#include <QAbstractItemView>
#include <QMenu>
#include <QTreeView>
#include <memory>
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

static QVector<Akregator::Article> articlesForIndexes(const QModelIndexList &indexes, FeedList *feedList)
{
    QVector<Akregator::Article> articles;
    for (const QModelIndex &i : indexes) {
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
        return nullptr;
    }

    return feedList->findByID(index.data(Akregator::SubscriptionListModel::SubscriptionIdRole).toInt());
}
} // anon namespace

SelectionController::SelectionController(QObject *parent)
    : AbstractSelectionController(parent)
    , m_feedList()
    , m_feedSelector()
    , m_subscriptionModel(new FilterUnreadProxyModel(this))
    , m_selectedSubscription()
{
    m_subscriptionModel->setDoFilter(Settings::hideReadFeeds());
    m_subscriptionModel->setSourceModel(new SubscriptionListModel(QSharedPointer<FeedList>(), this));

    connect(m_subscriptionModel, &FilterUnreadProxyModel::dataChanged, this, &SelectionController::subscriptionDataChanged);
}

SelectionController::~SelectionController()
{
    delete m_articleModel;
}

void SelectionController::setFeedSelector(QAbstractItemView *feedSelector)
{
    if (m_feedSelector == feedSelector) {
        return;
    }

    if (m_feedSelector) {
        m_feedSelector->disconnect(this);
        m_feedSelector->selectionModel()->disconnect(this);
        m_feedSelector->selectionModel()->disconnect(m_subscriptionModel);
    }

    m_feedSelector = feedSelector;

    if (!m_feedSelector) {
        return;
    }

    m_feedSelector->setModel(m_subscriptionModel);
    m_subscriptionModel->clearCache();

    connect(m_feedSelector.data(), &QAbstractItemView::customContextMenuRequested, this, &SelectionController::subscriptionContextMenuRequested);
    connect(m_feedSelector->selectionModel(), &QItemSelectionModel::currentChanged, this, &SelectionController::selectedSubscriptionChanged);
    connect(m_feedSelector.data(), &QAbstractItemView::activated, this, &SelectionController::selectedSubscriptionChanged);
    connect(m_feedSelector->selectionModel(), &QItemSelectionModel::selectionChanged, m_subscriptionModel, &FilterUnreadProxyModel::selectionChanged);
}

void SelectionController::setArticleLister(Akregator::ArticleLister *lister)
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
        connect(m_articleLister->itemView(), &QAbstractItemView::doubleClicked, this, &SelectionController::articleIndexDoubleClicked);
    }
}

void SelectionController::setSingleArticleDisplay(Akregator::SingleArticleDisplay *display)
{
    m_singleDisplay = display;
}

Akregator::Article SelectionController::currentArticle() const
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

QVector<Akregator::Article> SelectionController::selectedArticles() const
{
    if (!m_articleLister || !m_articleLister->articleSelectionModel()) {
        return QVector<Akregator::Article>();
    }
    return ::articlesForIndexes(m_articleLister->articleSelectionModel()->selectedRows(), m_feedList.data());
}

Akregator::TreeNode *SelectionController::selectedSubscription() const
{
    return ::subscriptionForIndex(m_feedSelector->selectionModel()->currentIndex(), m_feedList.data());
}

void SelectionController::setFeedList(const QSharedPointer<FeedList> &list)
{
    if (m_feedList == list) {
        return;
    }

    m_feedList = list;
    auto *m = qobject_cast<SubscriptionListModel *>(m_subscriptionModel->sourceModel());
    std::unique_ptr<SubscriptionListModel> oldModel(m);
    m_subscriptionModel->setSourceModel(new SubscriptionListModel(m_feedList, this));

    if (m_folderExpansionHandler) {
        m_folderExpansionHandler->setFeedList(m_feedList);
        m_folderExpansionHandler->setModel(m_subscriptionModel);
    }

    if (m_feedSelector) {
        if (m_feedList) {
            m_feedSelector->setModel(m_subscriptionModel);
            disconnect(m_feedSelector->selectionModel(), &QItemSelectionModel::currentChanged, this, &SelectionController::selectedSubscriptionChanged);
            connect(m_feedSelector->selectionModel(), &QItemSelectionModel::currentChanged, this, &SelectionController::selectedSubscriptionChanged);
        } else {
            disconnect(m_feedSelector->selectionModel(), &QItemSelectionModel::currentChanged, this, &SelectionController::selectedSubscriptionChanged);
        }
    }
}

void SelectionController::setFolderExpansionHandler(Akregator::FolderExpansionHandler *handler)
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

void SelectionController::articleHeadersAvailable(KJob *job)
{
    Q_ASSERT(job);
    Q_ASSERT(job == m_listJob);

    if (job->error()) {
        qCWarning(AKREGATOR_LOG) << job->errorText();
        return;
    }
    TreeNode *const node = m_listJob->node();

    Q_ASSERT(node); // if there was no error, the node must still exist
    Q_ASSERT(node == m_selectedSubscription); //...and equal the previously selected node

    ArticleModel *const newModel = new ArticleModel(m_listJob->articles());

    connect(node, &QObject::destroyed, newModel, &ArticleModel::clear);
    connect(node, &TreeNode::signalArticlesAdded, newModel, &ArticleModel::articlesAdded);
    connect(node, &TreeNode::signalArticlesRemoved, newModel, &ArticleModel::articlesRemoved);
    connect(node, &TreeNode::signalArticlesUpdated, newModel, &ArticleModel::articlesUpdated);

    m_articleLister->setIsAggregation(node->isAggregation());
    m_articleLister->setArticleModel(newModel);
    delete m_articleModel; // order is important: do not delete the old model before the new model is set in the view
    m_articleModel = newModel;

    disconnect(m_articleLister->articleSelectionModel(), &QItemSelectionModel::selectionChanged, this, &SelectionController::articleSelectionChanged);
    connect(m_articleLister->articleSelectionModel(), &QItemSelectionModel::selectionChanged, this, &SelectionController::articleSelectionChanged);

    m_articleLister->setScrollBarPositions(node->listViewScrollBarPositions());
}

void SelectionController::subscriptionDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (!Settings::autoExpandFolders()) {
        return;
    }

    if (!m_subscriptionModel) {
        qCCritical(AKREGATOR_LOG) << "m_subscriptionModel is NULL";
        return;
    }

    // need access to setExpanded
    auto *tv = qobject_cast<QTreeView *>(m_feedSelector);
    if (!tv) {
        qCCritical(AKREGATOR_LOG) << "Unable to cast m_feedSelector to QTreeView";
        return;
    }

    int startRow = topLeft.row();
    int endRow = bottomRight.row();
    QModelIndex parent = topLeft.parent();

    for (int row = startRow; row <= endRow; ++row) {
        QModelIndex idx = m_subscriptionModel->index(row, 0, parent);
        QVariant v = m_subscriptionModel->data(idx, SubscriptionListModel::HasUnreadRole);
        if (!v.toBool()) {
            return;
        }
        tv->setExpanded(idx, true);
    }
}

void SelectionController::selectedSubscriptionChanged(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    if (m_selectedSubscription && m_articleLister) {
        m_selectedSubscription->setListViewScrollBarPositions(m_articleLister->scrollBarPositions());
    }

    m_selectedSubscription = selectedSubscription();
    Q_EMIT currentSubscriptionChanged(m_selectedSubscription);

    // using a timer here internally to simulate async data fetching (which is still synchronous),
    // to ensure the UI copes with async behavior later on

    if (m_listJob) {
        m_listJob->disconnect(this); // Ignore if ~KJob() emits finished()
        delete m_listJob;
    }

    if (!m_selectedSubscription) {
        return;
    }

    auto *const job(new ArticleListJob(m_selectedSubscription));
    connect(job, &KJob::finished, this, &SelectionController::articleHeadersAvailable);
    m_listJob = job;
    m_listJob->start();
}

void SelectionController::subscriptionContextMenuRequested(const QPoint &point)
{
    Q_ASSERT(m_feedSelector);
    const TreeNode *const node = ::subscriptionForIndex(m_feedSelector->indexAt(point), m_feedList.data());
    if (!node) {
        return;
    }

    QWidget *w = ActionManager::getInstance()->container(node->isGroup() ? QStringLiteral("feedgroup_popup") : QStringLiteral("feeds_popup"));
    auto *popup = qobject_cast<QMenu *>(w);
    if (popup) {
        const QPoint globalPos = m_feedSelector->viewport()->mapToGlobal(point);
        popup->exec(globalPos);
    }
}

void SelectionController::articleSelectionChanged()
{
    const Akregator::Article article = currentArticle();
    if (m_singleDisplay) {
        m_singleDisplay->showArticle(article);
    }
    Q_EMIT currentArticleChanged(article);
}

void SelectionController::articleIndexDoubleClicked(const QModelIndex &index)
{
    const Akregator::Article article = ::articleForIndex(index, m_feedList.data());
    Q_EMIT articleDoubleClicked(article);
}

/**
 * Called when the applications settings are changed; sets whether we apply a the filter or not.
 */
void SelectionController::settingsChanged()
{
    m_subscriptionModel->setDoFilter(Settings::hideReadFeeds());
}

void SelectionController::setFilters(const std::vector<QSharedPointer<const Filters::AbstractMatcher>> &matchers)
{
    Q_ASSERT(m_articleLister);
    m_articleLister->setFilters(matchers);
}

void SelectionController::forceFilterUpdate()
{
    Q_ASSERT(m_articleLister);
    m_articleLister->forceFilterUpdate();
}
