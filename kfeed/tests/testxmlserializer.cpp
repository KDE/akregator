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
#include "serializer.h"

#include <QTest>

#include <QByteArray>


namespace {

void testItem( const KFeed::Item& item )
{
    const KFeed::XmlSerializerImpl serializer;
    QByteArray ba;
    serializer.serialize( item, ba );
    KFeed::Item deserialized;
    const bool success = serializer.deserialize( deserialized, ba );
    QVERIFY2( success, "Deserialization failed" );
    QCOMPARE( item, deserialized );
}

}

int main( char** argv, int argc )
{
    ::testItem( KFeed::Item() );
}
