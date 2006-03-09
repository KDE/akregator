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
#ifndef LIBSYNDICATION_RDF_ITEM_H
#define LIBSYNDICATION_RDF_ITEM_H

#include "resourcewrapper.h"

#include "../specificitem.h"

class QString;

namespace LibSyndication {
    
class SpecificItemVisitor;
    
namespace RDF {

class DublinCore;
class Item;
class Model;

/**
 * wrapper for RSS 1.0 Item resources.
 * 
 * @author Frank Osterfeld
 */
class KDE_EXPORT Item : public ResourceWrapper
{
    
    public:
        
        /**
         * creates an item object wrapping a null resource, isNull() is 
         * @c true.
         */
        Item();
        
        /**
         * Creates an item wrapping the given resource
         * @param resource resource to wrap, should be an instance 
         * of rss1:item, otherwise the wrapper will not return useful
         * information.
         */
        Item(ResourcePtr resource);
        
        /**
         * virtual destructor
         */
        virtual ~Item();

        /**
         * The item's title (required).
         * 
         * @return The item's title as HTML, or a null string if not specified
         */
        QString title() const;
        
        /**
         * A brief description/abstract of the item.
         * if encodedContent() is not provided, this can also contain the full
         * content.
         * 
         * @return description as HTML, or a null string if not specified
         */
        QString description() const;
        
        /**
         * The item's URL, usually pointing to a website containing the 
         * full content (news article, blog entry etc.).
         * 
         * @return the link
         */
        QString link() const;
        
        /**
         * returns a dublin core description of this 
         * item (including metadata such as item author
         * or subject)
         */
        DublinCore dc() const;
        
        /**
         * returns content (@c content:encoded) as HTML.
         * 
         * @return content as HTML, or a null string if not specified
         */
        QString encodedContent() const;
        
        /**
         * Returns a description of the item for debugging purposes.
         *
         * @return debug string
         */
        QString debugInfo() const;
};

typedef KSharedPtr<SpecificItem> SpecificItemPtr;
class SpecificItem : public LibSyndication::SpecificItem
{
    public:
        
        SpecificItem();
        SpecificItem(const Item& item);
        virtual ~SpecificItem();
        
        Item item() const;
        
        bool accept(SpecificItemVisitor* visitor);
        
    private:
        
        class SpecificItemPrivate;
        SpecificItemPrivate* d;

};

} // namespace RDF
} // namespace LibSyndication

#endif //  LIBSYNDICATION_RDF_ITEM_H
