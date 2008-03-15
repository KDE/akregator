/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <osterfeld@kde.org>

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
#ifndef AKREGATOR_BACKEND_FEEDSTORAGEMK4IMPL_H
#define AKREGATOR_BACKEND_FEEDSTORAGEMK4IMPL_H

#include "feedstorage.h"
namespace Akregator {
namespace Backend {

class StorageMK4Impl;
class FeedStorageMK4Impl : public FeedStorage
{
    public:
        FeedStorageMK4Impl(const QString& url, StorageMK4Impl* main);
        ~FeedStorageMK4Impl();


        void add(FeedStorage* source);
        void copyArticle(const QString& guid, FeedStorage* source);
        void clear();
        
        int unread() const;
        void setUnread(int unread);
        int totalCount() const;
        int lastFetch() const;
        void setLastFetch(int lastFetch);

        QStringList articles(const QString& tag=QString()) const;

        QStringList articles(const Category& cat) const;

        bool contains(const QString& guid) const;
        void addEntry(const QString& guid);
        void deleteArticle(const QString& guid);
        int comments(const QString& guid) const;
        QString commentsLink(const QString& guid) const;
        void setCommentsLink(const QString& guid, const QString& commentsLink);
        void setComments(const QString& guid, int comments);
        bool guidIsHash(const QString& guid) const;
        void setGuidIsHash(const QString& guid, bool isHash);
        bool guidIsPermaLink(const QString& guid) const;
        void setGuidIsPermaLink(const QString& guid, bool isPermaLink);
        uint hash(const QString& guid) const;
        void setHash(const QString& guid, uint hash);
        void setDeleted(const QString& guid);
        QString link(const QString& guid) const;
        void setLink(const QString& guid, const QString& link); 
        uint pubDate(const QString& guid) const;
        void setPubDate(const QString& guid, uint pubdate);
        int status(const QString& guid) const;
        void setStatus(const QString& guid, int status);
        QString title(const QString& guid) const;
        void setTitle(const QString& guid, const QString& title);
        QString description(const QString& guid) const;
        void setDescription(const QString& guid, const QString& description);
        QString content(const QString& guid) const;
        void setContent(const QString& guid, const QString& content);

        void setEnclosure(const QString& guid, const QString& url, const QString& type, int length);
        void removeEnclosure(const QString& guid);
        void enclosure(const QString& guid, bool& hasEnclosure, QString& url, QString& type, int& length) const;
        
        void addTag(const QString& guid, const QString& tag);
        void removeTag(const QString& guid, const QString& tag);
        QStringList tags(const QString& guid=QString()) const;

        void addCategory(const QString& guid, const Category& category);
        QList<Category> categories(const QString& guid=QString()) const;

        void setAuthor(const QString& guid, const QString& author);
        QString author(const QString& guid) const;
        
        void close();
        void commit();
        void rollback();
        
        void convertOldArchive();
   private:
        /** finds article by guid, returns -1 if not in archive **/
        int findArticle(const QString& guid) const;
        void setTotalCount(int total);
        class FeedStorageMK4ImplPrivate;
        FeedStorageMK4ImplPrivate* d;
};

} // namespace Backend
} // namespace Akregator

#endif // AKREGATOR_BACKEND_FEEDSTORAGEMK4IMPL_H
