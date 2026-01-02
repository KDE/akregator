/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
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

    [[nodiscard]] int unread() const;
    void setUnread(int unread);
    [[nodiscard]] int totalCount() const;
    [[nodiscard]] QDateTime lastFetch() const;
    void setLastFetch(const QDateTime &lastFetch);

    [[nodiscard]] QStringList articles() const;

    void article(const QString &guid, uint &hash, QString &title, int &status, QDateTime &pubDate) const;
    bool contains(const QString &guid) const;
    void addEntry(const QString &guid);
    void deleteArticle(const QString &guid);
    [[nodiscard]] bool guidIsHash(const QString &guid) const;
    void setGuidIsHash(const QString &guid, bool isHash);
    bool guidIsPermaLink(const QString &guid) const;
    void setGuidIsPermaLink(const QString &guid, bool isPermaLink);
    [[nodiscard]] uint hash(const QString &guid) const;
    void setHash(const QString &guid, uint hash);
    void setDeleted(const QString &guid);
    [[nodiscard]] QString link(const QString &guid) const;
    void setLink(const QString &guid, const QString &link);
    [[nodiscard]] QDateTime pubDate(const QString &guid) const;
    void setPubDate(const QString &guid, const QDateTime &pubdate);
    [[nodiscard]] int status(const QString &guid) const;
    void setStatus(const QString &guid, int status);
    [[nodiscard]] QString title(const QString &guid) const;
    void setTitle(const QString &guid, const QString &title);
    [[nodiscard]] QString description(const QString &guid) const;
    void setDescription(const QString &guid, const QString &description);
    [[nodiscard]] QString content(const QString &guid) const;
    void setContent(const QString &guid, const QString &content);

    void setEnclosure(const QString &guid, const QString &url, const QString &type, int length);
    void removeEnclosure(const QString &guid);
    void enclosure(const QString &guid, bool &hasEnclosure, QString &url, QString &type, int &length) const;

    void setAuthorName(const QString &guid, const QString &name);
    void setAuthorUri(const QString &guid, const QString &uri);
    void setAuthorEMail(const QString &guid, const QString &email);

    [[nodiscard]] QString authorName(const QString &guid) const;
    [[nodiscard]] QString authorUri(const QString &guid) const;
    [[nodiscard]] QString authorEMail(const QString &guid) const;

    void setCategories(const QString &, const QStringList &categories);
    [[nodiscard]] QStringList categories(const QString &guid) const;

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
