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

#include "person.h"

#include <QString>

class KFeed::PersonPrivate
{
public:

    bool operator==( const PersonPrivate& other ) const
    {
        return name == other.name && email == other.email && uri == other.uri;
    }

    bool operator!=( const PersonPrivate& other ) const
    {
        return name != other.name && email != other.email && uri != other.uri;
    }

    QString name;
    QString email;
    QString uri;
};

QString KFeed::Person::name() const
{
    return d->name;
}

void KFeed::Person::setName( const QString& name )
{
    d->name = name;
}

QString KFeed::Person::email() const
{
    return d->email;
}

void KFeed::Person::setEmail( const QString& email )
{
    d->email = email;
}

QString KFeed::Person::uri() const
{
    return d->uri;
}

void KFeed::Person::setUri( const QString& uri )
{
    d->uri = uri;
}

KFeed::Person::Person() : d( new PersonPrivate )
{
}

KFeed::Person::Person( const Person& other) : d( new PersonPrivate ) 
{
    *d = *(other.d);
}

KFeed::Person::~Person()
{
    delete d;
}

KFeed::Person& KFeed::Person::operator=( const KFeed::Person& other )
{
    *d = *(other.d);
    return *this;
}

bool KFeed::Person::operator==( const KFeed::Person& other ) const
{
    return *d == *(other.d);
}

bool KFeed::Person::operator!=( const KFeed::Person& other ) const
{
    return *d != *(other.d);
}
