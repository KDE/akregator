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

#include "category.h"

#include <QString>

class KFeed::CategoryPrivate
{
public:
    bool operator==( const CategoryPrivate& other ) const
    {
        return term == other.term && scheme == other.scheme && label == other.label;
    }

    QString term;
    QString scheme;
    QString label;
};

KFeed::Category::Category() : d( new CategoryPrivate )
{
}

KFeed::Category::Category( const Category& other) : d( new CategoryPrivate ) 
{
    *d = *(other.d);
}

KFeed::Category::~Category()
{
    delete d;
}

KFeed::Category& KFeed::Category::operator=( const KFeed::Category& other )
{
    *d = *(other.d);
    return *this;
}

bool KFeed::Category::operator==( const KFeed::Category& other ) const
{
    return *d == *(other.d);
}

QString KFeed::Category::term() const
{
    return d->term;
}

void KFeed::Category::setTerm( const QString& term )
{
    d->term = term;
}

QString KFeed::Category::scheme() const
{
    return d->scheme;
}

void KFeed::Category::setScheme( const QString& scheme )
{
    d->scheme = scheme;
}

QString KFeed::Category::label() const
{
    return d->label;
}

void KFeed::Category::setLabel( const QString& label )
{
    d->label = label;
}

