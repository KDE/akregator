/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

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

#ifndef AKREGATORMYARTICLE_H
#define AKREGATORMYARTICLE_H

#include "librss/article.h" /* <rss/article.h> */

class KURLLabel;
class QDomDocument;
class QDomElement;
class QWidget;

using namespace RSS;

namespace Akregator
{
    class Feed;
    /** A proxy class for RSS::Article with some additional methods to assist sorting. */
    class MyArticle
    {
        public:
            enum { Unread=0, Read, New };
            typedef QValueList<MyArticle> List;

            MyArticle();
            MyArticle(Article article);
            MyArticle(const MyArticle &other);
            MyArticle &operator=(const MyArticle &other);
            bool operator==(const MyArticle &other) const;
            bool operator!=(const MyArticle &other) const { return !operator==(other); }
            virtual ~MyArticle();

            int status() const;
            void setStatus(int s);
            
            /**
             * @return true if this article is the same as other, based on guid or link.
             */
       //     bool isTheSameAs(const MyArticle &other);
            void offsetFetchTime(int secs);

            QString title() const;
            const KURL &link() const;
            QString description() const;
            QString guid() const;
            /** if true, the article should be kept even when expired **/
            bool keep() const;
            void setKeep(bool keep);
            void setFeed(Feed* feed);
            bool isDeleted() const;
            void setDeleted();
            

            Feed* feed() const;

            /** returns a hash value used to detect changes in articles with non-hash GUIDs. If the guid is a hash itself, it returns @c 0 */
            
            uint hash() const;

            /** returns if the guid is a hash or an ID taken from the source */
            
            bool guidIsHash() const;
            
            bool guidIsPermaLink() const;
            const QDateTime &pubDate() const;
            KURLLabel *widget(QWidget *parent = 0, const char *name = 0) const;
            const KURL &commentsLink() const;
            int comments() const;
            
            void dumpXmlData( QDomElement parent, QDomDocument doc ) const;
            
            bool operator<(const MyArticle &other) const;
            bool operator<=(const MyArticle &other) const;
            bool operator>(const MyArticle &other) const;
            bool operator>=(const MyArticle &other) const;
            
        private:
            static uint calcHash(const QString& str);
            struct Private;
            Private *d;
            QString buildTitle();
    };
}

#endif
