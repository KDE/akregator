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
#include "enclosure.h"
#include "item.h"
#include "person.h"
#include "serializer.h"

#include <syndication/atom/constants.h>
#include <syndication/constants.h>

#include <KDateTime>

#include <QHash>
#include <QString>
#include <QVariant>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace {

enum TextMode {
    PlainText,
    Html
};

QString kfeedNamespace()
{
    return "http://akregator.kde.org/kfeed#";
}

class Element
{
public:
    Element( const QString& ns_, const QString& name_, const QVariant& defaultValue_ = QVariant() ) : ns( ns_ ), name( name_ ), qualifiedName( ns + ':' + name ), defaultValue( defaultValue_ )
    {
    }

    const QString ns;
    const QString name;
    const QString qualifiedName;
    const QVariant defaultValue;

    bool isNextIn( const QXmlStreamReader& reader ) const
    {
        return reader.isStartElement() && reader.name() == name && reader.namespaceUri() == ns;
    }

    void writeStartElement( QXmlStreamWriter& writer ) const
    {
        if ( !ns.isNull() )
            writer.writeStartElement( ns, name );
        else
            writer.writeStartElement( name );
    }

    void write( const QVariant& value , QXmlStreamWriter& writer, TextMode mode = ::PlainText ) const
    {
        if ( value == defaultValue )
            return;
        
        if ( ns.isEmpty() )
            writer.writeStartElement( name );
        else
            writer.writeStartElement( ns, name );
        if ( mode == ::Html )
        {
            writer.writeAttribute( "type", "html" );
        }
        const QVariant qv( value );
        Q_ASSERT( qv.canConvert( QVariant::String ) );
        writer.writeCharacters( qv.toString() );
        writer.writeEndElement();
    }
};

struct Elements 
{
    Elements() : atomNS( Syndication::Atom::atom1Namespace() ), 
                 kfeedNS( ::kfeedNamespace() ),
                 commentNS( Syndication::commentApiNamespace() ),
                 title( atomNS, "title", QString() ),
                 summary( atomNS, "summary", QString() ),
                 content( atomNS, "content", QString() ),
                 link( atomNS, "link", QString() ),
                 language( atomNS, "language", QString() ),
                 id( atomNS, "id", QString() ),
                 published( atomNS, "published", KDateTime().toString( KDateTime::ISODate ) ),
                 updated( atomNS, "updated", KDateTime().toString( KDateTime::ISODate ) ),
                 commentsCount( Syndication::slashNamespace(), "comments", -1 ),
                 commentsFeed( commentNS, "commentRss", QString() ),
                 commentPostUri( commentNS, "comment", QString() ),
                 commentsLink( kfeedNS, "commentsLink", QString() ),
                 status( kfeedNS, "status", KFeed::Read ),
                 hash( kfeedNS, "hash", 0 ),
                 idIsHash( kfeedNS, "idIsHash", false ),
                 sourceFeedId( kfeedNS, "sourceFeedId", -1 ),
                 name( atomNS, "name", QString() ),
                 uri( atomNS, "uri", QString() ),
                 email( atomNS, "email", QString() ),
                 author( atomNS, "author", QString() ),
                 category( atomNS, "category", QString() ),
                 customProperty( kfeedNS, "customProperty", QString() ),
                 key( kfeedNS, "key", QString() ),
                 value( kfeedNS, "value", QString() ),
                 entry( atomNS, "entry", QString() )
{}
    const QString atomNS;
    const QString kfeedNS;
    const QString commentNS;
    const Element title;
    const Element summary;
    const Element content;
    const Element link;
    const Element language;
    const Element id;
    const Element published;
    const Element updated;
    const Element commentsCount;
    const Element commentsFeed;
    const Element commentPostUri;
    const Element commentsLink;
    const Element status;
    const Element hash;
    const Element idIsHash;
    const Element sourceFeedId;
    const Element name;
    const Element uri;
    const Element email;
    const Element author;
    const Element category;
    const Element customProperty;
    const Element key;
    const Element value;
    const Element entry;
    static const Elements instance;
};

const Elements Elements::instance;

void writeAttributeIfNotEmpty( const QString& ns, const QString& element, const QVariant& value, QXmlStreamWriter& writer )
{
    const QString text = value.toString();
    if ( text.isEmpty() )
        return;
    writer.writeAttribute( ns, element, text );
}

void writeAttributeIfNotEmpty( const QString& element, const QVariant& value, QXmlStreamWriter& writer )
{
    const QString text = value.toString();
    if ( text.isEmpty() )
        return;
    writer.writeAttribute( element, text );
}

void writeLink( const QString& url, QXmlStreamWriter& writer )
{
    if ( url.isEmpty() )
        return;
    ::Elements::instance.link.writeStartElement( writer );
    writer.writeAttribute( "rel", "alternate" );
    writeAttributeIfNotEmpty( "href", url, writer );
    writer.writeEndElement();
}

