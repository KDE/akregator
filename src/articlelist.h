/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
#ifndef AKREGATORARTICLELIST_H
#define AKREGATORARTICLELIST_H

#include <article.h>   /* rss/article.h ! */
#include <klistview.h>

namespace Akregator
{
    class Feed;

    class ArticleListItem : public KListViewItem
    {
        public:
            ArticleListItem( QListView *parent, RSS::Article a, Feed *parent );

            /** Override compare() to compare by pubDate */
            int compare( QListViewItem *i, int col, bool ascending ) const;

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
