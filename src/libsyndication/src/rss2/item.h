/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>
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

#ifndef LIBSYNDICATION_RSS2_ITEM_H
#define LIBSYNDICATION_RSS2_ITEM_H

#include <ksharedptr.h>

class QDateTime;
class QDomDocument;
class QDomElement;
class QString;
template <class T> class QList;

namespace LibSyndication {
namespace RSS2 {

class Category;
class Enclosure;
class Source;

/**
 * An Item, representing the entries in an RSS feed.
 *
 * @author Frank Osterfeld
 */
class Item
{
    public:

        /**
         * static null object. See also Item() and isNull().
         *
         * @return reference to a static null object
         */
        static const Item& null();

        /**
         * Parses an item object from an <item> XML element.
         *
         * @param e The <item> element to parse the item from
         * @return the item parsed from XML, or a null object
         *         if parsing failed.
         */
        static Item fromXML(const QDomElement& e);

        /**
         * Default constructor, creates a null object, which is equal
         * to Item::null() and for which isNull() is @c true.
         */
        Item();

        /**
         * Copy constructor, creates a copy of @c other.
         * The d pointer is shared, so this is a cheap operation.
         *
         * @param other the object to be copied
         */
        Item(const Item& other);

        /**
         * Destructor.
         */
        virtual ~Item();

        /**
         * Assigns the values of @c other. The d pointer is shared, so
         * this is a cheap operation.
         *
         * @param other The object to assign
         * @return a reference to this object
         */
        Item& operator=(const Item& other);

        /**
         * Checks whether this item is equal to another.
         * Items are equal if all properties and contained elements (enclosures etc.) are equal.
         *
         * @param other another item
         * @return whether this object is equal to @c other or not
         */
        bool operator==(const Item& other) const;

        /**
         * returns whether this object is a null object.
         *
         * @return @c true, if this is a null object, otherwise @c false
         */
        bool isNull() const;

        /**
         * The title of the item.
         */
        QString title() const;

        /**
         * The URL of the item.
         */
        QString link() const;

        /**
         * The item synopsis.
         */
        QString description() const;
    
        /** returns data stored in content:encoded, xhtml:body, xhtml:div */
        QString content() const;

        /**
         * Set of categories this item is included in.
         */
        QList<Category> categories() const;

        /**
         * URL of a page for comments relating to the item.
         */
        QString comments() const;

        /**
         * The email address of the author of this item. For newspapers and
         * magazines syndicating via RSS, the author is the person who wrote
         * the article that this item describes. For collaborative weblogs, the
         * author of the item might be different from the managing editor or
         * webmaster.
         */
        QString author() const;

        /**
         * Describes a media object that is attached to the item.
         * See Enclosure for more details.
         */
        Enclosure enclosure() const;

        /**
         * "guid stands for globally unique identifier. It's a string that
         * uniquely identifies the item. When present, an aggregator may choose
         * to use this string to determine if an item is new.
         * There are no rules for the syntax of a guid. Aggregators must view
         * them as a string. It's up to the source of the feed to establish the
         * uniqueness of the string."
         */
        QString guid() const;

        /**
         * If @c true, it can be assumed that the guid is a permalink to the
         * item, that is, a url that can be opened in a Web browser, that
         * points to the full item.
         *
         * @return @c true if the guid is a permalink and can be interpreted as
         *                 URL
         */
         bool guidIsPermaLink() const;

        /**
         * Indicates when the item was published. If it's a date in the future,
         * yous may choose to not display the item until that date. 
         */
        QDateTime pubDate() const;

        /** 
         * The RSS channel that the item came from.
         */
        Source source() const;
    
        /**
         * Returns a description of the object and its
         * children for debugging purposes.
         *
         * @return debug string
         */
        QString debugInfo() const;

    private:

    Item(const QString& title, const QString& link, const QString& description,
         const QString& content, const QList<Category>& categories,
         const QString& comments, const QString& author, 
         const Enclosure& enclosure, const QString& guid, bool guidIsPermaLink,
         const QDateTime& pubDate, const Source& source);

    static Item* m_null;

    class ItemPrivate;
    KSharedPtr<ItemPrivate> d;
};

} // namespace RSS2
} // namespace LibSyndication

#endif // LIBSYNDICATION_RSS2_ITEM_H