void writeCategory( const KFeed::Category& category, QXmlStreamWriter& writer )
{
    ::Elements::instance.category.writeStartElement( writer );
    writeAttributeIfNotEmpty( "term", category.term(), writer );
    writeAttributeIfNotEmpty( "scheme", category.scheme(), writer );
    writeAttributeIfNotEmpty( "label", category.label(), writer );
    writer.writeEndElement();
}

void writeAuthor( const KFeed::Person& person, QXmlStreamWriter& writer )
{
    const ::Elements el;
    const QString atomNS = Syndication::Atom::atom1Namespace();
    ::Elements::instance.author.writeStartElement( writer );
    ::Elements::instance.name.write( person.name(), writer );
    ::Elements::instance.uri.write( person.uri(), writer );
    ::Elements::instance.email.write( person.email(), writer );
    writer.writeEndElement();
}

void writeEnclosure( const KFeed::Enclosure& enclosure, QXmlStreamWriter& writer )
{
    ::Elements::instance.link.writeStartElement( writer );
    writeAttributeIfNotEmpty( "rel", "enclosure", writer );
    writeAttributeIfNotEmpty( "href", enclosure.url(), writer );
    writeAttributeIfNotEmpty( "title", enclosure.title(), writer );
    writeAttributeIfNotEmpty( "length", enclosure.length(), writer );
    writeAttributeIfNotEmpty( "type", enclosure.type(), writer );
    const uint duration = enclosure.duration();
    if ( duration != 0 )
        writer.writeAttribute( Syndication::itunesNamespace(), "duration", QString::number( duration ) );
    writer.writeEndElement();
}

void writeItem( const KFeed::Item& item, QXmlStreamWriter& writer )
{
    const ::Elements el;
    const QString atomNS = Syndication::Atom::atom1Namespace();
    const QString commentNS = Syndication::commentApiNamespace();
    const QString kfeedNS = ::kfeedNamespace(); 
    writer.writeDefaultNamespace( atomNS );
    writer.writeNamespace( commentNS, "comment" );
    writer.writeNamespace( kfeedNS, "kfeed" );
    writer.writeNamespace( Syndication::itunesNamespace(), "itunes" );

    el.entry.writeStartElement( writer );
    el.title.write( item.title(), writer, ::Html );
    const QString description = item.description();
    el.summary.write( description, writer, ::Html );
    const QString content = item.content();
    if ( content != description )
        el.content.write( content, writer, ::Html );
    ::writeLink( item.link(), writer );
    el.language.write( item.language(), writer );
    el.id.write( item.id(), writer );

    const KDateTime updated = item.dateUpdated();
    const KDateTime published = item.datePublished();
    el.published.write( published.toString( KDateTime::ISODate ), writer );
    if ( updated.isValid() && updated != published )
    {
        el.updated.write( updated.toString( KDateTime::ISODate ), writer );
    }

    el.commentsFeed.write( item.commentsFeed(), writer );
    el.commentPostUri.write( item.commentPostUri(), writer );
    el.commentsCount.write( item.commentsCount(), writer );
    el.commentsLink.write( item.commentsLink(), writer );

    Q_FOREACH( const KFeed::Category i, item.categories() )
    {
        ::writeCategory( i, writer );
    }
    Q_FOREACH( const KFeed::Person i, item.authors() )
    {
        ::writeAuthor( i, writer );
    }
    Q_FOREACH( const KFeed::Enclosure i, item.enclosures() )
    {
        ::writeEnclosure( i, writer );
    }

    el.status.write( item.status(), writer );
    el.hash.write( item.hash(), writer );
    el.idIsHash.write( item.idIsHash(), writer );
    el.sourceFeedId.write( item.sourceFeedId(), writer );

    const QHash<QString, QString> props = item.customProperties();
    Q_FOREACH ( const QString i, props.keys() )
    {
        el.customProperty.writeStartElement( writer );
        el.key.write( i, writer );
        el.value.write( props[i], writer );
        writer.writeEndElement();
    }

    writer.writeEndElement();
}

void readLink( KFeed::Item& item, QXmlStreamReader& reader )
{
    const QXmlStreamAttributes attrs = reader.attributes();
    const QString rel = attrs.value( QString(), "rel" ).toString();
    if (  rel == "alternate" )
    {
        item.setLink( attrs.value( QString(), "href" ).toString() );
    }
    else if ( rel == "enclosure" )
    {
        KFeed::Enclosure enc;
        enc.setUrl( attrs.value( QString(), "href" ).toString() );
        enc.setType( attrs.value( QString(), "type" ).toString() );
        enc.setTitle( attrs.value( QString(), "title" ).toString() );
        bool ok;
        const uint length = attrs.value( QString(), "length" ).toString().toUInt( &ok );
        if ( ok )
            enc.setLength( length );
        const uint duration = attrs.value( Syndication::itunesNamespace(), "duration" ).toString().toUInt( &ok );
        if ( ok )
            enc.setDuration( duration );
        QList<KFeed::Enclosure> encs = item.enclosures();
        encs.append( enc );
        item.setEnclosures( encs );
    }
}

