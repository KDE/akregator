/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>

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
#ifndef FEEDSTORAGE_H
#define FEEDSTORAGE_H

#include <qobject.h>

class QString;
class QStringList;


namespace Akregator {
namespace Backend {

class Storage;

class FeedStorage : public QObject
{
    public:
    
        virtual int unread() = 0;
        virtual void setUnread(int unread) = 0;
        virtual int totalCount() = 0;
        virtual int lastFetch() = 0;
        virtual void setLastFetch(int lastFetch) = 0;
        
        /** returns the guids of all articles in this storage */
        virtual QStringList articles() = 0;

        /** Appends all articles from another storage. If there is already an article in this feed with the same guid, it is replaced by the article from the source
        @param source the archive which articles should be appended
        */
        virtual void add(FeedStorage* source) = 0;

        /** reads an article from another storage and adds it to this storage */
        virtual void copyArticle(const QString& guid, FeedStorage* source) = 0;

        /** deletes all articles from the archive */
        virtual void clear() = 0;

        
        virtual bool contains(const QString& guid) = 0;
        virtual void addEntry(const QString& guid) = 0;
        virtual void deleteArticle(const QString& guid) = 0;
        virtual int comments(const QString& guid) = 0;
        virtual QString commentsLink(const QString& guid) = 0;
        virtual void setCommentsLink(const QString& guid, const QString& commentsLink) = 0;
        virtual void setComments(const QString& guid, int comments) = 0;
        virtual bool guidIsHash(const QString& guid) = 0;
        virtual void setGuidIsHash(const QString& guid, bool isHash) = 0;
        virtual bool guidIsPermaLink(const QString& guid) = 0;
        virtual void setGuidIsPermaLink(const QString& guid, bool isPermaLink) = 0;
        virtual uint hash(const QString& guid) = 0;
        virtual void setHash(const QString& guid, uint hash) = 0;
        virtual void setDeleted(const QString& guid) = 0;
        virtual QString link(const QString& guid) = 0;
        virtual void setLink(const QString& guid, const QString& link) = 0;
        virtual uint pubDate(const QString& guid) = 0;
        virtual void setPubDate(const QString& guid, uint pubdate) = 0;
        virtual int status(const QString& guid) = 0;
        virtual void setStatus(const QString& guid, int status) = 0;
        virtual QString title(const QString& guid) = 0;
        virtual void setTitle(const QString& guid, const QString& title) = 0;
        virtual QString description(const QString& guid) = 0;
        virtual void setDescription(const QString& guid, const QString& description) = 0;

        virtual void addTag(const QString& guid, const QString& tag) = 0;
        virtual void removeTag(const QString& guid, const QString& tag) = 0;
        virtual QStringList tags(const QString& guid) = 0;


        virtual void close() = 0;
        virtual void commit() = 0;
        virtual void rollback() = 0;
    
        virtual void convertOldArchive() = 0;
};

}
}

#endif
