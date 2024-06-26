/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Teemu Rytilahti <tpr@d5k.net>
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "articleviewerwidget.h"

#include "actionmanager.h"
#include "akregator_debug.h"
#include "akregatorconfig.h"
#include "articleformatter.h"
#include "articlejobs.h"
#include "feed.h"
#include "openurlrequest.h"
#include "treenode.h"

#include <KActionCollection>

#include <QElapsedTimer>

#include "articleviewer-ng/webengine/articlehtmlwebenginewriter.h"
#include "articleviewer-ng/webengine/articleviewerwebengine.h"
#include "articleviewer-ng/webengine/articleviewerwebenginewidgetng.h"
#include "defaultnormalviewformatter.h"
#include <QGridLayout>
#include <QKeyEvent>

#include "defaultcombinedviewformatter.h"

using namespace Akregator;
using namespace Akregator::Filters;

ArticleViewerWidget::ArticleViewerWidget(KActionCollection *ac, QWidget *parent)
    : QWidget(parent)
    , m_node(nullptr)
    , m_articleViewerWidgetNg(new Akregator::ArticleViewerWebEngineWidgetNg(ac, this))
{
    auto layout = new QGridLayout(this);
    layout->setContentsMargins({});
    layout->addWidget(m_articleViewerWidgetNg);
    m_articleHtmlWriter = new Akregator::ArticleHtmlWebEngineWriter(m_articleViewerWidgetNg->articleViewerNg(), this);
    connect(m_articleViewerWidgetNg->articleViewerNg(), &ArticleViewerWebEngine::signalOpenUrlRequest, this, &ArticleViewerWidget::signalOpenUrlRequest);
    connect(m_articleViewerWidgetNg->articleViewerNg(), &ArticleViewerWebEngine::showStatusBarMessage, this, &ArticleViewerWidget::showStatusBarMessage);
}

ArticleViewerWidget::~ArticleViewerWidget() = default;

QSharedPointer<ArticleFormatter> ArticleViewerWidget::normalViewFormatter()
{
    if (!m_normalViewFormatter.data()) {
        m_normalViewFormatter = QSharedPointer<ArticleFormatter>(new DefaultNormalViewFormatter(m_articleViewerWidgetNg->articleViewerNg()));
    }
    return m_normalViewFormatter;
}

QSharedPointer<ArticleFormatter> ArticleViewerWidget::combinedViewFormatter()
{
    if (!m_combinedViewFormatter.data()) {
        m_combinedViewFormatter = QSharedPointer<ArticleFormatter>(new DefaultCombinedViewFormatter(m_articleViewerWidgetNg->articleViewerNg()));
    }
    return m_combinedViewFormatter;
}

void ArticleViewerWidget::slotZoomChangeInFrame(qreal value)
{
    m_articleViewerWidgetNg->articleViewerNg()->setZoomFactor(value);
}

void ArticleViewerWidget::slotCopy()
{
    m_articleViewerWidgetNg->articleViewerNg()->slotCopy();
}

void ArticleViewerWidget::slotSelectionChanged()
{
    ActionManager::getInstance()->action(QStringLiteral("viewer_copy"))->setEnabled(!m_articleViewerWidgetNg->articleViewerNg()->selectedText().isEmpty());
}

void ArticleViewerWidget::slotPrint()
{
    m_articleViewerWidgetNg->slotPrint();
}

void ArticleViewerWidget::slotPrintPreview()
{
    m_articleViewerWidgetNg->slotPrintPreview();
}

void ArticleViewerWidget::connectToNode(TreeNode *node)
{
    if (node) {
        if (m_viewMode == CombinedView) {
            connect(node, &TreeNode::signalChanged, this, &ArticleViewerWidget::slotUpdateCombinedView);
            connect(node, &TreeNode::signalArticlesAdded, this, &ArticleViewerWidget::slotArticlesAdded);
            connect(node, &TreeNode::signalArticlesRemoved, this, &ArticleViewerWidget::slotArticlesRemoved);
            connect(node, &TreeNode::signalArticlesUpdated, this, &ArticleViewerWidget::slotArticlesUpdated);
        } else if (m_viewMode == SummaryView) {
            connect(node, &TreeNode::signalChanged, this, &ArticleViewerWidget::slotShowSummary);
        }

        connect(node, &TreeNode::signalDestroyed, this, &ArticleViewerWidget::slotClear);
    }
}

