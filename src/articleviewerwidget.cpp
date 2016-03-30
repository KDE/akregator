/*
    This file is part of Akregator.

    Copyright (C) 2004 Teemu Rytilahti <tpr@d5k.net>
                  2005 Frank Osterfeld <osterfeld@kde.org>

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

#include "articleviewerwidget.h"

#include "akregatorconfig.h"
#include "aboutdata.h"
#include "actionmanager.h"
#include "actions.h"
#include "article.h"
#include "articleformatter.h"
#include "articlejobs.h"
#include "articlematcher.h"
#include "feed.h"
#include "folder.h"
#include "treenode.h"
#include "utils.h"
#include "openurlrequest.h"
#include "akregator_debug.h"
#include "kdepim-version.h"

#include <QAction>
#include <kactioncollection.h>
#include <KLocalizedString>
#include <QMenu>
#include <kmessagebox.h>
#include <krun.h>
#include <kshell.h>

#include <kstandardaction.h>
#include <ktoolinvocation.h>
#include <KParts/BrowserRun>
#include <kio/job.h>
#include <QUrl>
#include <grantleetheme/grantleethememanager.h>

#ifdef QTWEBENGINE_SUPPORT_OPTION
#include <articleviewer-ng/webengine/articleviewerwebenginewidgetng.h>
#include <articleviewer-ng/webengine/articleviewerwebengine.h>
#include <articleviewer-ng/webengine/articlehtmlwebenginewriter.h>
#else
#include <articleviewer-ng/webkit/articleviewerwidgetng.h>
#include <articleviewer-ng/webkit/articleviewerng.h>
#include "articleviewer-ng/webkit/articlehtmlwriter.h"
#endif
#include <QClipboard>
#include <QGridLayout>
#include <QKeyEvent>
#include <QApplication>
#include <defaultnormalviewformatter.h>

#include <QStandardPaths>
#include <defaultcombinedviewformatter.h>

using namespace Akregator;
using namespace Akregator::Filters;

using namespace Akregator;

ArticleViewerWidget::ArticleViewerWidget(const QString &grantleeDirectory, KActionCollection *ac, QWidget *parent)
    : QWidget(parent),
      m_imageDir(QUrl::fromLocalFile(QString(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/akregator/Media/")))),
      m_node(0),
      m_viewMode(NormalView),
#ifdef QTWEBENGINE_SUPPORT_OPTION
      m_articleViewerWidgetNg(new Akregator::ArticleViewerWebEngineWidgetNg(ac, this)),
#else
      m_articleViewerWidgetNg(new Akregator::ArticleViewerWidgetNg(ac, this)),
#endif
      m_normalViewFormatter(new DefaultNormalViewFormatter(grantleeDirectory, m_imageDir, m_articleViewerWidgetNg->articleViewerNg())),
      m_combinedViewFormatter(new DefaultCombinedViewFormatter(grantleeDirectory, m_imageDir, m_articleViewerWidgetNg->articleViewerNg()))
{
    QGridLayout *layout = new QGridLayout(this);
    layout->setMargin(0);
    layout->addWidget(m_articleViewerWidgetNg);
#ifdef QTWEBENGINE_SUPPORT_OPTION
    m_articleHtmlWriter = new Akregator::ArticleHtmlWebEngineWriter(m_articleViewerWidgetNg->articleViewerNg(), this);
    connect(m_articleViewerWidgetNg->articleViewerNg(), &ArticleViewerWebEngine::signalOpenUrlRequest, this, &ArticleViewerWidget::signalOpenUrlRequest);
    connect(m_articleViewerWidgetNg->articleViewerNg(), &ArticleViewerWebEngine::showStatusBarMessage, this, &ArticleViewerWidget::showStatusBarMessage);
#else
    m_articleHtmlWriter = new Akregator::ArticleHtmlWriter(m_articleViewerWidgetNg->articleViewerNg(), this);
    connect(m_articleViewerWidgetNg->articleViewerNg(), &ArticleViewerNg::signalOpenUrlRequest, this, &ArticleViewerWidget::signalOpenUrlRequest);
    connect(m_articleViewerWidgetNg->articleViewerNg(), &ArticleViewerNg::showStatusBarMessage, this, &ArticleViewerWidget::showStatusBarMessage);
#endif
}

ArticleViewerWidget::~ArticleViewerWidget()
{
}

int ArticleViewerWidget::pointsToPixel(int pointSize) const
{
    return (pointSize * m_articleViewerWidgetNg->articleViewerNg()->logicalDpiY() + 36) / 72;
}

void ArticleViewerWidget::slotZoomChangeInFrame(qreal value)
{
    m_articleViewerWidgetNg->articleViewerNg()->setZoomFactor(value);
}

void ArticleViewerWidget::slotZoomTextOnlyInFrame(bool textOnlyInFrame)
{
    Q_UNUSED(textOnlyInFrame)
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
    m_articleViewerWidgetNg->articleViewerNg()->slotPrint();
}

void ArticleViewerWidget::slotPrintPreview()
{
    m_articleViewerWidgetNg->articleViewerNg()->slotPrintPreview();
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
    beginWriting();
    m_articleHtmlWriter->queue(text);
    endWriting();
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

    QString summary = m_normalViewFormatter->formatSummary(node);
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

    m_viewMode = NormalView;
    disconnectFromNode(m_node);
    m_article = article;
    m_node = 0;
    m_link = article.link();
    if (article.feed()->loadLinkedWebsite()) {
        openUrl(article.link());
    } else {
        renderContent(m_normalViewFormatter->formatArticles(QVector<Akregator::Article>() << article, ArticleFormatter::ShowIcon));
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

void ArticleViewerWidget::setFilters(const std::vector< QSharedPointer<const AbstractMatcher> > &filters)
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

    QString text;

    int num = 0;
    QTime spent;
    spent.start();

    const std::vector< QSharedPointer<const AbstractMatcher> >::const_iterator filterEnd = m_filters.cend();

    QVector<Article> articles;
    Q_FOREACH (const Article &i, m_articles) {
        if (i.isDeleted()) {
            continue;
        }

        auto func = [i](const QSharedPointer<const Filters::AbstractMatcher> &matcher) -> bool { return !matcher->matches(i); };
        if (std::find_if(m_filters.cbegin(), filterEnd, func) != filterEnd) {
            continue;
        }
        articles << i;
        ++num;
    }
    text = m_combinedViewFormatter->formatArticles(articles, ArticleFormatter::NoIcon);

    qCDebug(AKREGATOR_LOG) << "Combined view rendering: (" << num << " articles):" << "generating HTML:" << spent.elapsed() << "ms";
    renderContent(text);
    qCDebug(AKREGATOR_LOG) << "HTML rendering:" << spent.elapsed() << "ms";
}

void ArticleViewerWidget::slotArticlesUpdated(TreeNode * /*node*/, const QVector<Article> & /*list*/)
{
    if (m_viewMode == CombinedView) {
        //TODO
        slotUpdateCombinedView();
    }
}

