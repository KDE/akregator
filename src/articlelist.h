/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
#ifndef AKREGATORARTICLELIST_H
#define AKREGATORARTICLELIST_H

#include "myarticle.h"
#include <klistview.h>

namespace Akregator
{
    class Feed;

    class ArticleListItem : public KListViewItem
    {
        public:
            ArticleListItem( QListView *parent, MyArticle a, Feed *parent );

            MyArticle article();
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
    };
}

#endif