void ArticleViewerWidget::disconnectFromNode(TreeNode *node)
{
    if (node) {
        node->disconnect(this);
    }
}

void ArticleViewerWidget::renderContent(const QString &text)
{
    m_currentText = text;
    reload();
}

void ArticleViewerWidget::beginWriting()
{
    m_articleHtmlWriter->begin();
}

void ArticleViewerWidget::endWriting()
{
    m_articleHtmlWriter->end();
}

void ArticleViewerWidget::slotShowSummary(TreeNode *node)
{
    m_viewMode = SummaryView;

    if (!node) {
        slotClear();
        return;
    }

    if (node != m_node) {
        disconnectFromNode(m_node);
        connectToNode(node);
        m_node = node;
    }

    const QString summary = normalViewFormatter()->formatSummary(node);
    m_link.clear();
    renderContent(summary);

    setArticleActionsEnabled(false);
}

void ArticleViewerWidget::showArticle(const Akregator::Article &article)
{
    if (article.isNull() || article.isDeleted()) {
        slotClear();
        return;
    }

    const QUrl xmlUrl = QUrl(article.feed()->xmlUrl());
    qCDebug(AKREGATOR_LOG) << "showing Article - xmlUrl:" << xmlUrl;
    m_articleHtmlWriter->setBaseUrl(xmlUrl);
    m_viewMode = NormalView;
    disconnectFromNode(m_node);
    m_article = article;
    m_node = nullptr;
    m_link = article.link();
    if (article.feed()->loadLinkedWebsite()) {
        openUrl(article.link());
    } else {
        renderContent(normalViewFormatter()->formatArticles(QList<Akregator::Article>() << article, ArticleFormatter::ShowIcon));
    }

    setArticleActionsEnabled(true);
}

bool ArticleViewerWidget::openUrl(const QUrl &url)
{
    if (!m_article.isNull() && m_article.feed()->loadLinkedWebsite()) {
        m_articleViewerWidgetNg->articleViewerNg()->load(url);
    } else {
        reload();
    }
    return true;
}

void ArticleViewerWidget::setFilters(const std::vector<QSharedPointer<const AbstractMatcher>> &filters)
{
    if (filters == m_filters) {
        return;
    }

    m_filters = filters;

    slotUpdateCombinedView();
}

void ArticleViewerWidget::slotUpdateCombinedView()
{
    if (m_viewMode != CombinedView) {
        return;
    }

    if (!m_node) {
        return slotClear();
    }

    m_articleViewerWidgetNg->saveCurrentPosition();
    QString text;

    int num = 0;
    QElapsedTimer spent;
    spent.start();

    const auto filterEnd = m_filters.cend();

    QList<Article> articles;
    for (const Article &i : std::as_const(m_articles)) {
        if (i.isDeleted()) {
            continue;
        }

        auto func = [i](const QSharedPointer<const Filters::AbstractMatcher> &matcher) -> bool {
            return !matcher->matches(i);
        };
        if (std::find_if(m_filters.cbegin(), filterEnd, func) != filterEnd) {
            continue;
        }
        articles << i;
        ++num;
    }
    text = combinedViewFormatter()->formatArticles(articles, ArticleFormatter::NoIcon);

    qCDebug(AKREGATOR_LOG) << "Combined view rendering: (" << num << " articles):"
                           << "generating HTML:" << spent.elapsed() << "ms";
    renderContent(text);
    qCDebug(AKREGATOR_LOG) << "HTML rendering:" << spent.elapsed() << "ms";
}

