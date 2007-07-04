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

#ifndef KFEED_ENCLOSURE_H
#define KFEED_ENCLOSURE_H

#include "kfeed_export.h"

class QString;

typedef unsigned int uint;

namespace KFeed {

class EnclosurePrivate;

class KFEED_EXPORT Enclosure
{
public:
    Enclosure();
    Enclosure( const Enclosure& other );
    ~Enclosure();

    QString url() const;
    void setUrl( const QString& url );

    QString title() const;
    void setTitle( const QString& title );

    QString type() const;
    void setType( const QString& type );

    uint length() const;
    void setLength( uint length );

    uint duration() const;
    void setDuration( uint duration );

    Enclosure& operator=( const Enclosure& other );
    bool operator==( const Enclosure& other ) const;

private:
    EnclosurePrivate* const d;
};

} // namespace KFeed

#endif // KFEED_ENCLOSURE_H
