/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef AKREGATORMYARTICLE_H
#define AKREGATORMYARTICLE_H

#include "librss/article.h" /* <rss/article.h> */

class KURLLabel;
class QWidget;

using namespace RSS;

namespace Akregator
{
    /** A proxy class for RSS::Article with some additional methods to assist sorting. */
    class MyArticle
    {
        public:
            typedef QValueList<MyArticle> List;

            MyArticle();
            MyArticle(Article article);
            MyArticle(const MyArticle &other);
            MyArticle &operator=(const MyArticle &other);
            bool operator==(const MyArticle &other) const;
            bool operator!=(const MyArticle &other) const { return !operator==(other); }
            virtual ~MyArticle();

            QString title() const;
            const KURL &link() const;
            QString description() const;
            QString guid() const;
            bool guidIsPermaLink() const;
            const QDateTime &pubDate() const;
            KURLLabel *widget(QWidget *parent = 0, const char *name = 0) const;

        private:
            struct Private;
            Private *d;
    };
};

#endif
