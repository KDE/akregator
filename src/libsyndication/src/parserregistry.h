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

#ifndef LIBSYNDICATION_PARSERREGISTRY_H
#define LIBSYNDICATION_PARSERREGISTRY_H

#include <QString>

namespace LibSyndication {

class AbstractParser;
class DocumentSource;

/**
 * Singleton that collects all the parsers for the various formats.
 * All parsers should be registered here.
 * To parse a feed source, pass it to the parse() method of this class.
 *
 * @author Frank Osterfeld
 */
class ParserRegistry
{
    public:

        /**
         * Singleton instance of ParserRegistry. Register your parsers here 
         */
        static ParserRegistry* self();

        ParserRegistry();
        virtual ~ParserRegistry();

        /**
         * Registers a parser at the registry. Parser::format() must be unique
         * in the registry. If there is already a parser with the same format
         * string, the registration fails.
         *
         * @param parser The parser to be registered
         * @return whether the parser was successfully registered or not.
         */ 
        bool registerParser(AbstractParser* parser);

        /**
         * Removes a parser from the registry.
         *
         * @param parser The parser to be unregistered
         */ 
        void unregisterParser(AbstractParser* parser);

        /**
         * tries to parse a given source with the parsers registered.
         * The source is passed to the first parser that accepts it.
         * 
         * @param source The source to be parsed
         * @param formatHint An optional hint which parser to test first. If
         * there is a parser with the given hint as format string (e.g., 
         * "rss2", "atom", "rdf"...), it is asked first to accept the source.
         * This can avoid unnecessary Parser::accept() checks and speed up
         * parsing. See also Parser::format().
         * @return The document parsed from the source, or NULL if no parser
         *         accepted the source.
         */
        Document* parse(const DocumentSource& source, const QString& formatHint=QString::null);

    private:

        static ParserRegistry* m_self;

        class ParserRegistryPrivate;
        ParserRegistryPrivate* d;
};

} // namespace LibSyndication

#endif // LIBSYNDICATION_PARSERREGISTRY_H
