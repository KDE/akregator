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

#ifndef LIBSYNDICATION_PARSERCOLLECTION_H
#define LIBSYNDICATION_PARSERCOLLECTION_H

#include <QString>

template <class T> class KSharedPtr;

namespace LibSyndication {

class AbstractParser;
class DocumentSource;
class Feed;
typedef KSharedPtr<Feed> FeedPtr;

/**
 * Singleton that collects all the format-specific implementations of
 * AbstractParser.
 * To parse a feed source, pass it to the parse() method of this class.
 * 
 * Example code:
 * 
 * @code
 * ...
 * DocumentSource src(someFile.readAll());
 * someFile.close();
 * 
 * FeedPtr feed = ParserCollection::self()->parse(src);
 * 
 * if (feed)
 * {
 *     QString title = feed->title();
 *     QList<ItemPtr> items = feed->items();
 *     ...
  * }
 * @endcode
 * 
 * @author Frank Osterfeld
 */
class ParserCollection
{
    public:

        /**
         * Singleton instance of ParserCollection.
         */
        static ParserCollection* self();

        /** destructor */
        virtual ~ParserCollection();

        /**
         * tries to parse a given source with the parsers registered.
         * The source is passed to the first parser that accepts it.
         * 
         * @param source The source to be parsed
         * @param formatHint An optional hint which parser to test first. If
         * there is a parser with the given hint as format string (e.g., 
         * "rss2", "atom", "rdf"...), it is asked first to accept the source.
         * This can avoid unnecessary AbstractParser::accept() checks and speed up
         * parsing. See also AbstractParser::format().
         * @return The document parsed from the source, or NULL if no parser
         * accepted the source.
         */
        FeedPtr parse(const DocumentSource& source, const QString& formatHint=QString::null);

        /**
         * Adds a parser to the collection. Parser::format() must be unique
         * in the collection. If there is already a parser with the same format
         * string, the parser isn't added.
         *
         * @param parser The parser to be registered
         * @return whether the parser was successfully registered or not.
         */
        bool registerParser(AbstractParser* parser);

    protected:
        
        /** constructor */
        ParserCollection();
        
        
    private:

        ParserCollection(const ParserCollection& other) {}
        ParserCollection& operator=(const ParserCollection& other) {}
        
        static ParserCollection* m_self;

        class ParserCollectionPrivate;
        ParserCollectionPrivate* d;
};

} // namespace LibSyndication

#endif // LIBSYNDICATION_PARSERCOLLECTION_H
