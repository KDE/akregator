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

#ifndef LIBSYNDICATION_MAPPER_H
#define LIBSYNDICATION_MAPPER_H

template <class T> class SharedPtr;

namespace LibSyndication {

class AbstractDocument;
typedef SharedPtr<AbstractDocument> AbstractDocumentPtr;

/**
 * @brief A mapper maps an AbstractDocument to something else.
 * The type of this "something else" is specified by the template
 * parameter T.
 * In the default implementation it is used with the Feed interface,
 * but its not limited to that. T can be an arbitrary class, but must
 * inherit from KShared (so it can be used with shared pointers).
 * 
 * There are three use cases that require you to implement your own
 * mapper (usually it shouldn't be necessary though).
 * For more information on the possible uses, see TODO: link docs.
 * 
 * 1) Add your own feed parser. In case you need support for another
 * feed format (Okay! News, CDF, totally backward-incompatible Atom 5.0, ... 
 * you name it), you can 
 * implement AbstractParser and AbstractDocument for it and provide a 
 * Mapper&lt;Feed>
 *  
 * * @code
 * class OkayNewsMapper : public Mapper<Feed>
 * {
 *     public:
 * 
 *     virtual FeedPtr map(AbstractDocumentPtr doc) const { ... }
 * };
 * 
 * parserCollection()->registerParser(new OkayNews::Parser, new OkayNewsMapper);
 * @endcode
 * 
 * 2) Implement your own mapper for the Feed abstraction, for an
 * existing parser. E.g. if you think LibSyndication does map Atom
 * all wrong, you can implement your own Atom mapper and use that instead
 * of the default one.
 * 
 * @code
 * class MyAtomMapper : public Mapper<Feed>
 * {
 *     public:
 * 
 *     virtual FeedPtr map(AbstractDocumentPtr doc) const { ... }
 * };
 * 
 * parserCollection()->changeMapper("atom", new MyAtomMapper);
 * @endcode
 * 
 * 3) Use your own abstraction. In case the Feed interface
 * does not fit your needs, you can use your own interface, let's 
 * say "MyFeed". Be aware you have to implement custom mappings for
 * all feed formats then:
 * 
 * @code
 * class MyFeed
 * {
 *     public:
 * 
 *     QString title() const; // my special title
 *     QList<Article> articles() const; // I name it articles
 * };
 * 
 * class MyAtomMapper : public Mapper<MyFeed> { ... };
 * class MyRDFMapper : public Mapper<MyFeed> { ... };
 * class MyRSS2Mapper : public Mapper<MyFeed> { ... };
 * 
 * ParserCollection<MyFeed>* coll = new ParserCollection<MyFeed>;
 * coll->registerParser(new Atom::Parser, new MyAtomMapper);
 * coll->registerParser(new RDF::Parser, new MyRDFMapper);
   coll->registerParser(new RSS2::Parser, new MyRSS2Mapper);
 * @endcode
 * 
 * @author Frank Osterfeld
 */
template <class T>
class Mapper
{
    public:
        
        /**
         * virtual destructor
         */
        virtual ~Mapper() {}
        
        /**
         * maps a format-specific document to abstraction of type 
         * @c T.
         *
         * \note implementations may assume @c doc to have the right
         * type and may just statically cast to the subclass without
         * further checking.
         *(ParserCollection takes care of passing the right subtype)
         * 
         * @param doc the document to map.
         * @return a newly created object implementing the E @c T.
         */
        virtual SharedPtr<T> map(AbstractDocumentPtr doc) const = 0;
};

} // namespace libsyndication

#endif // LIBSYNDICATION_MAPPER_H
