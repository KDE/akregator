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
#ifndef FEEDSTORAGEMK4IMPL_H
#define FEEDSTORAGEMK4IMPL_H

#include "feedstorage.h"
namespace Akregator {
namespace Backend {

class StorageMK4Impl;
class FeedStorageMK4Impl : public FeedStorage
{
    public:
        FeedStorageMK4Impl(const QString& url, StorageMK4Impl* main);
        virtual ~FeedStorageMK4Impl();
        
        virtual int unread();
        virtual void setUnread(int unread);
        virtual int totalCount();
        virtual int lastFetch();
        virtual void setLastFetch(int lastFetch);
      
        virtual QStringList articles();

        virtual bool contains(const QString& guid);
        virtual void addEntry(const QString& guid);
        virtual void deleteArticle(const QString& guid);
        virtual int comments(const QString& guid);
        virtual QString commentsLink(const QString& guid);
        virtual void setCommentsLink(const QString& guid, const QString& commentsLink);
        virtual void setComments(const QString& guid, int comments);
        virtual bool guidIsHash(const QString& guid);
        virtual void setGuidIsHash(const QString& guid, bool isHash);
        virtual bool guidIsPermaLink(const QString& guid);
        virtual void setGuidIsPermaLink(const QString& guid, bool isPermaLink);
        virtual uint hash(const QString& guid);
        virtual void setHash(const QString& guid, uint hash);
        virtual void setDeleted(const QString& guid);
        virtual QString link(const QString& guid);
        virtual void setLink(const QString& guid, const QString& link); 
        virtual QDateTime pubDate(const QString& guid);
        virtual void setPubDate(const QString& guid, const QDateTime& pubdate);
        virtual int status(const QString& guid);
        virtual void setStatus(const QString& guid, int status);
        virtual QString title(const QString& guid);
        virtual void setTitle(const QString& guid, const QString& title);
        virtual QString description(const QString& guid);
        virtual void setDescription(const QString& guid, const QString& description);
        virtual void close();
        virtual void commit();
        virtual void rollback();
        
        virtual void convertOldArchive();
   private:
        /** finds article by guid, returns -1 if not in archive **/
        int findArticle(const QString& guid);
        void setTotalCount(int total);
        class FeedStorageMK4ImplPrivate;
        FeedStorageMK4ImplPrivate* d;
};

}
}
#endif // FEEDSTORAGEMK4IMPL_H
