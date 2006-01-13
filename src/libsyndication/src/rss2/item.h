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

#include "../elementwrapper.h"

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
 * An Item, representing an entry in an RSS feed.
 *
 * @author Frank Osterfeld
 */
class Item : public ElementWrapper
{
    public:
        
        /**
         * Parses an item object from an <item> XML element.
         *
         * @param e The <item> element to parse the item from
         * @return the item parsed from XML, or a null object
         *         if parsing failed.
         */
        static Item fromXML(const QDomElement& e);

        /**
         * Default constructor, creates a null object, for which isNull() is
         * @c true.
         */
        Item();
      
        /**
         * The title of the item.
         * 
         * @return The title in plain text. Note that e.g. characters like <,
         * >, & are not escaped!
         * (TODO: this might change, check what makes more sense)
         */
        QString title() const;

        /**
         * The URL of the item. This usually links to the web representation
         * of the item, e.g. the full news article.
         * 
         * @return an URL, or QString::null if not set
         */
        QString link() const;

        /**
         * The item synopsis. This might contain a short summary of the
         * item, but also the full content. If content() is set, that usually
         * contains the full content instead.
         * 
         * @return a string in HTML format (whitespace is irrelevant, <br/> is
         * used for newlines, "&", "<", ">" are escaped) summarizing the item.
         * QString::null if no description was specified.
         */
        QString description() const;
    
        /**
         * Returns the actual content of the item. In RSS2, this can be stored
         * in multiple structs, e.g. in content:encoded, xhtml:body or
         * xhtml:div. If this is not set, description() might also contain the
         * content of the item.
         * 
         * @return the content in HTML format (whitespace is irrelevant, <br/> is used for newlines, "&", "<", ">" are escaped) If no content is specified, QString::null is returned.
         */
        QString content() const;

        /**
         * Set of categories this item is included in.
         * 
         * @return a list of categories, possibly empty.
         */
        QList<Category> categories() const;

        /**
         * URL of a page for comments relating to the item.
         * 
         * @return an URL to the comments, or QString::null if not set
         */
        QString comments() const;

        /**
         * The email address of the author of this item. For newspapers and
         * magazines syndicating via RSS, the author is the person who wrote
         * the article that this item describes. For collaborative weblogs, the
         * author of the item might be different from the managing editor or
         * webmaster.
         * 
         * @return an email address of the author, or QString::null if not
         * specified
         */
        QString author() const;

        /**
         * Describes a media object that is attached to the item.
         * See Enclosure for more details.
         * 
         * @return an enclosure object if set, or a null object (see
         * Enclosure.isNull()) otherwise.
         */
        Enclosure enclosure() const;

        /**
         * "guid stands for globally unique identifier. It's a string that
         * uniquely identifies the item. When present, an aggregator may choose
         * to use this string to determine if an item is new.
         * There are no rules for the syntax of a guid. Aggregators must view
         * them as a string. It's up to the source of the feed to establish the
         * uniqueness of the string."
         * 
         * @return a guid string, or QString::null if none specified in the
         * feed
         */
        QString guid() const;

        /**
         * If @c true, it can be assumed that the guid is a permalink to the
         * item, that is, a url that can be opened in a Web browser, that
         * points to the full item.
         *
         * @return @c true if the guid is a permalink and can be interpreted as
         * URL
         */
         bool guidIsPermaLink() const;

        /**
         * Indicates when the item was published. If it's a date in the future,
         * yous may choose to not display the item until that date.
         * 
         * @return the publication date, or an invalid QDateTime object if
         * either no date was specified or parsing failed.
         */
        QDateTime pubDate() const;

        /** 
         * The RSS channel that the item came from. See Source class for more
         * information.
         * 
         * @return a Source object, or a null object (see Source.isNull()) if
         * not set.
         */
        Source source() const;
    
        /**
         * Returns a description of the object and its children for debugging
         * purposes.
         *
         * @return debug string
         */
        QString debugInfo() const;

    private:

        Item(const QDomElement& element);
};

} // namespace RSS2
} // namespace LibSyndication

#endif // LIBSYNDICATION_RSS2_ITEM_H
