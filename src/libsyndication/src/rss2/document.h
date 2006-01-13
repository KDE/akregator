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

#ifndef LIBSYNDICATION_RSS2_DOCUMENT_H
#define LIBSYNDICATION_RSS2_DOCUMENT_H

#include "elementwrapper.h"

#include "../abstractdocument.h"


class QDateTime;
class QDomDocument;
class QDomElement;
class QString;

template <class T> class QList;
template <class T> class QSet;

namespace LibSyndication {
namespace RSS2
{

class Category;
class Cloud;
class Image;
class Item;
class TextInput;

class Document : public LibSyndication::AbstractDocument, public ElementWrapper
{
    public:
    
        /**
         * Parses an RSS2 document from an XML document.
         * TODO: More on supported formats etc.
         *
         * @param document The dom document to parse the document from
         * @return the document parsed from XML, or a null object
         *         if parsing failed.
         */
        static Document fromXML(const QDomDocument& document);

        /**
         * Default constructor, creates a null object, for which
         * isNull() is @c true.
         */
        Document();
        
        bool accept(DocumentVisitor* visitor);
    
        /**
         * The title of the channel.
         *
         * @return title TODO: more on escaping/HTML
         */
        QString title() const;
    
        /**
         * The URL to the HTML website corresponding to the channel.
         *
         * @return TODO 
         */
        QString link() const;
    
        /**
         * Phrase or sentence describing the channel.
         *
         * @return TODO
         */
        QString description() const;
    
        /**
         *
         * @return TODO
         */
        QString language() const;
    
        /**
         * Copyright notice for content in the channel.
         *
         * @return TODO
         */
        QString copyright() const;
    
        /**
         * Email address for person responsible for editorial content.
         *
         * @return TODO
         */
        QString managingEditor() const;
    
        /**
         * Email address for person responsible for technical issues relating
         * to channel.
         *
         * @return TODO
         */
        QString webMaster() const;
    
        /** 
         * The publication date for the content in the channel. For example,
         * the New York Times publishes on a daily basis, the publication date
         * flips once every 24 hours. That's when the pubDate of the channel
         * changes.
         *
         * @return TODO
         */
        QDateTime pubDate() const;
    
        /**
         * The last time the content of the channel changed.
         *
         * @return TODO
         */
        QDateTime lastBuildDate() const;
    
        /**
         * Specifies one or more categories that the channel belongs to.
         *
         * @return TODO
         */
        QList<Category> categories() const;
    
        /**
         * A string indicating the program used to generate the channel.
         *
         * @return TODO
         */
        QString generator() const;
    
        /**
         * A URL that points to the documentation for the format used in the
         * RSS file. It's probably a pointer to this page. It's for people who
         * might stumble across an RSS file on a Web server 25 years from now
         * and wonder what it is.
         *
         * @return TODO
         */
        QString docs() const;
    
        /**
         * Allows processes to register with a cloud to be notified of updates
         * to the channel, implementing a lightweight publish-subscribe
         * protocol for RSS feeds.
         *
         * @return TODO
         */
        Cloud cloud() const;
    
        /**
         * ttl stands for time to live. It's a number of minutes that indicates
         * how long a channel can be cached before refreshing from the source.
         *
         * @return TODO
         */
        int ttl() const;
    
        /**
         * Specifies a GIF, JPEG or PNG image that can be displayed with the
         * channel.
         *
         * @return TODO
         */
        Image image() const;
    
        /**
         * Specifies a text input box that can be displayed with the channel.
         *
         * @return TODO
         */
        TextInput textInput() const;
    
        /**
         * Contains a set of hours (from 0 to 23), time in GMT, when the
         * channel is not updated.
         *
         * @return TODO
         */
        QSet<int> skipHours() const;
    
        /** days of week, used for skip days */
        enum DayOfWeek
        {
            
            Monday = 0, /**< self-explanatory */
            Tuesday = 1, /**< self-explanatory */
            Wednesday = 2, /**< self-explanatory */
            Thursday = 3, /**< self-explanatory */
            Friday = 4, /**< self-explanatory */
            Saturday = 5, /**< self-explanatory */
            Sunday = 6 /**< self-explanatory */
        };
    
        /**
         * A set of week days where aggregators shouldn't read the channel.
         *
         * @return TODO
         */
        QSet<DayOfWeek> skipDays() const;
    
        /**
         *
         *
         * @return TODO
         */
        QList<Item> items() const;
    
        /**
         * Returns a description of the object and its children for
         * debugging purposes.
         *
         * @return debug string
         */
        QString debugInfo() const;
    
    private:
    
        Document(const QDomElement& element);
    
};

} // namespace RSS2
} // namespace LibSyndication

#endif // LIBSYNDICATION_RSS2_DOCUMENT_H
