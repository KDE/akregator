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

#ifndef LIBSYNDICATION_ATOM_SOURCE_H
#define LIBSYNDICATION_ATOM_SOURCE_H

#include <elementwrapper.h>

#include <ctime>

class QDomElement;
class QString;

template <class T> class QList;

namespace LibSyndication {
namespace Atom {

class Category;
class Generator;
class Link;
class Person;

/**
 * If an entry was copied from another feed, this class contains
 * a description of the source feed.
 * 
 * @author Frank Osterfeld
 */
class KDE_EXPORT Source : public ElementWrapper
{
    public:

        /**
         * creates a null source object
         */
        Source();
        
        /**
         * creates a Source object wrapping a atom:source element.
         * 
         * @param element a DOM element, should be a atom:source element
         * (although not enforced), otherwise this object will not parse 
         * anything useful
         */
        Source(const QDomElement& element);

        /**
         * authors of the original content (optional)
         */
        QList<Person> authors() const;

        /**
         * contributors to the original content (optional)
         */
        QList<Person> contributors() const;

        /**
         * categories the source feed is assigned to (optional)
         */
        QList<Category> categories() const;

        /**
         * description of the software which generated the source feed
         * (optional)
         */
        Generator generator() const;

        /**
         * URL of an image serving as a feed icon (optional)
         * 
         * @return icon URL, or a null string if not specified
         */
        QString icon() const;

        /**
         * a string that unambigously identifies the source feed (optional)
         *
         * @return the ID of the source feed, or a null string if not
         * specified.
         */
        QString id() const;

        /**
         * a list of links. See Link for more information on
         * link types.
         */
        QList<Link> links() const;

        /**
         * URL of an image, the logo of the source feed (optional)
         * 
         * @return image URL, or a null string if not specified in the feed.
         */
        QString logo() const;

        /**
         * copyright information (optional)
         * 
         * @return copyright information for the source,
         * or a null string if not specified
         */
        QString rights() const;

        /**
         * description or subtitle of the source feed (optional).
         * 
         * @return subtitle string as HTML, or a null string
         * if not specified.
         */
        QString subtitle() const;
        
        /**
         * source feed title (optional).
         * 
         * @return title string as HTML, or a null string if not specified
         */
        QString title() const;

        /**
         * The datetime of the last modification of the source feed
         * content. (optional)
         * 
         * @return the modification date in seconds since epoch
         */
        time_t updated() const;

        /**
         * description of this source object for debugging purposes
         *
         * @return debug string
         */
        QString debugInfo() const;
};

} // namespace Atom
} // namespace LibSyndication

#endif // LIBSYNDICATION_ATOM_SOURCE_H
