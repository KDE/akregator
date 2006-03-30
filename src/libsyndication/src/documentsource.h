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

#ifndef LIBSYNDICATION_DOCUMENTSOURCE_H
#define LIBSYNDICATION_DOCUMENTSOURCE_H

#include "sharedptr.h"

#include <kdepimmacros.h>

class QByteArray;
class QDomDocument;

namespace LibSyndication {

/** 
 * Represents the source of a syndication document, as read from the
 * downloaded file.
 *
 * It provides a (cached) DOM representation of the document, but keeps
 * the raw data available (for (rarely used) non-XML formats like Okay!
 * News).
 *
 * This way the document can be passed to all available parsers (to find the
 * right one for the source), regardless whether they parse XML formats or
 * non-XML formats, without having every parser to do the XML parsing again.
 *
 * @author Frank Osterfeld
 */
class KDE_EXPORT DocumentSource
{
    public:

        /**
         * Creates an empty document source. The raw representation is empty and 
         * the DOM representation will be invalid.
         */
        DocumentSource();
        
        /**
         * Creates a DocumentSource object from a raw byte array
         * 
         * @param source the raw source (of the downloaded feed file usually)
         * @param url the URL/path the source was read from
         */
        DocumentSource(const QByteArray& source, const QString& url);
        
        /** 
         * Copy constructor. The d pointer is shared, so this is a cheap
         * operation.
         * 
         * @param other DocumentSource to copy
         */
        DocumentSource(const DocumentSource& other);

        /** 
         * destructor 
         */
        ~DocumentSource();

        /**
         * Assignment operator. The d pointer is shared, so this is a cheap
         * operation.
         * 
         * @param other DocumentSource to assign to this instance
         * @return reference to this instance
         */
        DocumentSource& operator=(const DocumentSource& other);

        /**
         * Returns the feed source as byte array.
         *
         * @return the feed source as raw byte array.
         */
        const QByteArray& asByteArray() const;

        /**
         * returns the size the source array in bytes.
         *
         * @return the size of the byte array in bytes. 
         * See also QByteArray::size()
         */
        unsigned int size() const;

        /**
         * calculates a hash value for the source array.
         * This can be used to decide whether the feed has changed since
         * the last fetch. If the hash hasn't changed since the last fetch,
         * the feed wasn't modified with high probability.
         * 
         * @return the hash calculated from the source, 0 if the
         * source is empty
         */
        unsigned int hash() const;

        /**
         * Returns the feed source as DOM document.
         * The document is parsed only on the first call of this method
         * and then cached.
         *
         * @return XML representation parsed from the raw source
         */
        const QDomDocument& asDomDocument() const;

        /**
         * returns the URL the document source was loaded from
         */
        const QString& url() const;
        
    private: 

        class DocumentSourcePrivate;
        SharedPtr<DocumentSourcePrivate> d;
};

} // namespace LibSyndication

#endif // LIBSYNDICATION_DOCUMENTSOURCE_H
