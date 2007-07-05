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

#include "testitem.h"
#include "item.h"

#include <KDateTime>

#include <qtest_kde.h>

void TestItem::testDefaultValues()
{
    KFeed::Item item;
    QCOMPARE( item.commentsCount(), -1 );
    QCOMPARE( item.status(), static_cast<int>( KFeed::Read ) );
    QCOMPARE( item.idIsHash(), false );
    QCOMPARE( item.sourceFeedId(), -1 );
    QCOMPARE( item.hash(), 0 );
}

void TestItem::testNullDates()
{
    KFeed::Item item;
    QVERIFY( !item.datePublished().isValid() );
    QVERIFY( !item.dateUpdated().isValid() );
}

void TestItem::testSingleNullDates()
{
    KFeed::Item item1;
    const KDateTime now = KDateTime::currentLocalDateTime();
    item1.setDateUpdated( now );
    QVERIFY( !item1.datePublished().isValid() );
    QCOMPARE( item1.dateUpdated(), now );

    KFeed::Item item2;
    item2.setDatePublished( now );
    QCOMPARE( item2.datePublished(), now );
    // dateUpdated() must return datePublished() if no updated() date is set
    QCOMPARE( item2.dateUpdated(), item2.datePublished() );
    const KDateTime tomorrow = now.addDays( 1 );
    item2.setDatePublished( tomorrow );
    QCOMPARE( item2.datePublished(), tomorrow );
    QCOMPARE( item2.dateUpdated(), item2.datePublished() );
}

void TestItem::testContentAndDescription()
{
    // content() must return description() if no content is set
    const QString desc1 = "Hello! I'm a description!";
    const QString desc2 = "Hello! I'm another description!";
    const QString content = "Hi there. Content is king!";
    KFeed::Item item;
    item.setDescription( desc1 );
    QCOMPARE( item.description(), desc1 );
    QCOMPARE( item.content(), desc1 );
    item.setContent( content );
    QCOMPARE( item.description(), desc1 );
    QCOMPARE( item.content(), content );
    item.setContent( QString() );
    QCOMPARE( item.description(), desc1 );
    QCOMPARE( item.content(), desc1 );
    item.setDescription( desc2 );
    item.setContent( content );
    QCOMPARE( item.description(), desc2 );
    QCOMPARE( item.content(), content );
}

QTEST_KDEMAIN( TestItem, NoGUI )

#include "testitem.moc"
