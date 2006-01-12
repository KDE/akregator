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

#ifndef LIBSYNDICATION_ABSTRACTPARSER_H
#define LIBSYNDICATION_ABSTRACTPARSER_H

class QString;

namespace LibSyndication {

class Document;
class DocumentSource;

/**
 * Interface for all parsers. The parsers for the various formats must
 * implement this interface and register themselves at the ParserRegistry.
 *
 * @author Frank Osterfeld
 */
class AbstractParser
{
    public:

        virtual ~AbstractParser() {}

        /**
         * Lets the parser check if it can parse the passed source.
         * Parser implementations should do a _quick_ check for the file
         * format (i.e. check for feed format and version number in the root
         * element) to find out if the source is in a supported format. They
         * should _not_ completely parse the document to test for full
         * compliance to the format specification.
         *
         * @param source the document source to be checked
         * @return whether @c source seems to be in a format supported by the
         * parser
         */
        virtual bool accept(const DocumentSource& source) const = 0;

        /**
         * Lets the parser parse a document source. The parser returns a
         * valid document instance if successfull, or an invalid one otherwise.
         * TODO: add isValid() to Document and link it here
         *
         * @param source The document source to be parsed
         * @return a pointer to a newly created document parsed from @c source
         */
        virtual Document* parse(const DocumentSource& source) const = 0;

        /**
         * Returns the name of the format supported by this
         * parser.
         *
         * @return a string like "rss2", "atom" or "rdf"
         */
        virtual QString format() const = 0;
};

} // namespace LibSyndication

#endif // LIBSYNDICATION_ABSTRACTPARSER_H
