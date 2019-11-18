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
#ifndef AKREGATOR_BACKEND_FEEDSTORAGE_H
#define AKREGATOR_BACKEND_FEEDSTORAGE_H

#include <QObject>
class QString;
class QStringList;
class QDateTime;

namespace Akregator {
namespace Backend {
class Storage;

class FeedStorage : public QObject //krazy:exclude=qobject
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

#endif // AKREGATOR_BACKEND_FEEDSTORAGE_H
