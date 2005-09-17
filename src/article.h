#include <QList>
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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef AKREGATOR_ARTICLE_H
#define AKREGATOR_ARTICLE_H

class QDateTime;
class QDomDocument;
class QDomElement;
class QString;
class QStringList;
class QWidget;

template <class T> class QList;

typedef unsigned int uint;

class KURL;
class KURLLabel;

namespace RSS
{
    class Article;
    class Enclosure;
}

namespace Akregator
{
    namespace Backend
    {
        class FeedStorage;
    }
    class Feed;
    /** A proxy class for RSS::Article with some additional methods to assist sorting. */
    class Article
    {
        public:
            enum Status { Unread=0, Read, New };
            typedef QList<Article> List;

            Article();
            /** creates am article object for an existing article.
             The constructor accesses the archive to load it's data
             */
            Article(const QString& guid, Feed* feed);
            /** creates an article object from a parsed librss Article
               the article is added to the archive if not yet stored, or updated if stored but modified
            */
            Article(RSS::Article article, Feed* feed);
            
            Article(RSS::Article article, Backend::FeedStorage* archive);
            Article(const Article &other);
            Article &operator=(const Article &other);
            bool operator==(const Article &other) const;
            bool operator!=(const Article &other) const { return !operator==(other); }
            virtual ~Article();

            bool isNull() const;
            
            int status() const;
            void setStatus(int s);

            void offsetPubDate(int secs);

            QString title() const;
            KURL link() const;
            QString description() const;
            QString guid() const;
            /** if true, the article should be kept even when expired **/
            bool keep() const;
            void setKeep(bool keep);
            bool isDeleted() const;
            
            RSS::Enclosure enclosure() const;
            
            void setDeleted();
            

            Feed* feed() const;

            /** returns a hash value used to detect changes in articles with non-hash GUIDs. If the guid is a hash itself, it returns @c 0 */
            
            uint hash() const;

            /** returns if the guid is a hash or an ID taken from the source */
            
            bool guidIsHash() const;
            
            bool guidIsPermaLink() const;
            
            const QDateTime& pubDate() const;
            
            KURL commentsLink() const;
            
            int comments() const;
            
            void addTag(const QString& tag);
            void removeTag(const QString& tag);
            bool hasTag(const QString& tag) const;
            QStringList tags() const;
            
            bool operator<(const Article &other) const;
            bool operator<=(const Article &other) const;
            bool operator>(const Article &other) const;
            bool operator>=(const Article &other) const;


        private:
            void initialize(RSS::Article article, Backend::FeedStorage* archive);
            static uint calcHash(const QString& str);
            static QString buildTitle(const QString& description);
            
            struct Private;
            Private *d;
    };
}

#endif
