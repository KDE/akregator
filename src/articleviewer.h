/***************************************************************************
 *   Copyright (C) 2004 by Sashmit Bhaduri                                 *
 *   smt@vfemail.net                                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef ARTICLEVIEWER_H
#define ARTICLEVIEWER_H

#include <klocale.h>

#include <qcolor.h>
#include <qfont.h>
#include <qpaintdevicemetrics.h>

#include "articlefilter.h"
#include "viewer.h"


namespace Akregator
{
    class Feed;
    class FeedGroup;
    class MyArticle;
    class TreeNode;
    
    /** This HTML viewer is used to display articles. 
    Use the high-level interface provided by the public slots whereever possible (and extend them when necessary instead of using low-level methods).*/
    class ArticleViewer : public Viewer
    {
        Q_OBJECT
        public:
            /** Constructor */
            ArticleViewer(QWidget* parent, const char* name);

            /** Opens the welcome message. */
            void openDefault();
            
            /** Repaints the view. */
            void reload();
            
            public slots: 
	    
            // Commandment: We are your interfaces.
            // You shall not use strange interfaces before us.
	    
            /** Show single article (normal view) 
                @param article the article to render */
            void slotShowArticle(const MyArticle& article);
            
            /** Shows the articles of the tree node @c node (combined view). Changes in the node will update the view automatically. 
            @param node The node to observe */
            void slotShowNode(TreeNode* node);
            
            /** Set filters @c textFilter and @c statusFilter which will be used if the viewer is in combined view mode 
            @param textFilter text filter 
            @param statusFilter status filter */    
            void slotSetFilter(const ArticleFilter& textFilter, const ArticleFilter& statusFilter);
            
            /** Update view if combined view mode is set. Has to be called when the displayed node gets modified. */ 
            void slotUpdateCombinedView();
            
            /** Clears the canvas and disconnects from the currently observed node (if in combined view mode). */
            void slotClear();
	    
        protected:
            /** nobody uses this right now. Do we need it? -fo */
            virtual void openPage(const KURL&url, const KParts::URLArgs& args, const QString &mimetype);
            
        private:
            /** Takes an article and renders it as HTML            
            @param f article's feed (used for feed icon atm) -- article.feed() would do. better use a (No)Icon flag. -fo
            @param article The article to render
            @return the rendered article as HTML */
            QString formatArticle(Feed* feed, const MyArticle& article);
            
            /** Resets the canvas and adds writes the HTML header to it.
             */
            void beginWriting();
            
            /** Finishes writing to the canvas and completes the HTML (by adding closing tags) */
            void endWriting();
            
            /** generates the CSS settings used for rendering */
            void generateCSS();   
            
            
            void slotOpenLinkInternal();
                                  
            QString m_htmlHead;
            QPaintDeviceMetrics m_metrics;
            QString m_currentText;
            QString m_imageDir;
            TreeNode* m_node;
            ArticleFilter m_textFilter; 
            ArticleFilter m_statusFilter;
            enum ViewMode { normalView, combinedView };
            ViewMode m_viewMode;
        
        private slots:
            bool slotOpenURLRequest(const KURL& url, const KParts::URLArgs& args);
    };
}

#endif // ARTICLEVIEWER_H
