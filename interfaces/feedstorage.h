/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#pragma once

#include <QObject>
class QString;
#include <QStringList>
class QDateTime;

namespace Akregator
{
namespace Backend
{
class Storage;

class FeedStorage : public QObject // krazy:exclude=qobject
{
public:
    virtual int unread() const = 0;
    virtual void setUnread(int unread) = 0;
    virtual int totalCount() const = 0;
    virtual QDateTime lastFetch() const = 0;
    virtual void setLastFetch(const QDateTime &lastFetch) = 0;

    /** returns the guids of all articles in this storage. */
    virtual QStringList articles() const = 0;

    virtual void article(const QString &guid, uint &hash, QString &title, int &status, QDateTime &pubDate) const = 0;
    virtual bool contains(const QString &guid) const = 0;
    virtual void addEntry(const QString &guid) = 0;
    virtual void deleteArticle(const QString &guid) = 0;
    virtual bool guidIsHash(const QString &guid) const = 0;
    virtual void setGuidIsHash(const QString &guid, bool isHash) = 0;
    virtual bool guidIsPermaLink(const QString &guid) const = 0;
    virtual void setGuidIsPermaLink(const QString &guid, bool isPermaLink) = 0;
    virtual uint hash(const QString &guid) const = 0;
    virtual void setHash(const QString &guid, uint hash) = 0;
    virtual void setDeleted(const QString &guid) = 0;
    virtual QString link(const QString &guid) const = 0;
    virtual void setLink(const QString &guid, const QString &link) = 0;
    virtual QDateTime pubDate(const QString &guid) const = 0;
    virtual void setPubDate(const QString &guid, const QDateTime &pubdate) = 0;
    virtual int status(const QString &guid) const = 0;
    virtual void setStatus(const QString &guid, int status) = 0;
    virtual QString title(const QString &guid) const = 0;
    virtual void setTitle(const QString &guid, const QString &title) = 0;
    virtual QString description(const QString &guid) const = 0;
    virtual void setDescription(const QString &guid, const QString &description) = 0;
    virtual QString content(const QString &guid) const = 0;
    virtual void setContent(const QString &guid, const QString &content) = 0;

    virtual void setEnclosure(const QString &guid, const QString &url, const QString &type, int length) = 0;
    virtual void removeEnclosure(const QString &guid) = 0;

    virtual void setAuthorName(const QString & /*guid*/, const QString &name) = 0;
    virtual void setAuthorUri(const QString & /*guid*/, const QString &uri) = 0;
    virtual void setAuthorEMail(const QString & /*guid*/, const QString &email) = 0;

    virtual QString authorName(const QString &guid) const = 0;
    virtual QString authorUri(const QString &guid) const = 0;
    virtual QString authorEMail(const QString &guid) const = 0;

    virtual void enclosure(const QString &guid, bool &hasEnclosure, QString &url, QString &type, int &length) const = 0;

    virtual void setCategories(const QString & /*guid*/, const QStringList &categories) = 0;

    virtual QStringList categories(const QString &guid) const = 0;
};
} // namespace Backend
} // namespace Akregator