void ArticleViewerWidget::slotArticlesUpdated(TreeNode * /*node*/, const QList<Article> & /*list*/)
{
    if (m_viewMode == CombinedView) {
        slotUpdateCombinedView();
    }
}

void ArticleViewerWidget::slotArticlesAdded(TreeNode * /*node*/, const QList<Article> &list)
{
    if (m_viewMode == CombinedView) {
        // TODO sort list, then merge
        m_articles << list;
        std::sort(m_articles.begin(), m_articles.end());
        slotUpdateCombinedView();
    }
}

void ArticleViewerWidget::slotArticlesRemoved(TreeNode * /*node*/, const QList<Article> &list)
{
    Q_UNUSED(list)

    if (m_viewMode == CombinedView) {
        slotUpdateCombinedView();
    }
}

void ArticleViewerWidget::slotClear()
{
    disconnectFromNode(m_node);
    m_node = nullptr;
    m_article = Article();
    m_articles.clear();

    renderContent(QString());
}

void ArticleViewerWidget::showNode(TreeNode *node)
{
    m_viewMode = CombinedView;

    if (node != m_node) {
        disconnectFromNode(m_node);
    }

    connectToNode(node);

    m_articles.clear();
    m_article = Article();
    m_node = node;

    delete m_listJob;

    m_listJob = node->createListJob();
    connect(m_listJob.data(), &ArticleListJob::finished, this, &ArticleViewerWidget::slotArticlesListed);
    m_listJob->start();

    slotUpdateCombinedView();
}

qreal ArticleViewerWidget::zoomFactor() const
{
    return m_articleViewerWidgetNg->articleViewerNg()->zoomFactor();
}

void ArticleViewerWidget::slotArticlesListed(KJob *job)
{
    Q_ASSERT(job);
    Q_ASSERT(job == m_listJob);

    TreeNode *node = m_listJob->node();

    if (job->error() || !node) {
        if (!node) {
            qCWarning(AKREGATOR_LOG) << "Node to be listed is already deleted";
        } else {
            qCWarning(AKREGATOR_LOG) << job->errorText();
        }
        slotUpdateCombinedView();
        return;
    }

    m_articles = m_listJob->articles();
    std::sort(m_articles.begin(), m_articles.end());

    if (!m_articles.isEmpty()) {
        m_link = m_articles.first().link();
    } else {
        m_link = QUrl();
    }

    slotUpdateCombinedView();
}

void ArticleViewerWidget::keyPressEvent(QKeyEvent *e)
{
    e->ignore();
}

void ArticleViewerWidget::updateAfterConfigChanged()
{
    switch (m_viewMode) {
    case NormalView:
        if (!m_article.isNull()) {
            renderContent(normalViewFormatter()->formatArticles(QList<Akregator::Article>() << m_article, ArticleFormatter::ShowIcon));
        }
        break;
    case CombinedView:
        slotUpdateCombinedView();
        break;
    case SummaryView:
        slotShowSummary(m_node);
        break;
    }
    m_articleViewerWidgetNg->articleViewerNg()->updateSecurity();
}

void ArticleViewerWidget::reload()
{
    beginWriting();
    m_articleHtmlWriter->queue(m_currentText);
    endWriting();
}

QSize ArticleViewerWidget::sizeHint() const
{
    // Increase height a bit so that we can (roughly) read 25 lines of text
    QSize sh = QWidget::sizeHint();
    sh.setHeight(qMax(sh.height(), 25 * fontMetrics().height()));
    return sh;
}

void ArticleViewerWidget::displayAboutPage()
{
    m_articleViewerWidgetNg->articleViewerNg()->showAboutPage();
}

void ArticleViewerWidget::setArticleActionsEnabled(bool enabled)
{
    ActionManager::getInstance()->setArticleActionsEnabled(enabled);
}

Akregator::ArticleViewerWebEngineWidgetNg *ArticleViewerWidget::articleViewerWidgetNg() const
{
    return m_articleViewerWidgetNg;
}

#include "moc_articleviewerwidget.cpp"
