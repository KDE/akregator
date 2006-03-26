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

#include "specificdocument.h"
#include "abstractparser.h"
#include "documentsource.h"
#include "feed.h"
#include "global.h"
#include "mapper.h"

#include <QDomDocument>
#include <QHash>
#include <QString>

namespace LibSyndication {


/**
 * Collects all the format-specific parser implementations.
 * To parse a feed source, pass it to the parse() method of this class.
 * In most cases, you should use the global singleton
 * LibSyndication::parserCollection().
 * When loading the source from the web, use Loader instead of using
 * this class directly.
 * 
 * Example code:
 * 
 * @code
 * ...
 * QFile someFile(somePath);
 * ... 
 * DocumentSource src(someFile.readAll());
 * someFile.close();
 * 
 * FeedPtr feed = parserCollection()->parse(src);
 * 
 * if (feed)
 * {
 *     QString title = feem_title();
 *     QList<ItemPtr> items = feem_items();
 *     ...
 * }
 * @endcode
 *
 * TODO: explain &lt;T>
 * 
 * @author Frank Osterfeld
 */
template <class T>
class KDE_EXPORT ParserCollection
{
    public:

        /** destructor */
        virtual ~ParserCollection() {}

        /**
         * tries to parse a given source with the parsers registered.
         * The source is passed to the first parser that accepts it.
         * 
         * @param source The source to be parsed
         * @param formatHint An optional hint which parser to test first. If
         * there is a parser with the given hint as format string (e.g., 
         * "rss2", "atom", "rdf"...), it is asked first to accept the source.
         * This can avoid unnecessary AbstractParser::accept() checks and speed
         * up parsing. See also AbstractParser::format().
         * @return The feed document parsed from the source, or NULL if no
         * parser accepted the source.
         */
        virtual SharedPtr<T> parse(const DocumentSource& source,
                            const QString& formatHint=QString()) = 0;

        
        /**
         * returns the error code of the last parse() call.
         * 
         * @return the last error, or Success if parse() was successful
         * or not yet called at all.
         */
        virtual ErrorCode lastError() const = 0;
        
        /**
         * Adds a parser and corresponding mapper to the collection.
         * AbstractParser::format() must be unique
         * in the collection. If there is already a parser with the same format
         * string, the parser isn't added.
         * 
         * @note ownership for both @c parser and @c mapper is taken by the
         * implementation, so don't delete them in your code!
         *
         * @param parser The parser to be registered
         * @param mapper the mapper that should be used for building the
         * abstraction
         * @return whether the parser was successfully registered or not.
         */
        virtual bool registerParser(AbstractParser* parser, Mapper<T>* mapper) = 0;
        
        /**
         * Changes the specific format to abstraction mapping for a parser.
         * 
         * @param format the format string of the parser whose 
         * mapping should be changed. See AbstractParser::format.
         * @param mapper Mapper implementation doing the mapping from the
         * format specific representation to abstraction of type T.
         */
        virtual void changeMapper(const QString& format, Mapper<T>* mapper) = 0;

};

} // namespace LibSyndication

#endif // LIBSYNDICATION_PARSERCOLLECTION_H
