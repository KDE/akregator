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
 * You must have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */
#ifndef LIBSYNDICATION_SPECIFICITEMVISITOR_H
#define LIBSYNDICATION_SPECIFICITEMVISITOR_H

#include <kdepimmacros.h>

namespace LibSyndication {

class AbstractDocument;

namespace Atom
{
    class Entry;
}

namespace RDF
{
    class Item;
}

namespace RSS2 
{
    class Item;
}

class KDE_EXPORT SpecificItemVisitor
{
    public:

        virtual ~SpecificItemVisitor();

        virtual bool visit(SpecificItem* item);
        
        virtual bool visitSpecificRSS2Item(LibSyndication::RSS2::Item* item);
        
        virtual bool visitSpecificRDFItem(LibSyndication::RDF::Item* item);
        
        virtual bool visitSpecificAtomItem(LibSyndication::Atom::Entry* item);
};

} // namespace LibSyndication

#endif // LIBSYNDICATION_SPECIFICITEMVISITOR_H
