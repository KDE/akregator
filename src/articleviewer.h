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
    
    class ArticleViewer : public Viewer
    {
        Q_OBJECT
        public:
            ArticleViewer(QWidget* parent, const char* name);

            void openDefault();
            void reload();
            
            public slots: 
	    
            // Commandment: We are your interfaces.
            // You shall not use strange interfaces before us.
	    
            /** show single article (normal view) **/
            void slotShowArticle(const MyArticle& article);
            /** show all articles of node (combined view **/
            void slotShowNode(FeedGroup* node);
            /** set filters for combined view **/    
            void slotSetFilter(const ArticleFilter& textFilter, const ArticleFilter& statusFilter);
            /** update view if combined view set. For node and filter changes **/ 
            void slotUpdateCombinedView();
            /** clear and disconnect from observed node **/
            void slotClear();
	    
        protected:
            /** nobody uses this right now. Do we need it? -fo **/
            virtual void openPage(const KURL&url, const KParts::URLArgs& args, const QString &mimetype);
            
        private:
            QString formatArticle(Feed *f, MyArticle a);
            void beginWriting();
            void endWriting();
            void generateCSS();   
            void slotOpenLinkInternal();
                                  
            QString m_htmlHead;
            QPaintDeviceMetrics m_metrics;
            QString m_currentText;
            QString m_imageDir;
            FeedGroup* m_node;
            ArticleFilter m_textFilter; // these could be more general, so it could be used for search folders
            ArticleFilter m_statusFilter;
            enum ViewMode { normalView, combinedView };
            ViewMode m_viewMode;
        private slots:
            bool slotOpenURLRequest(const KURL& url, const KParts::URLArgs& args);
    };
}

#endif // ARTICLEVIEWER_H
