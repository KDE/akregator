/*
    This file is part of Akregator.

    Copyright (C) 2004 Teemu Rytilahti <tpr@d5k.net>

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

#ifndef AKREGATOR_ARTICLEVIEWERWIDGET_H
#define AKREGATOR_ARTICLEVIEWERWIDGET_H

#include "article.h"
#include "akregator_export.h"
#include "config-akregator.h"
#include <QWidget>

#include <QPointer>

#include <QSharedPointer>
#include <vector>
#include <QUrl>

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
    ~ArticleViewerWidget();

    /** Repaints the view. */
    void reload();

    void displayAboutPage();

    void showArticle(const Article &article);

    /** Shows the articles of the tree node @c node (combined view).
     * Changes in the node will update the view automatically.
     *
     *  @param node The node to observe */
    void showNode(Akregator::TreeNode *node);

    QSize sizeHint() const Q_DECL_OVERRIDE;

    qreal zoomFactor() const;
    bool zoomTextOnlyInFrame() const;

    Akregator::ArticleViewerWebEngineWidgetNg *articleViewerWidgetNg() const;

public Q_SLOTS:
    void slotZoomTextOnlyInFrame(bool textOnlyInFrame);
    void slotPrint();

    /** Set filters which will be used if the viewer is in combined view mode
     */
    void setFilters(const std::vector< QSharedPointer<const Akregator::Filters::AbstractMatcher> > &filters);

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

    void slotPaletteOrFontChanged();

    void slotPrintPreview();
    void slotCopy();

    void slotZoomChangeInFrame(qreal value);
Q_SIGNALS:

    /** This gets emitted when url gets clicked */
    void signalOpenUrlRequest(Akregator::OpenUrlRequest &);
    void showStatusBarMessage(const QString &msg);

    void selectionChanged();

protected: // methods
    int pointsToPixel(int points) const;

    bool openUrl(const QUrl &url);

protected Q_SLOTS:
    void slotSelectionChanged();

    void slotArticlesListed(KJob *job);

    void slotArticlesUpdated(Akregator::TreeNode *node, const QVector<Akregator::Article> &list);
    void slotArticlesAdded(Akregator::TreeNode *node, const QVector<Akregator::Article> &list);
    void slotArticlesRemoved(Akregator::TreeNode *node, const QVector<Akregator::Article> &list);

    // from ArticleViewer
private:

    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;

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
    QUrl m_imageDir;
    QPointer<TreeNode> m_node;
    QPointer<ArticleListJob> m_listJob;
    Article m_article;
    QVector<Article> m_articles;
    QUrl m_link;
    std::vector<QSharedPointer<const Filters::AbstractMatcher> > m_filters;
    enum ViewMode { NormalView, CombinedView, SummaryView };
    ViewMode m_viewMode;
    Akregator::ArticleHtmlWebEngineWriter *m_articleHtmlWriter;
    Akregator::ArticleViewerWebEngineWidgetNg *m_articleViewerWidgetNg;
    QSharedPointer<ArticleFormatter> m_normalViewFormatter;
    QSharedPointer<ArticleFormatter> m_combinedViewFormatter;
};

} // namespace Akregator

#endif // AKREGATOR_ARTICLEVIEWER_H

