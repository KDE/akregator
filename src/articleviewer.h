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
#include "articlefilter.h"

#include <khtml_part.h>

class KUrl;

namespace KIO
{
    class MetaData;
}

namespace Akregator
{

    class Feed;
    class Folder;
    class OpenURLRequest;
    class TreeNode;
    
    class ArticleViewer : public KHTMLPart
    {
        Q_OBJECT
        public:
            ArticleViewer(QWidget* parent, const char* name=0L);
            virtual ~ArticleViewer();

            bool closeURL();
        
            /** Repaints the view. */
            void reload();
        
            void displayAboutPage();

            bool openURL(const KUrl &url);
        
        public slots:
        
            void slotScrollUp(); 
            void slotScrollDown();
            void slotZoomIn();
            void slotZoomOut();
            void slotSetZoomFactor(int percent);
            void slotPrint();
            void setSafeMode();
   
        // Commandment: We are your interfaces.
        // You shall not use strange interfaces before us.
    
        /** Show single article (normal view) 
            @param article the article to render */
            void slotShowArticle(const Article& article);
        
        /** Shows the articles of the tree node @c node (combined view). Changes in the node will update the view automatically. 
            @param node The node to observe */
            void slotShowNode(TreeNode* node);
        
        /** Set filters @c textFilter and @c statusFilter which will be used if the viewer is in combined view mode 
            @param textFilter text filter 
            @param statusFilter status filter */    
            void slotSetFilter(const Akregator::Filters::ArticleMatcher& textFilter, const Akregator::Filters::ArticleMatcher& statusFilter);
        
            /** Update view if combined view mode is set. Has to be called when the displayed node gets modified. */ 
            void slotUpdateCombinedView();
        
            /** Clears the canvas and disconnects from the currently observed node (if in combined view mode). */
            void slotClear();
    
            void slotShowSummary(TreeNode *node);

            virtual void slotPaletteOrFontChanged();
        
        signals:
            // This gets emitted when url gets clicked */
        //void urlClicked(const KUrl& url, bool background=false);
        
            /** This gets emitted when url gets clicked */
            void signalOpenURLRequest(OpenURLRequest&); 

            protected: // methods
                int pointsToPixel(int points) const;

        protected slots:
    
        // FIXME: Sort out how things are supposed to work and clean up the following slots

            /** reimplemented to handle url selection according to the settings for LMB and MMB */
            virtual void urlSelected(const QString &url, int button, int state, const QString &_target, KParts::URLArgs args);

            /** slot for handling openURLRequestDelayed() signal from the browserextension. Handles POST requests (for forms) only, other link handling goes to urlSelected(). Does nothing in Viewer, reimplemented in PageViewer to make forms working */
            virtual void slotOpenURLRequest(const KUrl& url, const KParts::URLArgs& args);

            virtual void slotPopupMenu(KXMLGUIClient*, const QPoint&, const KUrl&, const KParts::URLArgs&, KParts::BrowserExtension::PopupFlags, mode_t);

            /** Copies current link to clipboard. */
            void slotCopyLinkAddress();

            /** Copies currently selected text to clipboard */
            virtual void slotCopy();

            /** Opens @c m_url inside this viewer */
            virtual void slotOpenLinkInternal();

            /** Opens @c m_url in external viewer, eg. Konqueror */
            virtual void slotOpenLinkInBrowser();

            /** Opens @c m_url in foreground tab */
            virtual void slotOpenLinkInForegroundTab();

            /** Opens @c m_url in background tab */
            virtual void slotOpenLinkInBackgroundTab();

            virtual void slotSaveLinkAs();

            /** This changes cursor to wait cursor */
            void slotStarted(KIO::Job *);

            /** This reverts cursor back to normal one */
            void slotCompleted();

            virtual void slotSelectionChanged();

            void slotArticlesUpdated(TreeNode* node, const QList<Article>& list);
            void slotArticlesAdded(TreeNode* node, const QList<Article>& list);
            void slotArticlesRemoved(TreeNode* node, const QList<Article>& list);
        
            protected: // attributes
                KUrl m_url;
        
    // from ArticleViewer  
        protected:
        
            virtual void keyPressEvent(QKeyEvent* e);

            friend class ShowNodeSummaryVisitor;
            class ShowSummaryVisitor;
            ShowSummaryVisitor* m_showSummaryVisitor;
        
        /** renders @c body. Use this method whereever possible.
            @param body html to render, without header and footer */
            void renderContent(const QString& body);
        
        /** Takes an article and renders it as HTML with settings for normal view and widescreen view
            @param f article's feed (used for feed icon atm) -- article.feed() would do. better use a (No)Icon flag. -fo
            @param article The article to render
            @return the rendered article as HTML */
            QString formatArticleNormalMode(Feed* feed, const Article& article);
        
        /** Takes an article and renders it as HTML with settings for combined view
            @param f article's feed (used for feed icon atm) -- article.feed() would do. better use a (No)Icon flag. -fo
            @param article The article to render
            @return the rendered article as HTML */
            QString formatArticleCombinedMode(Feed* feed, const Article& article);
        
        /** Resets the canvas and adds writes the HTML header to it.
         */
            void beginWriting();

            /** Finishes writing to the canvas and completes the HTML (by adding closing tags) */
            void endWriting();
        
            /** generates the CSS used for rendering in single article mode (normal and wide screen view) */
            void generateNormalModeCSS();
            /** generates the CSS for combined view mode */
            void generateCombinedModeCSS();
            void connectToNode(TreeNode* node);
            void disconnectFromNode(TreeNode* node);
                                
            QString m_normalModeCSS;
            QString m_combinedModeCSS;
            QString m_htmlFooter;
            QString m_currentText;
            KUrl m_imageDir;
            TreeNode* m_node;
            Article m_article;
            KUrl m_link;
            Akregator::Filters::ArticleMatcher m_textFilter; 
            Akregator::Filters::ArticleMatcher m_statusFilter;
            enum ViewMode { NormalView, CombinedView, SummaryView };
            ViewMode m_viewMode;
    };

} // namespace Akregator

#endif // AKREGATOR_ARTICLEVIEWER_H

