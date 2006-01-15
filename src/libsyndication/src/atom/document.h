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
typedef KSharedPtr<EntryDocument> EntryDocumentPtr;
typedef KSharedPtr<FeedDocument> FeedDocumentPtr;


class FeedDocument : public LibSyndication::AbstractDocument, public ElementWrapper
{
    public:
    
        FeedDocument();
        FeedDocument(const QDomElement& element);
    
        bool accept(DocumentVisitor* visitor);
    
        /* 1..* v each entry has >=1 author element */
        
        QList<Person> authors() const;
        
        /* 0..* */
        QList<Person> contributors() const;
        
        /* 0..* */
        QList<Category> categories() const;
        
        /* 0..1 */
        QString icon() const;
            
        /* 0..1 */
        QString logo() const;
        
        QString id() const;
            
        /* 0..1 */
        QString rights() const;
        
        /* 1 */
        QString title() const;
            
        /* 0..1 */
        QString subtitle() const;
        
        /* 0..1 */
        Generator generator() const;
        
        time_t updated() const;
            
        QList<Link> links() const;
            
        QList<Entry> entries() const;
    
        QString debugInfo() const;
};

class EntryDocument : public LibSyndication::AbstractDocument, public ElementWrapper
{
    public:

        EntryDocument();
        EntryDocument(const QDomElement& element);
    
        bool accept(DocumentVisitor* visitor);
    
        Entry entry() const;
    
        QString debugInfo() const;
};

} // namespace Atom
} // namespace LibSyndication

#endif // LIBSYNDICATION_ATOM_DOCUMENT_H
