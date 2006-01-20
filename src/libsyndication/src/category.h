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

#ifndef LIBSYNDICATION_CATEGORY_H
#define LIBSYNDICATION_CATEGORY_H

#include <ksharedptr.h>

namespace LibSyndication {

class Category;
typedef KSharedPtr<Category> CategoryPtr;

/**
 * A category for categorizing items or whole feeds.
 * A category can be an informal string set by the feed author
 * ("General", "Stuff I like"),
 * a tag assigned by readers, as known from flickr.com  
 * or de.licio.us ("KDE", "funny"),
 * or a term from a formally defined ontology.
 * 
 * To represent the category in a user interface, use label()
 * (or term() as fallback).
 * To create a key for e.g. storage purposes, use scheme() + term().
 * 
 * @author Frank Osterfeld
 */
class Category : public KShared
{
    public:

        /** 
         * destructor
         */
        virtual ~Category() {}
        
        /**
         * returns whether this object is a null category
         */
        virtual bool isNull() const = 0;
        

        /**
         * A term identifying the category, e.g. "general", "life", "books"
         * or "Basketball & other sport I like".
         * The term must be unique in its scheme (see scheme()).
         * 
         * In user interfaces, use it only if there is no label() available.
         * 
         * @return  category term. This string is never empty.
         */
        virtual QString term() const = 0;
        
        /**
         * An optional scheme the term is part of. This can be some 
         * vocabulary/ontology such as Dublin Core.
         * Think of it as the term's namespace, grouping a set of categories.
         * When managing categories, scheme() + term() identifies a category
         * unambigously and can be used as key.
         * 
         * @return the scheme this category is part of, or QString::null
         * if not specified
         */
        virtual QString scheme() const = 0;
        
        /**
         * An optional human-readable label of the category. If specified, this
         * string should be used to represent this category in a user interface.
         * If not specified, use term() instead.
         * 
         * @return the label of this category, or QString::null if not specified
         */
        virtual QString label() const = 0;

        /**
         * Description of the category for debugging purposes.
         * 
         * @return debug string
         */
        virtual QString debugInfo() const;
};
    
} // namespace LibSyndication

#endif // LIBSYNDICATION_CATEGORY_H
