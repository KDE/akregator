/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#pragma once

#include <QObject>

#include "akregator_export.h"

#include <memory>

namespace Akregator
{
namespace Backend
{
class Storage;
class AKREGATOR_EXPORT FeedStorage : public QObject
{
public:
    FeedStorage(const QString &url, Storage *main);
    ~FeedStorage();

    Q_REQUIRED_RESULT int unread() const;
    void setUnread(int unread);
    Q_REQUIRED_RESULT int totalCount() const;
    Q_REQUIRED_RESULT QDateTime lastFetch() const;
    void setLastFetch(const QDateTime &lastFetch);

    Q_REQUIRED_RESULT QStringList articles() const;

    void article(const QString &guid, uint &hash, QString &title, int &status, QDateTime &pubDate) const;
    bool contains(const QString &guid) const;
    void addEntry(const QString &guid);
    void deleteArticle(const QString &guid);
    Q_REQUIRED_RESULT bool guidIsHash(const QString &guid) const;
    void setGuidIsHash(const QString &guid, bool isHash);
    bool guidIsPermaLink(const QString &guid) const;
    void setGuidIsPermaLink(const QString &guid, bool isPermaLink);
    Q_REQUIRED_RESULT uint hash(const QString &guid) const;
    void setHash(const QString &guid, uint hash);
    void setDeleted(const QString &guid);
    Q_REQUIRED_RESULT QString link(const QString &guid) const;
    void setLink(const QString &guid, const QString &link);
    Q_REQUIRED_RESULT QDateTime pubDate(const QString &guid) const;
    void setPubDate(const QString &guid, const QDateTime &pubdate);
    Q_REQUIRED_RESULT int status(const QString &guid) const;
    void setStatus(const QString &guid, int status);
    Q_REQUIRED_RESULT QString title(const QString &guid) const;
    void setTitle(const QString &guid, const QString &title);
    Q_REQUIRED_RESULT QString description(const QString &guid) const;
    void setDescription(const QString &guid, const QString &description);
    Q_REQUIRED_RESULT QString content(const QString &guid) const;
    void setContent(const QString &guid, const QString &content);

    void setEnclosure(const QString &guid, const QString &url, const QString &type, int length);
    void removeEnclosure(const QString &guid);
    void enclosure(const QString &guid, bool &hasEnclosure, QString &url, QString &type, int &length) const;

    void setAuthorName(const QString &guid, const QString &name);
    void setAuthorUri(const QString &guid, const QString &uri);
    void setAuthorEMail(const QString &guid, const QString &email);

    Q_REQUIRED_RESULT QString authorName(const QString &guid) const;
    Q_REQUIRED_RESULT QString authorUri(const QString &guid) const;
    Q_REQUIRED_RESULT QString authorEMail(const QString &guid) const;

    void setCategories(const QString &, const QStringList &categories);
    Q_REQUIRED_RESULT QStringList categories(const QString &guid) const;

    void close();
    void commit();
    void rollback();

private:
    void markDirty();
    /** finds article by guid, returns -1 if not in archive **/
    int findArticle(const QString &guid) const;
    void setTotalCount(int total);

private:
    class FeedStoragePrivate;
    std::unique_ptr<FeedStoragePrivate> const d;
};
} // namespace Backend
} // namespace Akregator
