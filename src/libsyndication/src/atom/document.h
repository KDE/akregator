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

#ifndef LIBSYNDICATION_ATOM_DOCUMENT_H
#define LIBSYNDICATION_ATOM_DOCUMENT_H

#include "../abstractdocument.h"
#include "../elementwrapper.h"

#include <ctime>

class QDomDocument;
template <class T> class QList;

namespace LibSyndication {

class DocumentVisitor;

namespace Atom {

class Category;
class Entry;
class EntryDocument;
class FeedDocument;
class Generator;
class Link;
class Person;
typedef SharedPtr<EntryDocument> EntryDocumentPtr;
typedef SharedPtr<FeedDocument> FeedDocumentPtr;

/**
 * An Atom 1.0 Feed Document, containing metadata describing the 
 * feed and a number of entries.
 * 
 * @author Frank Osterfeld
 */
class KDE_EXPORT FeedDocument : public LibSyndication::SpecificDocument,
public ElementWrapper
{
    public:
    
        /**
         * default constructor, creates a null feed, which
         * is invalid.
         * @see isValid()
         */
        FeedDocument();
        
        /**
         * creates a FeedDocument wrapping an atom:feed element.
         * @param element a DOM element, should be a atom:feed document (although not enforced), otherwise this object
         * will not parse anything useful
         */
        FeedDocument(const QDomElement& element);
    
        /**
         * Used by visitors for double dispatch. See DocumentVisitor
         * for more information.
         * @param visitor the visitor calling the method
         */
        bool accept(DocumentVisitor* visitor);
    
        /**
         * a list of persons who are the authors of this feed.
         * According to the Atom 1.0 spec, a feed must have an
         * author unless all entries in it have one.
         */
        QList<Person> authors() const;
        
        /**
         * a list of persons who contribute to this feed. (optional)
         */
        QList<Person> contributors() const;
        
        /**
         * a list of categories this feed is assigned to (optional)
         */
        QList<Category> categories() const;
        
        /**
         * URL of an image serving as a feed icon (optional)
         * 
         * @return icon URL, or a null string if not specified in the feed.
         */
        QString icon() const;
            
        /**
         * URL of an image serving as a feed logo (optional)
         * 
         * @return image URL, or a null string if not specified in the feed.
         */
        QString logo() const;
        
        /** 
         * a string that unambigously identifies the feed (required)
         * 
         * @return the ID of the feed. As defined in the Atom spec it must be
         * a valid URI (which is neither checked nor enforced by this parser)
         * 
         */
        QString id() const;
            
        /**
         * copyright information (optional)
         * 
         * @return copyright information for the feed (intended for human
         * readers), or a null string if not specified
         */
        QString rights() const;
        
        /**
         * feed title (required).
         * 
         * @return title string as HTML.
         */
        QString title() const;
            
        /**
         * description or subtitle of the feed (optional).
         * 
         * @return subtitle string as HTML, or a null string
         * if not specified in the feed.
         */
        QString subtitle() const;
        
        /** 
         * description of the agent used to generate the feed. See
         * Generator for more information (optional).
         * 
         * @return description of the generator, or a null Generator object 
         * if not specified in the feed.
         */
        Generator generator() const;
        
        /**
         * The datetime of the last modification of the feed content.
         * 
         * @return the modification date in seconds since epoch
         */
        time_t updated() const;
        
        /**
         * a list of links. See Link for more information on
         * link types.
         */
        QList<Link> links() const;
            
        /**
         * a list of the entries (items) in this feed.
         */
        QList<Entry> entries() const;
    
        /**
         * returns a description of this feed document for debugging 
         * purposes.
         * 
         * @return debug string
         */
        QString debugInfo() const;
        
        /**
         * returns whether this document is valid or not.
         * Invalid documents do not contain any useful
         * information.
         */
        bool isValid() const;
};

/**
 * An Atom 1.0 Entry Document, containing a single Atom entry outside
 * of the context of a feed.
 * 
 * @author Frank Osterfeld
 */
class KDE_EXPORT EntryDocument : public LibSyndication::SpecificDocument, public LibSyndication::ElementWrapper
{
    public:

        /**
         * default constructor, creates a null document, which is invalid.
         * @see isValid()
         */
        EntryDocument();
        
        /**
         * creates an Atom Entry Document wrapping an atom:entry element.
         * @param element a DOM element, should be a atom:entry element 
         * (although not enforced), otherwise this object will not parse 
         * anything useful
         */
        EntryDocument(const QDomElement& element);
    
        /**
         * Used by visitors for double dispatch. See DocumentVisitor
         * for more information.
         * @param visitor the visitor calling the method
         */
        bool accept(DocumentVisitor* visitor);
    
        /**
         * returns the single entry described in the source.
         * 
         * @return the entry
         */
        Entry entry() const;
    
        /**
         * returns a description of this entry document for debugging 
         * purposes.
         * 
         * @return debug string
         */
        QString debugInfo() const;
        
        /**
         * returns whether this document is valid or not.
         * Invalid documents do not contain any useful
         * information.
         */
        bool isValid() const;
};

} // namespace Atom
} // namespace LibSyndication

#endif // LIBSYNDICATION_ATOM_DOCUMENT_H
