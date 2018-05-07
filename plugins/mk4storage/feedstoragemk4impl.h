/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
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
    FeedStorageMK4Impl(const QString &url, StorageMK4Impl *main);
    ~FeedStorageMK4Impl() override;

    Q_REQUIRED_RESULT int unread() const override;
    void setUnread(int unread) override;
    Q_REQUIRED_RESULT int totalCount() const override;
    Q_REQUIRED_RESULT QDateTime lastFetch() const override;
    void setLastFetch(const QDateTime &lastFetch) override;

    Q_REQUIRED_RESULT QStringList articles() const override;
    QVector<SmallArticle> articlesForCache() const override;

    void article(const QString &guid, uint &hash, QString &title, int &status, QDateTime &pubDate) const override;
    bool contains(const QString &guid) const override;
    void addEntry(const QString &guid) override;
    void deleteArticle(const QString &guid) override;
    Q_REQUIRED_RESULT bool guidIsHash(const QString &guid) const override;
    void setGuidIsHash(const QString &guid, bool isHash) override;
    bool guidIsPermaLink(const QString &guid) const override;
    void setGuidIsPermaLink(const QString &guid, bool isPermaLink) override;
    Q_REQUIRED_RESULT uint hash(const QString &guid) const override;
    void setHash(const QString &guid, uint hash) override;
    void setDeleted(const QString &guid) override;
    Q_REQUIRED_RESULT QString link(const QString &guid) const override;
    void setLink(const QString &guid, const QString &link) override;
    Q_REQUIRED_RESULT QDateTime pubDate(const QString &guid) const override;
    void setPubDate(const QString &guid, const QDateTime &pubdate) override;
    Q_REQUIRED_RESULT int status(const QString &guid) const override;
    void setStatus(const QString &guid, int status) override;
    Q_REQUIRED_RESULT QString title(const QString &guid) const override;
    void setTitle(const QString &guid, const QString &title) override;
    Q_REQUIRED_RESULT QString description(const QString &guid) const override;
    void setDescription(const QString &guid, const QString &description) override;
    Q_REQUIRED_RESULT QString content(const QString &guid) const override;
    void setContent(const QString &guid, const QString &content) override;

    void setEnclosure(const QString &guid, const QString &url, const QString &type, int length) override;
    void removeEnclosure(const QString &guid) override;
    void enclosure(const QString &guid, bool &hasEnclosure, QString &url, QString &type, int &length) const override;

    void setAuthorName(const QString &guid, const QString &name) override;
    void setAuthorUri(const QString &guid, const QString &uri) override;
    void setAuthorEMail(const QString &guid, const QString &email) override;

    Q_REQUIRED_RESULT QString authorName(const QString &guid) const override;
    Q_REQUIRED_RESULT QString authorUri(const QString &guid) const override;
    Q_REQUIRED_RESULT QString authorEMail(const QString &guid) const override;

    void setCategories(const QString &, const QStringList &categories) override;
    Q_REQUIRED_RESULT QStringList categories(const QString &guid) const override;

    void close();
    void commit();
    void rollback();
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
