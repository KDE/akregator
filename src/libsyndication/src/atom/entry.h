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

#ifndef LIBSYNDICATION_ATOM_ENTRY_H
#define LIBSYNDICATION_ATOM_ENTRY_H

#include "../elementwrapper.h"

#include <ctime>

class QDomElement;
class QString;
template <class T> class QList;

namespace LibSyndication {
namespace Atom {

class Category;
class Content;
class Link;
class Person;
class Source;

class Entry : public ElementWrapper
{
    public:
    
        Entry();
        Entry(const QDomElement& element);
    
        /* 1..0, or atom:source with an author in it, or a feed with author */
        QList<Person> authors() const;
        
        /* 0..* */
        QList<Category> categories() const;
    
        /* 0..* */
        QList<Person> contributors() const;
    
        /* 1 */
        QString id() const;
    
        /* at least one link with rel=alternate */
        QList<Link> links() const;
    
        /* 0..1*/
        QString rights() const;
    
        /* 0..1*/
        Source source() const;
    
        /* 0..1*/
        time_t published() const;
    
        /* 1 */
        time_t updated() const;
    
        /* 0..1
        mandatory when 
            - atom:content with src attr. (thus empty)
            - atom:content is not XML but Base64 encoded      
        */
        QString summary() const;
    
        /* 1 */
        QString title() const;
        
        /* 0..1 */
        Content content() const;
    
        QString debugInfo() const;
};

} // namespace Atom
} // namespace LibSyndication

#endif // LIBSYNDICATION_ATOM_ENTRY_H
