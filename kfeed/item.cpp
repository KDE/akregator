
/*
 * This file is part of the kfeed library
 *
 * Copyright (C) 2007 Frank Osterfeld <osterfeld@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "item.h"

#include "category.h"
#include "enclosure.h"
#include "person.h"

#include <KDateTime>

#include <QHash>
#include <QList>
#include <QString>
#include <QVariant>

class KFeed::ItemPrivate
{
public:
    ItemPrivate() : status( New ), hash( 0 ), idIsHash( false ), commentsCount( 0 ), feedId( 0 )
    {}


    bool operator==( const ItemPrivate& other ) const
    {
        return status == other.status 
            && hash == other.hash 
            && idIsHash == other.idIsHash 
            && id == other.id && title == other.title
            && link == other.link
            && description == other.description
            && content == other.content
            && datePublished == other.datePublished
            && dateUpdated == other.dateUpdated
            && authors == other.authors
            && enclosures == other.enclosures
            && categories == other.categories 
            && language == other.language 
            && commentsCount == other.commentsCount
            && commentsLink == other.commentsLink
            && commentsFeed == other.commentsFeed 
            && commentPostUri == other.commentPostUri
            && customProperties == other.customProperties
            && feedId == other.feedId;
    }
    int status;
    int hash;
    bool idIsHash;
    QString id;
    QString title;
    QString link;
    QString description;
    QString content;
    KDateTime datePublished;
    KDateTime dateUpdated;
    QList<KFeed::Person> authors;
    QList<KFeed::Enclosure> enclosures;
    QList<KFeed::Category> categories;
    QString language;
    int commentsCount;
    QString commentsLink;
    QString commentsFeed;
    QString commentPostUri;
    QHash<QString, QVariant> customProperties;
    int feedId;
};

KFeed::Item::Item() : d( new ItemPrivate )
{
}

KFeed::Item::~Item()
{
    delete d;
}

KFeed::Item& KFeed::Item::operator=( const Item& other )
{
    *d = *(other.d);
    return *this;
}

bool KFeed::Item::operator==( const Item& other ) const
{
    return *d == *(other.d);
}

KFeed::Item::Item( const Item& other ) : d( new ItemPrivate )
{
    *d = *(other.d);
}

QString KFeed::Item::title() const
{
    return d->title;
}

void KFeed::Item::setTitle( const QString& title ) 
{
    d->title = title;
}

QString KFeed::Item::description() const
{
    return d->description;
}

void KFeed::Item::setDescription( const QString& description ) 
{
    d->description = description;
}

QString KFeed::Item::link() const
{
    return d->link;
}

void KFeed::Item::setLink( const QString& link ) 
{
    d->link = link;
}

QString KFeed::Item::content() const
{
    return d->content;
}

void KFeed::Item::setContent( const QString& content ) 
{
    d->content = content;
}

KDateTime KFeed::Item::datePublished() const
{
    return d->datePublished;
}

void KFeed::Item::setDatePublished( const KDateTime& datePublished ) 
{
    d->datePublished = datePublished;
}

KDateTime KFeed::Item::dateUpdated() const
{
    return d->dateUpdated;
}

void KFeed::Item::setDateUpdated( const KDateTime& dateUpdated ) 
{
    d->dateUpdated = dateUpdated;
}

QString KFeed::Item::id() const
{
    return d->id;
}

void KFeed::Item::setId( const QString& id ) 
{
    d->id = id;
}

QList<KFeed::Person> KFeed::Item::authors() const
{
    return d->authors;
}

void KFeed::Item::setAuthors( const QList<KFeed::Person>& authors ) 
{
    d->authors = authors;
}

QList<KFeed::Category> KFeed::Item::categories() const
{
    return d->categories;
}

void KFeed::Item::setCategories( const QList<KFeed::Category>& categories ) 
{
    d->categories = categories;
}

QList<KFeed::Enclosure> KFeed::Item::enclosures() const
{
    return d->enclosures;
}

void KFeed::Item::setEnclosures( const QList<KFeed::Enclosure>& enclosures ) 
{
    d->enclosures = enclosures;
}

QString KFeed::Item::language() const
{
    return d->language;
}

void KFeed::Item::setLanguage( const QString& language )
{
    d->language = language;
}

int KFeed::Item::commentsCount() const
{
    return d->commentsCount;
}

void KFeed::Item::setCommentsCount( int commentsCount )
{
    d->commentsCount = commentsCount;
}

QString KFeed::Item::commentsLink() const
{
    return d->commentsLink;
}

void KFeed::Item::setCommentsLink( const QString& commentsLink )
{
    d->commentsLink = commentsLink;
}

QString KFeed::Item::commentsFeed() const
{
    return d->commentsFeed;
}

void KFeed::Item::setCommentsFeed( const QString& commentsFeed )
{
    d->commentsFeed = commentsFeed;
}

QString KFeed::Item::commentPostUri() const
{
    return d->commentPostUri;
}

void KFeed::Item::setCommentPostUri( const QString& commentPostUri )
{
    d->commentPostUri = commentPostUri;
}

bool KFeed::Item::idIsHash() const
{
    return d->idIsHash;
}

void KFeed::Item::setIdIsHash( bool idIsHash )
{
    d->idIsHash = idIsHash;
}

int KFeed::Item::hash() const
{
    return d->hash;
}

void KFeed::Item::setHash( int hash )
{
    d->hash = hash;
}

int KFeed::Item::status() const
{
    return d->status;
}

void KFeed::Item::setStatus( int status )
{
    d->status = status;
}

int KFeed::Item::feedId() const
{
    return d->feedId;
}

void KFeed::Item::setFeedId( int feedId )
{
    d->feedId = feedId;
}

QHash<QString, QVariant> KFeed::Item::customProperties() const
{
    return d->customProperties;
}

QVariant KFeed::Item::customProperty( const QString& key ) const
{
    return d->customProperties[key];
}

void KFeed::Item::setCustomProperty( const QString& key, const QVariant& value )
{
    d->customProperties[key] = value;
}

