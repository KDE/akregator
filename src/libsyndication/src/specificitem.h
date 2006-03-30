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
#ifndef LIBSYNDICATION_SPECIFICITEM_H
#define LIBSYNDICATION_SPECIFICITEM_H

#include <kdepimmacros.h>

#include "sharedptr.h"

class QString;

namespace LibSyndication {

class SpecificItemVisitor;
class SpecificItem;

typedef SharedPtr<SpecificItem> SpecificItemPtr;

/**
 * Interface for all format-specific item-like classes, such as
 * RSS2/RDF items, and Atom entries. To process items based on their
 * format, use a SpecificItemVisitor.
 * 
 * @author Frank Osterfeld
 */
class KDE_EXPORT SpecificItem
{
    public:

        /**
         * virtual dtor
         */
        virtual ~SpecificItem();

        /**
         * This must be implemented for the double dispatch
         * technique (Visitor pattern).
         * 
         * The usual implementation is
         * @code
         * return visitor->visit(this);
         * @endcode
         * 
         * See also SpecificItemVisitor.
         * 
         * @param visitor the visitor "visiting" this object
         */
        virtual bool accept(SpecificItemVisitor* visitor) = 0;

};

} // namespace LibSyndication

#endif // LIBSYNDICATION_SPECIFICITEM_H

