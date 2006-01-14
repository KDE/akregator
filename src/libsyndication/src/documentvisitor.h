/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>
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

#ifndef LIBSYNDICATION_DOCUMENTVISITOR_H
#define LIBSYNDICATION_DOCUMENTVISITOR_H

namespace LibSyndication {

class AbstractDocument;

namespace Atom
{
    class EntryDocument;
    class FeedDocument;
}

namespace RDF
{
    class Document;
}

namespace RSS2 
{
    class Document;
}

/**
 * Visitor interface, following the Visitor design pattern. Use this if you
 * want to process documents and the way how to handle the document depends
 * on it's concrete type (e.g. RSS2::Document, RDF::Document...).
 *
 * TODO: insert code example
 *
 * @author Frank Osterfeld
 */
class DocumentVisitor
{
    public:

        virtual ~DocumentVisitor() {}

        virtual bool visit(AbstractDocument* document);

        virtual bool visit(LibSyndication::RSS2::Document* document) { return false; }
        
        virtual bool visit(LibSyndication::RDF::Document* document) { return false; }
        
        virtual bool visit(LibSyndication::Atom::FeedDocument* document) { return false; }
        
        virtual bool visit(LibSyndication::Atom::EntryDocument* document) { return false; }
};

} // namespace LibSyndication

#endif // LIBSYNDICATION_DOCUMENTVISITOR_H
