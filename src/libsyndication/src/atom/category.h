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

#ifndef LIBSYNDICATION_ATOM_CATEGORY_H
#define LIBSYNDICATION_ATOM_CATEGORY_H

#include "../elementwrapper.h"

class QDomElement;
class QString;

namespace LibSyndication {
namespace Atom {

/**
 * A category for categorizing items or whole feeds.
 * A category can be an informal string set by the feed author ("General",
 * "Stuff I like"), a tag assigned by readers, as known from flickr.com
 * or de.licio.us ("KDE", "funny"), or a term from a formally defined ontology.
 * 
 * To represent the category in a user interface, use label() (or term() as
 * fallback). To create a key for e.g. storage purposes, use scheme() + term().
 * 
 * @author Frank Osterfeld
 */
class KDE_EXPORT Category : public ElementWrapper
{
    public:
        
        /**
         * creates a null category object.
         */
        Category();
        
        /**
         * creates a Category object wrapping an atom:category element.
         * @param element a DOM element, should be a atom:category element
         * (although not enforced), otherwise this object will not parse 
         * anything useful
         */
        Category(const QDomElement& element);
        

        /**
         * a term describing the category. (required)
         * 
         * @return the category term as plain text (no HTML, "&", "<" etc. are
         * unescaped!)
         */
        QString term() const;
    
        /**
         * naming scheme the category term is part of. (optional)
         * A term is unique in its scheme (like in C++ identifiers are 
         * unique in their namespaces)
         * 
         * @return a URI representing the scheme, or a null string
         * if not specified
         */
        QString scheme() const;
    
        /**
         * Label of the category (optional).
         * If specified, this string should be used to represent this category
         * in a user interface.
         * If not specified, use term() instead.
         * 
         * @return the label as plain text (no HTML, "&", "<" etc. are
         * unescaped!), or a null string if not specified
         */
        QString label() const;
    
        /**
         * description of this category object for debugging purposes
         * 
         * @return debug string
         */
        QString debugInfo() const;
};

} // namespace Atom
} // namespace LibSyndication

#endif // LIBSYNDICATION_ATOM_CATEGORY_H
