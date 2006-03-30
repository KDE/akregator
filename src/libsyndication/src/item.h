/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2006 Frank Osterfeld <frank.osterfeld@kdemail.net>
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

#ifndef LIBSYNDICATION_ITEM_H
#define LIBSYNDICATION_ITEM_H

#include "sharedptr.h"

#include <kdepimmacros.h>

#include <ctime>

template <class T> class QList;

namespace LibSyndication {

class Category;
typedef SharedPtr<Category> CategoryPtr;
class Enclosure;
typedef SharedPtr<Enclosure> EnclosurePtr;
class Item;
typedef SharedPtr<Item> ItemPtr;
class Person;
typedef SharedPtr<Person> PersonPtr;
class SpecificItem;
typedef SharedPtr<SpecificItem> SpecificItemPtr;

/**
 * An item from a news feed.
 * 
 * @author Frank Osterfeld
 */
class KDE_EXPORT Item
{
    public:

        /** 
         * destructor
         */
        virtual ~Item();
        
                
        /**
         * returns the format-specific item this object abstracts from.
         * Use it if you need to access format-specifics that are not covered
         * by this abstraction.
         * 
         */
        virtual SpecificItemPtr specificItem() const = 0;

        /**
         * The title of the item.
         * 
         * This string may contain HTML markup.(Importantly, occurrences of
         * the characters &lt;,'\n', '&amp;', '\'' and  '\"' are escaped).
         * 
         * @return the title of the item as HTML, or a null string if not
         * specified
         */
        virtual QString title() const = 0;
        
        /**
         * returns a link to the (web) resource described by this item. In most 
         * cases, this will be a website containing the full article associated
         * with this item.
         * 
         * @return a URL, or a null string if not specified
         */
        virtual QString link() const = 0;
        
        /**
         * returns the description of the item. The description can either be 
         * a tag line, a short summary of the item content up to a complete 
         * article. If content() is non-empty, it 
         * 
         * This string may contain HTML markup. (Importantly, occurrences of
         * the characters &lt;,'\n', '&amp;', '\'' and  '\"' are escaped).
         * 
         * @return the description as HTML, or a null string if not specified
         */
        virtual QString description() const = 0;
        
        /**
         * returns the content of the item. If provided, this is the most
         * comprehensive text content available for this item. If it is empty,
         * use description() (which might also contain complete article
         * content).
         * 
         * This string may contain HTML markup. (Importantly, occurrences of
         * the characters &lt;,'\n', '&amp;', '\'' and  '\"' are escaped).
         *
         * @return content string as HTML, or a null string if not set
         */
        virtual QString content() const = 0;
        
        /**
         * returns the date when the item was initially published.
         * 
         * @return publication date, as seconds since epoch (Jan 1st 1970), or 0
         * (epoch) if not set
         */
        virtual time_t datePublished() const = 0;
        
        /**
         * returns the date when the item was modified the last time. If no such
         * date is provided by the feed, this method returns the value of 
         * datePublished().
         *
         * @return modification date, as seconds since epoch (Jan 1st 1970)
         */
        virtual time_t dateUpdated() const = 0;
        
        /**
         * returns an identifier that identifies the item within its 
         * feed. The ID must be unique within its feed. If no ID is provided
         * by the feed source, a hash from title, description and content is
         * returned.
         * Generated hash IDs start with "hash:".
         */
        virtual QString id() const = 0;

        /**
         * returns a list of persons who created the item content. If there is a
         * distinction between authors and contributors (Atom), both are added
         * to the list, where authors are added first.
         * 
         * @return list of authors (and possibly other contributing persons)
         */
        virtual QList<PersonPtr> authors() const = 0;
        
        /**
         * returns the language used in the item's content
         * 
         * @return TODO: tell about language codes and link them
         */
        virtual QString language() const = 0;
        
        /**
         * returns a list of enclosures describing files available on the net.
         * (often used for audio files, so-called "Podcasts").
         * 
         * @return a list of enclosures associated with this item
         */
        virtual QList<EnclosurePtr> enclosures() const = 0;
        
        /**
         * returns a list of categories this item is filed in.
         * See Category for more information on categories.
         *
         * @return a list of categories
         */
        virtual QList<CategoryPtr> categories() const = 0;
        
        /**
         * The number of comments posted for this item.
         *
         * @return the number of comments associated to this item, or -1 if not
         * specified
         */
        virtual int commentsCount() const = 0;
        
        /**
         * Link to an HTML site which contains the comments belonging to
         * this item.
         * 
         * @return URL to the comments page, or a null string if not set
         */
        virtual QString commentsLink() const = 0;
        
        /**
         * URL of feed syndicating comments belonging to this item.
         * 
         * @return comments feed URL, or a null string if not set
         */
        virtual QString commentsFeed() const = 0;
        
        /**
         * URI that can be used to post comments via an HTTP POST request using
         * the Comment API.
         * For more details on the Comment API, see 
         * http://wellformedweb.org/story/9
         * 
         * @return URI for posting comments, or a null string if not set
         */
        virtual QString commentPostUri() const = 0;
       
        /**
         * returns a description of the item for debugging purposes
         * 
         * @return debug string
         */
        virtual QString debugInfo() const;
};
    
} // namespace LibSyndication

#endif // LIBSYNDICATION_ITEM_H
