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

#include "enclosure.h"

#include <QString>

struct KFeed::EnclosurePrivate
{
public:
    EnclosurePrivate() : length( 0 ), duration( 0 )
    {
    }

    bool operator==( const EnclosurePrivate& other ) const
    {
        return url == other.url && title == other.title && type == other.type && length == other.length && duration == other.duration;
    }

    bool operator!=( const EnclosurePrivate& other ) const
    {
        return url != other.url && title != other.title && type != other.type && length != other.length && duration != other.duration;
    }

    QString url;
    QString title;
    QString type;
    uint length;
    uint duration;
};

KFeed::Enclosure::Enclosure() : d( new EnclosurePrivate )
{
}

KFeed::Enclosure::Enclosure( const Enclosure& other) : d( new EnclosurePrivate ) 
{
    *d = *(other.d);
}

KFeed::Enclosure::~Enclosure()
{
    delete d;
}

KFeed::Enclosure& KFeed::Enclosure::operator=( const KFeed::Enclosure& other )
{
    *d = *(other.d);
    return *this;
}

bool KFeed::Enclosure::operator==( const KFeed::Enclosure& other ) const
{
    return *d == *(other.d);
}


bool KFeed::Enclosure::operator!=( const KFeed::Enclosure& other ) const
{
    return *d != *(other.d);
}

QString KFeed::Enclosure::url() const
{
    return d->url;
}

void KFeed::Enclosure::setUrl( const QString& url )
{
    d->url = url;
}

QString KFeed::Enclosure::title() const
{
    return d->title;
}

void KFeed::Enclosure::setTitle( const QString& title )
{
    d->title = title;
}

QString KFeed::Enclosure::type() const
{
    return d->type;
}

void KFeed::Enclosure::setType( const QString& type )
{
    d->type = type;
}

uint KFeed::Enclosure::length() const
{
    return d->length;
}

void KFeed::Enclosure::setLength( uint length )
{
    d->length = length;
}

uint KFeed::Enclosure::duration() const
{
    return d->duration;
}

void KFeed::Enclosure::setDuration( uint duration )
{
    d->duration = duration;
}

