/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "akregator_export.h"
#include "types.h"

#include <QSharedPointer>
#include <Syndication/Person>

class QDateTime;
class QString;

template<class T> class QList;

using uint = unsigned int;

class QUrl;

namespace Syndication
{
class Enclosure;
class Item;
using ItemPtr = QSharedPointer<Item>;
}

namespace Akregator
{
namespace Backend
{
class FeedStorage;
}

class Feed;
/** A proxy class for Syndication::ItemPtr with some additional methods to assist sorting. */
class AKREGATOR_EXPORT Article
{
    friend class ArticleDeleteJob;
    friend class ArticleModifyJob;
    friend class Feed;

public:
    enum ContentOption {
        ContentAndOnlyContent, /*< returns the content field even if empty */
        DescriptionAsFallback /*< uses the description field as fallback if the content field is empty */
    };

    Article();
    /** creates am article object for an existing article.
        The constructor accesses the archive to load it's data
        */
    Article(const QString &guid, Feed *feed, Backend::FeedStorage *archive = nullptr);

    /** creates an article object from a parsed librss Article
        the article is added to the archive if not yet stored, or updated if stored but modified
    */
    Article(const Syndication::ItemPtr &article, Feed *feed);

    Article(const Syndication::ItemPtr &article, Backend::FeedStorage *archive);
    Article(const Article &other);
    ~Article();

    void swap(Article &other)
    {
        std::swap(d, other.d);
    }

    Article &operator=(const Article &other);
    bool operator==(const Article &other) const;
    bool operator!=(const Article &other) const;

    bool isNull() const;

    int status() const;

    QString title() const;
    QUrl link() const;
    QString description() const;

    QString content(ContentOption opt = ContentAndOnlyContent) const;

    QString guid() const;
    /** if true, the article should be kept even when expired **/
    bool keep() const;

    bool isDeleted() const;

    void offsetPubDate(int secs);

    Feed *feed() const;

    /** returns a hash value used to detect changes in articles with non-hash GUIDs. If the guid is a hash itself, it returns @c 0 */

    uint hash() const;

    /** returns if the guid is a hash or an ID taken from the source */

    bool guidIsHash() const;

    bool guidIsPermaLink() const;

    QDateTime pubDate() const;

    QUrl commentsLink() const;

    int comments() const;

    QString authorName() const;
    QString authorUri() const;
    QString authorEMail() const;
    QString authorAsHtml() const;
    QString authorShort() const;

    QSharedPointer<const Syndication::Enclosure> enclosure() const;

    bool operator<(const Article &other) const;
    bool operator<=(const Article &other) const;
    bool operator>(const Article &other) const;
    bool operator>=(const Article &other) const;

private: // only for our friends
    void setStatus(int s);
    void setDeleted();
    void setKeep(bool keep);

private:
    struct Private;
    Private *d;
};
} // namespace Akregator
Q_DECLARE_TYPEINFO(Akregator::Article, Q_MOVABLE_TYPE);
