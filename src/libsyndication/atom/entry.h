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

#include <elementwrapper.h>
#include <specificitem.h>

#include <ctime>

class QDomElement;
class QString;
template <class T> class QList;

namespace LibSyndication {

class SpecificItemVisitor;

namespace Atom {

class Category;
class Content;
class Link;
class Person;
class Source;

/**
 * an Atom entry, equivalent to the "items" in the RSS world.
 *
 * @author Frank Osterfeld
 */
class KDE_EXPORT Entry : public ElementWrapper, public SpecificItem
{
    public:
    
        /**
         * creates a null entry object
         */
        Entry();
        
        /**
         * creates an Entry object wrapping an atom:entry element.
         * @param element a DOM element, should be a atom:entry element
         * (although not enforced), otherwise this object will not parse 
         * anything useful
         */
        Entry(const QDomElement& element);
    
        bool accept(SpecificItemVisitor* visitor);
        
        /**
         * list of persons who are authors of this entry.
         * 
         * This is optional if the containing feed has an author description,
         * and required if not.
         */
        QList<Person> authors() const;
        
        /**
         * a list of categories this entry is filed to (optional)
         */
        QList<Category> categories() const;
    
        /**
         * list of persons contributing to this entry (optional)
         */
        QList<Person> contributors() const;
    
        /**
         * ID of the article. (required)
         * The ID must be unique inside this feed. The atom spec defines it as a
         * URI (which is not enforced by this parser)
         */
        QString id() const;
    
        /**
         * links pointing to associated web sites and other resources.
         * 
         * Links are optional if the entry provides Content.
         * Otherwise, it must contain at least one link with
         * a @c rel value of @c "alternate". (see Link).
         */
        QList<Link> links() const;
    
        /**
         * copyright information (optional)
         * 
         * @return copyright information for the entry (intended for human
         * readers), or a null string if not specified
         */
        QString rights() const;
    
        /**
         * source description of the content (optional)
         * 
         * If the content was copied from another feed, this object contains
         * information about the source feed.
         * 
         * @return source description, or a null object if not 
         * specified
         */
        Source source() const;
    
        /**
         * The datetime of the publication of this entry (optional).
         * 
         * @return the publication date in seconds since epoch
         */
        time_t published() const;
    
        /**
         * The datetime of the last modification of this entry (required).
         * 
         * @return the modification date in seconds since epoch
         */
        time_t updated() const;
    
        /**
         * a short summary, abstract or excerpt of an entry. (optional)
         * This is usually more verbose than title() and but does not
         * contain the whole content as content() does.
         * 
         * @return the summary as HTML, or a null string if not specified
         */
        QString summary() const;
    
        /**
         * title of the entry (required).
         * 
         * @return the title as HTML
         */
        QString title() const;
        
        /**
         * content of the entry (optional)
         * See @ref Content for details
         * 
         * @return entry content, or a null content object if not specified
         */
        Content content() const;
    
        /**
         * returns a description of this entry for debugging purposes
         * 
         * @return debug string
         */
        QString debugInfo() const;
};


} // namespace Atom
} // namespace LibSyndication

#endif // LIBSYNDICATION_ATOM_ENTRY_H
