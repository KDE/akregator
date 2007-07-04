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

#ifndef KFEED_SERIALIZER_H
#define KFEED_SERIALIZER_H

#include "kfeed_export.h"

class QByteArray;

namespace KFeed
{

class Item;

class KFEED_EXPORT Serializer
{
public:
    virtual void serialize( const KFeed::Item& item, QByteArray& array ) const = 0;
    virtual bool deserialize( KFeed::Item& item, const QByteArray& array ) const = 0;
};

class KFEED_EXPORT XmlSerializerImpl : public Serializer
{
public:
    void serialize( const KFeed::Item& item, QByteArray& array ) const;
    bool deserialize( KFeed::Item& item, const QByteArray& array ) const;
};

}

#endif // KFEED_SERIALIZER_H
