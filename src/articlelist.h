/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
#ifndef AKREGATORARTICLELIST_H
#define AKREGATORARTICLELIST_H

#include "articlefilter.h"

#include <klistview.h>

class QKeyEvent;

namespace Akregator
{
    class Feed;
    class FeedGroup;
    class MyArticle;
    class TreeNode;
    
    class ArticleListItem : public KListViewItem
    {
        public:
            ArticleListItem( QListView *parent, QListViewItem *after, const MyArticle& a, Feed *parent );
            ~ArticleListItem();

            const MyArticle& article();
            Feed *feed();
            void paintCell ( QPainter * p, const QColorGroup & cg, int column, int width, int align );
            
        private:
            struct Private;
            Private *d;
    };
    
    
    class ArticleList : public KListView
    {
        Q_OBJECT
        public:
            ArticleList(QWidget *parent = 0, const char *name = 0);
            ~ArticleList();
           
            void setReceiveUpdates(bool doReceive, bool remember=true);
            
        public slots:
            void slotShowNode(TreeNode* node);
            void slotClear();
            void slotUpdate();
            void slotSetFilter(const ArticleFilter& textFilter, const ArticleFilter& statusFilter);
            void slotPreviousArticle();
            void slotNextArticle();
            void slotPreviousUnreadArticle();
            void slotNextUnreadArticle();

        signals:
             void signalArticleSelected(MyArticle article);
             
        protected:
            virtual void keyPressEvent(QKeyEvent* e);

        protected slots:
            virtual void slotSelectionChanged(QListViewItem* item);    
            
        private:
            bool m_updated;
            bool m_doReceive;
            TreeNode* m_node;
            ArticleFilter m_textFilter;
            ArticleFilter m_statusFilter;
            enum ColumnMode { groupMode, feedMode };
            ColumnMode m_columnMode;
    };
}

#endif
