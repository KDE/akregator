/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Teemu Rytilahti <tpr@d5k.net>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "akregator_export.h"
#include "article.h"
#include <QWidget>

#include <QPointer>

#include <QSharedPointer>
#include <QUrl>
#include <vector>

class KJob;
class KActionCollection;

namespace Akregator
{
namespace Filters
{
class AbstractMatcher;
}

class ArticleFormatter;
class ArticleListJob;
class OpenUrlRequest;
class TreeNode;
class ArticleHtmlWebEngineWriter;
class ArticleViewerWebEngineWidgetNg;

class AKREGATOR_EXPORT ArticleViewerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ArticleViewerWidget(const QString &grantleeDirectory, KActionCollection *ac, QWidget *parent);
    ~ArticleViewerWidget() override;

    /** Repaints the view. */
    void reload();

    void displayAboutPage();

    void showArticle(const Article &article);

    /** Shows the articles of the tree node @c node (combined view).
     * Changes in the node will update the view automatically.
     *
     *  @param node The node to observe */
    void showNode(Akregator::TreeNode *node);

    Q_REQUIRED_RESULT QSize sizeHint() const override;

    Q_REQUIRED_RESULT qreal zoomFactor() const;

    Akregator::ArticleViewerWebEngineWidgetNg *articleViewerWidgetNg() const;

    void updateAfterConfigChanged();

public Q_SLOTS:
    void slotPrint();

    /** Set filters which will be used if the viewer is in combined view mode
     */
    void setFilters(const std::vector<QSharedPointer<const Akregator::Filters::AbstractMatcher>> &filters);

    /** Update view if combined view mode is set. Has to be called when
     * the displayed node gets modified.
     */
    void slotUpdateCombinedView();

    /**
     * Clears the canvas and disconnects from the currently observed node
     * (if in combined view mode).
     */
    void slotClear();

    void slotShowSummary(Akregator::TreeNode *node);

    void slotPrintPreview();
    void slotCopy();

    void slotZoomChangeInFrame(qreal value);
Q_SIGNALS:

    /** This gets emitted when url gets clicked */
    void signalOpenUrlRequest(Akregator::OpenUrlRequest &);
    void showStatusBarMessage(const QString &msg);

    void selectionChanged();

protected: // methods
    bool openUrl(const QUrl &url);

protected Q_SLOTS:
    void slotSelectionChanged();

    void slotArticlesListed(KJob *job);

    void slotArticlesUpdated(Akregator::TreeNode *node, const QVector<Akregator::Article> &list);
    void slotArticlesAdded(Akregator::TreeNode *node, const QVector<Akregator::Article> &list);
    void slotArticlesRemoved(Akregator::TreeNode *node, const QVector<Akregator::Article> &list);

    // from ArticleViewer
private:
    QSharedPointer<ArticleFormatter> combinedViewFormatter();
    QSharedPointer<ArticleFormatter> normalViewFormatter();
    void keyPressEvent(QKeyEvent *e) override;

    /** renders @c body. Use this method whereever possible.
     *  @param body html to render, without header and footer */
    void renderContent(const QString &body);

    /** Resets the canvas and adds writes the HTML header to it.
     */
    void beginWriting();

    /** Finishes writing to the canvas and completes the HTML (by adding closing tags) */
    void endWriting();

    void connectToNode(TreeNode *node);
    void disconnectFromNode(TreeNode *node);

    void setArticleActionsEnabled(bool enabled);

private:
    QString m_currentText;
    QPointer<TreeNode> m_node;
    QPointer<ArticleListJob> m_listJob;
    Article m_article;
    QVector<Article> m_articles;
    QUrl m_link;
    std::vector<QSharedPointer<const Filters::AbstractMatcher>> m_filters;
    enum ViewMode { NormalView, CombinedView, SummaryView };
    ViewMode m_viewMode = NormalView;
    Akregator::ArticleHtmlWebEngineWriter *m_articleHtmlWriter = nullptr;
    Akregator::ArticleViewerWebEngineWidgetNg *const m_articleViewerWidgetNg;
    QSharedPointer<ArticleFormatter> m_normalViewFormatter;
    QSharedPointer<ArticleFormatter> m_combinedViewFormatter;
    const QString m_grantleeDirectory;
};
} // namespace Akregator

