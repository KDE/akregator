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

#include "testxmlserializer.h"

#include "enclosure.h"
#include "person.h"
#include "category.h"
#include "item.h"
#include "serializer.h"

#include <KDateTime>

#include <qtest_kde.h>

#include <QDebug>
#include <QByteArray>
#include <QVariant>

namespace {

void printItem( const KFeed::Item& item )
{
    const KFeed::XmlSerializerImpl serializer;
    QByteArray ba;
    serializer.serialize( item, ba );
    qDebug() << ba;
}

void testItem( const KFeed::Item& item )
{
    const KFeed::XmlSerializerImpl serializer;
    QByteArray ba;
    serializer.serialize( item, ba );
    KFeed::Item deserialized;
    const bool success = serializer.deserialize( deserialized, ba );
    QVERIFY2( success, "Deserialization failed" );
    if ( item != deserialized )
    {
        ::printItem( item );
        ::printItem( deserialized );
    }
    QCOMPARE( item, deserialized );
}

}

void TestXmlSerializer::testEmptyItem()
{
    ::testItem( KFeed::Item() );
}

void TestXmlSerializer::testDates()
{
    KFeed::Item item;
    const KDateTime updated = KDateTime::currentLocalDateTime();
    const KDateTime published = updated.addDays( -4 );
    item.setDatePublished( published );
    item.setDateUpdated( updated );
    ::testItem( item );
}

void TestXmlSerializer::testSimpleItems()
{
    KFeed::Item item;
    item.setTitle( "Some title" );
    item.setDescription( "Some description" );
    item.setLink( "http://akregator.kde.org" );
    item.setContent( "Content makes the world go round" );
    item.setId( "http://uniqueid" );
    item.setIdIsHash( true );
    item.setLanguage( "en" );
    item.setHash( 5 );
    ::testItem( item );
    item.setHash( 0 );
    ::testItem( item );
}

void TestXmlSerializer::testStatus()
{
    KFeed::Item item;
    item.setStatus( KFeed::Read );
    ::testItem( item );
    item.setStatus( KFeed::New );
    ::testItem( item );
    item.setStatus( KFeed::Unread );
    ::testItem( item );
    item.setStatus( KFeed::Unread | KFeed::Important );
    ::testItem( item );
    item.setStatus( KFeed::Read | KFeed::Important );
    ::testItem( item );
}

void TestXmlSerializer::testCustomProperties()
{
    KFeed::Item item;
    item.setId( "http://uniqueid" );
    item.setTitle( "Some title" );
    item.setDescription( "Some description" );
    item.setLink( "http://akregator.kde.org" );
    item.setCustomProperty( "foo", "bar" );
    item.setCustomProperty( "bar", "foo" );
    item.setCustomProperty( "foobar", QString() );
    ::testItem( item );
}

void TestXmlSerializer::testEnclosures()
{
    KFeed::Item item;
    item.setLink( "http://akregator.kde.org" );
    KFeed::Enclosure enc;
    enc.setUrl( "http://akregator.kde.org/some.mp3" );
    enc.setType( "audio/mpeg" );
    enc.setTitle( "This is an enclosure!" );
    enc.setLength( 123456789 );
    KFeed::Enclosure enc2 = enc;
    enc.setDuration( 60 );
    QList<KFeed::Enclosure> encs;
    encs.append( enc );
    encs.append( enc2 );
    item.setEnclosures( encs );
    ::testItem( item );
}

void TestXmlSerializer::testCategories()
{
    KFeed::Category cat;
    cat.setTerm( "term1" );
    cat.setScheme( "http://Blabla#" );
    cat.setLabel( "Term 1" );
    KFeed::Category cat2;
    cat2.setTerm( "Term2" );
    cat2.setLabel( "Tada" );
    QList<KFeed::Category> cats;
    cats.append( cat );
    cats.append( cat2 );
    cats.append( KFeed::Category() );
    KFeed::Item item;
    item.setCategories( cats );
    ::testItem( item );
}

void TestXmlSerializer::testAuthors()
{
    KFeed::Person a1;
    a1.setName( "John Doe" );
    KFeed::Person a2;
    a2.setName( "John Doe" );
    a2.setUri( "http://doeweb.net/John" );
    a2.setEmail( "joe@doeweb.net" );
    KFeed::Person a3;
    QList<KFeed::Person> authors;
    authors.append( a1 );
    authors.append( a2 );
    authors.append( a3 );
    KFeed::Item item;
    item.setAuthors( authors );
    ::testItem( item );
}

void TestXmlSerializer::testComments()
{
    KFeed::Item item;
    item.setCommentsCount( 10 );
    item.setCommentsLink( "http://heyho#comment" );
    item.setCommentsFeed( "http://heyho/comments.rss" );
    item.setCommentPostUri( "http://whatever" );
    ::testItem( item );
}


QTEST_KDEMAIN( TestXmlSerializer, NoGUI )

#include "testxmlserializer.moc"
