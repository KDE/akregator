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

#ifndef LIBSYNDICATION_RDF_DOCUMENT_H
#define LIBSYNDICATION_RDF_DOCUMENT_H

#include "resourcewrapper.h"

#include "../document.h"

template <class T> class QList;

namespace LibSyndication {
namespace RDF {

class Content;
class DublinCore;
class Image;
class Item;
class Model;
class Syndication;
class TextInput;

class Document : public LibSyndication::Document, public ResourceWrapper
{
    
    public:
            
        Document();
        Document(ResourcePtr resource);
        virtual ~Document();
        
        virtual bool accept(DocumentVisitor* visitor);
    
        QString title() const;
        
        QString description() const;
        
        QString link() const;
        
        /**
         * returns a dublin core description of the document.
         * 
         * @return a DC description describing this document
         */
        DublinCore dc() const;
        
        Syndication syn() const;
        
        QList<Item> items() const;
        
        Image image() const;
        
        TextInput textInput() const;
        
        /**
        * Returns a description of the document for debugging purposes.
        *
        * @return debug string
        */
        virtual QString debugInfo() const;
     
};

} // namespace RDF
} // namespace LibSyndication

#endif //  LIBSYNDICATION_RDF_DOCUMENT_H