void ArticleViewerWidget::slotArticlesAdded(TreeNode * /*node*/, const QVector<Article> &list)
{
    if (m_viewMode == CombinedView) {
        //TODO sort list, then merge
        m_articles << list;
        std::sort(m_articles.begin(), m_articles.end());
        slotUpdateCombinedView();
    }
}

void ArticleViewerWidget::slotArticlesRemoved(TreeNode * /*node*/, const QVector<Article> &list)
{
    Q_UNUSED(list)

    if (m_viewMode == CombinedView) {
        //TODO
        slotUpdateCombinedView();
    }
}

void ArticleViewerWidget::slotClear()
{
    disconnectFromNode(m_node);
    m_node = 0;
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

bool ArticleViewerWidget::zoomTextOnlyInFrame() const
{
    return false;
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

    if (node && !m_articles.isEmpty()) {
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

void ArticleViewerWidget::slotPaletteOrFontChanged()
{
    reload();
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

#ifdef QTWEBENGINE_SUPPORT_OPTION
Akregator::ArticleViewerWebEngineWidgetNg *ArticleViewerWidget::articleViewerWidgetNg() const
{
    return m_articleViewerWidgetNg;
}
#else
Akregator::ArticleViewerWidgetNg *ArticleViewerWidget::articleViewerWidgetNg() const
{
    return m_articleViewerWidgetNg;
}
#endif
