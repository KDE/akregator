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

#include "testenclosure.h"
#include "enclosure.h"

#include <qtest_kde.h>

void TestEnclosure::testDefaultValues()
{
    const KFeed::Enclosure enc;
    QCOMPARE( enc.url(), QString() );
    QCOMPARE( enc.type(), QString() );
    QCOMPARE( enc.title(), QString() );
    QCOMPARE( enc.duration(), static_cast<uint>( 0 ) );
    QCOMPARE( enc.length(), static_cast<uint>( 0 ) );
}

QTEST_KDEMAIN( TestEnclosure, NoGUI )

#include "testenclosure.moc"
