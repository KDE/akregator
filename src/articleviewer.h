/*
    This file is part of Akregator.

    Copyright (C) 2004 Sashmit Bhaduri <smt@vfemail.net>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef ARTICLEVIEWER_H
#define ARTICLEVIEWER_H

#include <klocale.h>

#include <qcolor.h>
#include <qfont.h>

#include "article.h"
#include "articlefilter.h"
#include "viewer.h"

class QKeyEvent;

namespace Akregator
{
    class Feed;
    class Folder;
    class TreeNode;
    
    /** This HTML viewer is used to display articles. 
    Use the high-level interface provided by the public slots whereever possible (and extend them when necessary instead of using low-level methods).*/
     class ArticleViewer : public Viewer
    {
        Q_OBJECT
        public:
            /** Constructor */
            ArticleViewer(QWidget* parent, const char* name);
            virtual ~ArticleViewer();
            
            virtual bool openURL(const KURL &url);

            /** Repaints the view. */
            void reload();

            void displayAboutPage();
            
            public slots:
	    
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
            
        protected slots:
                    
            void slotArticlesUpdated(TreeNode* node, const QValueList<Article>& list);
            void slotArticlesAdded(TreeNode* node, const QValueList<Article>& list);
            void slotArticlesRemoved(TreeNode* node, const QValueList<Article>& list);
            
        protected:
            
            virtual void keyPressEvent(QKeyEvent* e);
            virtual void urlSelected (const QString &url, int button, int state, const QString &_target, KParts::URLArgs args);

        private:

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
            KURL m_imageDir;
            TreeNode* m_node;
            Article m_article;
            KURL m_link;
            Akregator::Filters::ArticleMatcher m_textFilter; 
            Akregator::Filters::ArticleMatcher m_statusFilter;
            enum ViewMode { NormalView, CombinedView, SummaryView };
            ViewMode m_viewMode;
   };
}

#endif // ARTICLEVIEWER_H
