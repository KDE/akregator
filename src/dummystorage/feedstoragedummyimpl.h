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
#ifndef AKREGATOR_BACKEND_FEEDSTORAGEDUMMYIMPL_H
#define AKREGATOR_BACKEND_FEEDSTORAGEDUMMYIMPL_H

#include "feedstorage.h"
#include <QList>
namespace Akregator {
namespace Backend {
class StorageDummyImpl;
class FeedStorageDummyImpl : public FeedStorage
{
public:
    FeedStorageDummyImpl(const QString &url, StorageDummyImpl *main);
    ~FeedStorageDummyImpl() override;

    int unread() const override;
    void setUnread(int unread) override;
    int totalCount() const override;
    QDateTime lastFetch() const override;
    void setLastFetch(const QDateTime &lastFetch) override;

    QStringList articles() const override;

    void article(const QString &guid, uint &hash, QString &title, int &status, QDateTime &pubDate) const override;
    bool contains(const QString &guid) const override;
    void addEntry(const QString &guid) override;
    void deleteArticle(const QString &guid) override;
    bool guidIsHash(const QString &guid) const override;
    void setGuidIsHash(const QString &guid, bool isHash) override;
    bool guidIsPermaLink(const QString &guid) const override;
    void setGuidIsPermaLink(const QString &guid, bool isPermaLink) override;
    uint hash(const QString &guid) const override;
    void setHash(const QString &guid, uint hash) override;
    void setDeleted(const QString &guid) override;
    QString link(const QString &guid) const override;
    void setLink(const QString &guid, const QString &link) override;
    QDateTime pubDate(const QString &guid) const override;
    void setPubDate(const QString &guid, const QDateTime &pubdate) override;
    int status(const QString &guid) const override;
    void setStatus(const QString &guid, int status) override;
    QString title(const QString &guid) const override;
    void setTitle(const QString &guid, const QString &title) override;
    QString description(const QString &guid) const override;
    void setDescription(const QString &guid, const QString &description) override;
    QString content(const QString &guid) const override;
    void setContent(const QString &guid, const QString &content) override;

    void setEnclosure(const QString &guid, const QString &url, const QString &type, int length) override;
    void removeEnclosure(const QString &guid) override;
    void enclosure(const QString &guid, bool &hasEnclosure, QString &url, QString &type, int &length) const override;

    void setAuthorName(const QString &guid, const QString &authorName) override;
    void setAuthorUri(const QString &guid, const QString &authorUri) override;
    void setAuthorEMail(const QString &guid, const QString &authorEMail) override;

    QString authorName(const QString &guid) const override;
    QString authorUri(const QString &guid) const override;
    QString authorEMail(const QString &guid) const override;

private:
    /** finds article by guid, returns -1 if not in archive **/
    int findArticle(const QString &guid) const;
    void setTotalCount(int total);
    class FeedStorageDummyImplPrivate;
    FeedStorageDummyImplPrivate *d;
};
} // namespace Backend
} // namespace Akregator

#endif // AKREGATOR_FEEDSTORAGEDUMMYIMPL_H
