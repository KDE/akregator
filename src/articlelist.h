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

namespace Akregator
{
    class Feed;
    class FeedGroup;
    class MyArticle;
    class TreeNode;
    
    class ArticleListItem : public KListViewItem
    {
        public:
            ArticleListItem( QListView *parent, QListViewItem *after, MyArticle a, Feed *parent );
            ~ArticleListItem();

            MyArticle article();
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
        public slots:
            void slotShowNode(TreeNode* node);
            void slotClear();
            void slotUpdate();
            void slotSetFilter(const ArticleFilter& textFilter, const ArticleFilter& statusFilter);
                                        
        private:
            TreeNode* m_node;
            ArticleFilter m_textFilter;
            ArticleFilter m_statusFilter;
            enum ColumnMode { groupMode, feedMode };
            ColumnMode m_columnMode;
    };
}

#endif
