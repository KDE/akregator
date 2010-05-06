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

#ifndef AKREGATOR_ARTICLEVIEWER_H
#define AKREGATOR_ARTICLEVIEWER_H

#include "article.h"

#include <khtml_part.h>

#include <QFrame>
#include <QPointer>

#include <boost/shared_ptr.hpp>
#include <vector>

class KJob;
class KUrl;

namespace Akregator {

namespace Filters {
    class AbstractMatcher;
}

class ArticleFormatter;
class ArticleListJob;
class OpenUrlRequest;
class TreeNode;

class ArticleViewerPart;

class ArticleViewer : public QWidget
{
    Q_OBJECT
    public:
        explicit ArticleViewer(QWidget* parent);
        ~ArticleViewer();


        /** Repaints the view. */
        void reload();

        void displayAboutPage();

        KParts::ReadOnlyPart* part() const;

        void setNormalViewFormatter(const boost::shared_ptr<ArticleFormatter>& formatter);

        void setCombinedViewFormatter(const boost::shared_ptr<ArticleFormatter>& formatter);

        void showArticle( const Article& article );

        /** Shows the articles of the tree node @c node (combined view).
         * Changes in the node will update the view automatically.
         *
         *  @param node The node to observe */
        void showNode(Akregator::TreeNode* node);

    public slots:

        void slotZoomIn(int);
        void slotZoomOut(int);
        void slotSetZoomFactor(int percent);
        void slotPrint();

        /** Set filters which will be used if the viewer is in combined view mode
         */
        void setFilters( const std::vector< boost::shared_ptr<const Akregator::Filters::AbstractMatcher> >& filters );

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

    signals:

        /** This gets emitted when url gets clicked */
        void signalOpenUrlRequest(Akregator::OpenUrlRequest&);

        void started(KIO::Job*);
        void selectionChanged();
        void completed();

    protected: // methods
        int pointsToPixel(int points) const;

        bool openUrl(const KUrl &url);

    protected slots:

        void slotOpenUrlRequestDelayed(const KUrl&, const KParts::OpenUrlArguments&, const KParts::BrowserArguments&);

        void slotCreateNewWindow(const KUrl& url,
                                    const KParts::OpenUrlArguments& args,
                                    const KParts::BrowserArguments& browserArgs,
                                    const KParts::WindowArgs& windowArgs,
                                    KParts::ReadOnlyPart** part);

        void slotPopupMenu(const QPoint&, const KUrl&, mode_t, const KParts::OpenUrlArguments&, const KParts::BrowserArguments&, KParts::BrowserExtension::PopupFlags);

        /** Copies current link to clipboard. */
        void slotCopyLinkAddress();

        /** Copies currently selected text to clipboard */
        void slotCopy();

        /** Opens @c m_url inside this viewer */
        void slotOpenLinkInternal();

        /** Opens @c m_url in external viewer, eg. Konqueror */
        void slotOpenLinkInBrowser();

        /** Opens @c m_url in foreground tab */
        void slotOpenLinkInForegroundTab();

        /** Opens @c m_url in background tab */
        void slotOpenLinkInBackgroundTab();

        void slotSaveLinkAs();

        /** This changes cursor to wait cursor */
        void slotStarted(KIO::Job *);

        /** This reverts cursor back to normal one */
        void slotCompleted();

        void slotSelectionChanged();

        void slotArticlesListed(KJob* job);

        void slotArticlesUpdated(Akregator::TreeNode* node, const QList<Akregator::Article>& list);
        void slotArticlesAdded(Akregator::TreeNode* node, const QList<Akregator::Article>& list);
        void slotArticlesRemoved(Akregator::TreeNode* node, const QList<Akregator::Article>& list);


    // from ArticleViewer
    private:

        virtual void keyPressEvent(QKeyEvent* e);

        /** renders @c body. Use this method whereever possible.
         *  @param body html to render, without header and footer */
        void renderContent(const QString& body);

        /** Resets the canvas and adds writes the HTML header to it.
            */
        void beginWriting();

        /** Finishes writing to the canvas and completes the HTML (by adding closing tags) */
        void endWriting();

        void updateCss();

        void connectToNode(TreeNode* node);
        void disconnectFromNode(TreeNode* node);

        void setArticleActionsEnabled(bool enabled);

    private:
        KUrl m_url;
        QString m_normalModeCSS;
        QString m_combinedModeCSS;
        QString m_htmlFooter;
        QString m_currentText;
        KUrl m_imageDir;
        QPointer<TreeNode> m_node;
        QPointer<ArticleListJob> m_listJob;
        Article m_article;
        QList<Article> m_articles;
        KUrl m_link;
        std::vector<boost::shared_ptr<const Filters::AbstractMatcher> > m_filters;
        enum ViewMode { NormalView, CombinedView, SummaryView };
        ViewMode m_viewMode;
        ArticleViewerPart* m_part;
        boost::shared_ptr<ArticleFormatter> m_normalViewFormatter;
        boost::shared_ptr<ArticleFormatter> m_combinedViewFormatter;
};

class ArticleViewerPart : public KHTMLPart
{
    Q_OBJECT

    public:
        explicit ArticleViewerPart(QWidget* parent);

        bool closeUrl();

        int button() const;

    protected:

        /** reimplemented to get the mouse button */
        bool urlSelected(const QString &url, int button, int state, const QString &_target,
                         const KParts::OpenUrlArguments& args = KParts::OpenUrlArguments(),
                         const KParts::BrowserArguments& browserArgs = KParts::BrowserArguments());

    private:

        int m_button;
};

} // namespace Akregator

#endif // AKREGATOR_ARTICLEVIEWER_H

