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
namespace Akregator
{
namespace Backend
{

class StorageMK4Impl;
class FeedStorageMK4Impl : public FeedStorage
{
public:
    FeedStorageMK4Impl(const QString &url, StorageMK4Impl *main);
    ~FeedStorageMK4Impl();

    void add(FeedStorage *source) Q_DECL_OVERRIDE;
    void copyArticle(const QString &guid, FeedStorage *source) Q_DECL_OVERRIDE;
    void clear() Q_DECL_OVERRIDE;

    int unread() const Q_DECL_OVERRIDE;
    void setUnread(int unread) Q_DECL_OVERRIDE;
    int totalCount() const Q_DECL_OVERRIDE;
    int lastFetch() const Q_DECL_OVERRIDE;
    void setLastFetch(int lastFetch);

    QStringList articles(const QString &tag = QString()) const Q_DECL_OVERRIDE;

    QStringList articles(const Category &cat) const Q_DECL_OVERRIDE;

    bool contains(const QString &guid) const Q_DECL_OVERRIDE;
    void addEntry(const QString &guid) Q_DECL_OVERRIDE;
    void deleteArticle(const QString &guid) Q_DECL_OVERRIDE;
    int comments(const QString &guid) const Q_DECL_OVERRIDE;
    QString commentsLink(const QString &guid) const Q_DECL_OVERRIDE;
    void setCommentsLink(const QString &guid, const QString &commentsLink) Q_DECL_OVERRIDE;
    void setComments(const QString &guid, int comments) Q_DECL_OVERRIDE;
    bool guidIsHash(const QString &guid) const Q_DECL_OVERRIDE;
    void setGuidIsHash(const QString &guid, bool isHash) Q_DECL_OVERRIDE;
    bool guidIsPermaLink(const QString &guid) const Q_DECL_OVERRIDE;
    void setGuidIsPermaLink(const QString &guid, bool isPermaLink) Q_DECL_OVERRIDE;
    uint hash(const QString &guid) const Q_DECL_OVERRIDE;
    void setHash(const QString &guid, uint hash) Q_DECL_OVERRIDE;
    void setDeleted(const QString &guid) Q_DECL_OVERRIDE;
    QString link(const QString &guid) const Q_DECL_OVERRIDE;
    void setLink(const QString &guid, const QString &link) Q_DECL_OVERRIDE;
    uint pubDate(const QString &guid) const Q_DECL_OVERRIDE;
    void setPubDate(const QString &guid, uint pubdate) Q_DECL_OVERRIDE;
    int status(const QString &guid) const Q_DECL_OVERRIDE;
    void setStatus(const QString &guid, int status) Q_DECL_OVERRIDE;
    QString title(const QString &guid) const Q_DECL_OVERRIDE;
    void setTitle(const QString &guid, const QString &title) Q_DECL_OVERRIDE;
    QString description(const QString &guid) const Q_DECL_OVERRIDE;
    void setDescription(const QString &guid, const QString &description) Q_DECL_OVERRIDE;
    QString content(const QString &guid) const Q_DECL_OVERRIDE;
    void setContent(const QString &guid, const QString &content) Q_DECL_OVERRIDE;

    void setEnclosure(const QString &guid, const QString &url, const QString &type, int length) Q_DECL_OVERRIDE;
    void removeEnclosure(const QString &guid) Q_DECL_OVERRIDE;
    void enclosure(const QString &guid, bool &hasEnclosure, QString &url, QString &type, int &length) const;

    void addTag(const QString &guid, const QString &tag) Q_DECL_OVERRIDE;
    void removeTag(const QString &guid, const QString &tag) Q_DECL_OVERRIDE;
    QStringList tags(const QString &guid = QString()) const;

    void addCategory(const QString &guid, const Category &category) Q_DECL_OVERRIDE;
    QList<Category> categories(const QString &guid = QString()) const;

    void setAuthorName(const QString &guid, const QString &name) Q_DECL_OVERRIDE;
    void setAuthorUri(const QString &guid, const QString &uri) Q_DECL_OVERRIDE;
    void setAuthorEMail(const QString &guid, const QString &email) Q_DECL_OVERRIDE;

    QString authorName(const QString &guid) const;
    QString authorUri(const QString &guid) const;
    QString authorEMail(const QString &guid) const;

    void close() Q_DECL_OVERRIDE;
    void commit() Q_DECL_OVERRIDE;
    void rollback() Q_DECL_OVERRIDE;

    void convertOldArchive() Q_DECL_OVERRIDE;
private:
    void markDirty();
    /** finds article by guid, returns -1 if not in archive **/
    int findArticle(const QString &guid) const;
    void setTotalCount(int total);
    class FeedStorageMK4ImplPrivate;
    FeedStorageMK4ImplPrivate *d;
};

} // namespace Backend
} // namespace Akregator

#endif // AKREGATOR_BACKEND_FEEDSTORAGEMK4IMPL_H
