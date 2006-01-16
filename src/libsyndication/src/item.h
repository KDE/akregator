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

#include <ksharedptr.h>

#include <ctime>

namespace LibSyndication {

class Enclosure;
class Item;
typedef KSharedPtr<Item> ItemPtr;
    
class Item : public KShared
{
    public:
        
        virtual ~Item() {}
        
        /**
         * 
         * @return 
         */
        virtual QString title() const = 0;
        
        /**
         * returns a link to the (web) resource described by this item. In most 
         * cases, this will be a website containing the full article associated
         * with this item.
         */
        virtual QString link() const = 0;
        
        /**
         * returns the description of the item. The description can either be 
         * a tag line, a short summary of the item content up to a complete 
         * article. If no description is provided by the feed, this method 
         * returns the content() string.
         */
        virtual QString description() const = 0;
        
        /**
         * returns the content of the item.
         *  
         * TODO: specify format (HTML)
         * @return content, or QString::null if not set
         */
        virtual QString content() const = 0;
        
        /**
         * returns the date when the item was published. If no publication date
         * is provided by the feed, the current date (when parsed) is inserted, 
         * maintaining the order of the items in the feed by subtracting n
         * seconds from the n-th item in the list.
         * 
         * @return
         */
        virtual time_t datePublished() const = 0;
        
        /**
         * returns the date when the item was modified the last time. If no such 
         * date is provided by the feed, this method returns the value of 
         * datePublished().
         *
         * @return
         */
        virtual time_t dateUpdated() const = 0;
        
        /**
         * returns an ID that identifies the item within its feed. The ID must
         * be unique within its feed. If no ID is provided by the feed source,
         * a hash from title, description and content is returned.
         */
        virtual QString id() const = 0;

        virtual QString author() const = 0;
        
        virtual QString language() const = 0;
        
        
        /**
         * returns an enclosure describing a file available on the net.
         * (mostly used for audio files, so-called "Podcasts")
         * 
         * @return an enclosure object associated with this item, or a null enclosure
         * if not specified
         */
        //Enclosure enclosure() const = 0;
};
    
} // namespace LibSyndication

#endif // LIBSYNDICATION_ITEM_H