void readAuthor( KFeed::Item& item, QXmlStreamReader& reader )
{
    KFeed::Person author;
    int depth = 1;
    while ( !reader.atEnd() && depth > 0 )
    {
        reader.readNext();
        if ( reader.isEndElement() )
            --depth;
        else if ( reader.isStartElement() )
        {
            if ( ::Elements::instance.name.isNextIn( reader ) )
                author.setName( reader.readElementText() );
            else if ( ::Elements::instance.uri.isNextIn( reader ) )
                author.setUri( reader.readElementText() );
            else if ( ::Elements::instance.email.isNextIn( reader ) )
                author.setEmail( reader.readElementText() );
        }

    }
    QList<KFeed::Person> authors = item.authors();
    authors.append( author );
    item.setAuthors( authors );
}

void readCategory( KFeed::Item& item, QXmlStreamReader& reader )
{
    const QXmlStreamAttributes attrs = reader.attributes();
    KFeed::Category cat;
    cat.setTerm( attrs.value( QString(), "term" ).toString() );
    cat.setScheme( attrs.value( QString(), "scheme" ).toString() );
    cat.setLabel( attrs.value( QString(), "label" ).toString() );
    QList<KFeed::Category> cats = item.categories();
    cats.append( cat );
    item.setCategories( cats );
}

void readCustomProperty( KFeed::Item& item, QXmlStreamReader& reader )
{
    QString key;
    QString value;
    int depth = 1;
    while ( !reader.atEnd() && depth > 0 )
    {
        reader.readNext();
        if ( reader.isEndElement() )
            --depth;
        else if ( reader.isStartElement() )
        {
            if ( ::Elements::instance.key.isNextIn( reader ) )
                key = reader.readElementText();
            else if ( ::Elements::instance.value.isNextIn( reader ) )
                value = reader.readElementText();
        }
    }
    item.setCustomProperty( key, value );
}

} // namespace

void KFeed::XmlSerializerImpl::serialize( const KFeed::Item& item, QByteArray& array ) const 
{
    QXmlStreamWriter writer( &array );
    writer.writeStartDocument();
    ::writeItem( item, writer );
    writer.writeEndDocument();
}

bool KFeed::XmlSerializerImpl::deserialize( KFeed::Item& item, const QByteArray& array ) const
{
    QXmlStreamReader reader( array );
    reader.setNamespaceProcessing( true );
    const ::Elements el;

    while ( !reader.atEnd() )
    {
        reader.readNext();
        if ( reader.isStartElement() )
        {
            if ( el.title.isNextIn( reader ) )
                item.setTitle( reader.readElementText() );
            else if ( el.summary.isNextIn( reader ) )
                item.setDescription( reader.readElementText() );
            else if ( el.content.isNextIn( reader ) )
                item.setContent( reader.readElementText() );
            else if ( el.language.isNextIn( reader ) )
                item.setLanguage( reader.readElementText() );
            else if ( el.id.isNextIn( reader ) )
                item.setId( reader.readElementText() );
            else if ( el.status.isNextIn( reader ) )
                item.setStatus( reader.readElementText().toInt() );
            else if ( el.hash.isNextIn( reader ) )
                item.setHash( reader.readElementText().toInt() );
            else if ( el.idIsHash.isNextIn( reader ) )
                item.setIdIsHash( QVariant( reader.readElementText() ).toBool() );
            else if ( el.sourceFeedId.isNextIn( reader ) )
                item.setSourceFeedId( reader.readElementText().toInt() );
            else if ( el.commentsLink.isNextIn( reader ) )
                item.setCommentsLink( reader.readElementText() );
            else if ( el.commentPostUri.isNextIn( reader ) )
                item.setCommentPostUri( reader.readElementText() );
            else if ( el.commentsCount.isNextIn( reader ) )
                item.setCommentsCount( reader.readElementText().toInt() );
            else if ( el.commentsFeed.isNextIn( reader ) )
                item.setCommentsFeed( reader.readElementText() );
            else if ( el.link.isNextIn( reader ) )
                ::readLink( item, reader );
            else if ( el.author.isNextIn( reader ) )
                ::readAuthor( item, reader );
            else if ( el.category.isNextIn( reader ) )
                ::readCategory( item, reader );
            else if ( el.published.isNextIn( reader ) )
                item.setDatePublished( KDateTime::fromString( reader.readElementText(), KDateTime::ISODate ) );
            else if ( el.updated.isNextIn( reader ) )
                item.setDateUpdated( KDateTime::fromString( reader.readElementText(), KDateTime::ISODate ) );
            else if ( el.customProperty.isNextIn( reader ) )
                ::readCustomProperty( item, reader );

/*
            customProperties */
        }
    }
    return !reader.hasError();
}
