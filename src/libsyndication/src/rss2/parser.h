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

#ifndef LIBSYNDICATION_RSS2_PARSER_H
#define LIBSYNDICATION_RSS2_PARSER_H

#include "document.h"

#include <abstractparser.h>

namespace LibSyndication {

class DocumentSource;

namespace RSS2 {

class Document;

/**
 * Parser implementation for the RSS 0.9x/2.0 format family
 * 
 * @author Frank Osterfeld
 */
class KDE_EXPORT Parser : public LibSyndication::AbstractParser
{
    public:

        /** default constructor */
        Parser();
        
        /** destructor */
        virtual ~Parser();
        
        /**
         * checks whether a document source looks like an RSS0.9x/2.0 document
         * 
         * @param source a document source to check
         * @return @c true if the source looks like an RSS2 document
         */
        bool accept(const DocumentSource& source) const;

        /**
         * creates an RSS2 wrapper for a document source.
         * The wrapper will only return useful values if the source is really
         * RSS 0.9x/2.0, so call accept() before to check.
         * 
         * @param source the source to wrap
         * @return A LibSyndication::RSS2::Document instance wrapping the XML
         * source, or a null document (not a null pointer!) if there is no @c
         * &lt;channel> root element in the source.
         */
        LibSyndication::SpecificDocumentPtr parse(const DocumentSource& source) const;

        /**
         * returns the format string of this parser implementation, which is
         * "rss2".
         *
         * @return @c "rss2"
         */
        QString format() const;
        
    protected:
        
        /** @internal */
        Parser(const Parser& other);
        
        /** @internal */
        Parser& operator=(const Parser& other);
                
    private:
        class ParserPrivate;
        ParserPrivate* d;
};

} // namespace RSS2
} // namespace LibSyndication

#endif // LIBSYNDICATION_RSS2_PARSER_H
