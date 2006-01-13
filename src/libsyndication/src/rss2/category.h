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

#ifndef LIBSYNDICATION_RSS2_CATEGORY_H
#define LIBSYNDICATION_RSS2_CATEGORY_H

#include "../elementwrapper.h"

class QDomElement;
class QString;

namespace LibSyndication {
namespace RSS2 {

/**
 * A category which can be assigned to items or whole feeds.
 * These can be simple tags as known from delicious or Technorati, or
 * a category from a hierarchical taxonomy or ontology.
 *
 * @author Frank Osterfeld
 */
class Category : public ElementWrapper
{
    public:

        /**
         * Parses a category from an <category> XML element.
         *
         * @param e The <category> element to parse the category from
         * @return the category parsed from XML, or a null object
         *         if parsing failed.
         */
        static Category fromXML(const QDomElement& e);


        /**
         * Default constructor, creates a null object, for which isNull() is
         * @c true.
         */
        Category();

        /**
         * Name of the category. This is both to be used as identifier and as
         * human-readable string. It can bea forward-slash-separated string 
         * to identify a hierarchic location in the domain indicated by
         * domain(). Examples: "General", "Programming", "Funny",
         * "Books/History".
         *
         * @return The category identifier/name as string or QString::null for
         * null objects.
         * 
         */
        QString category() const;

        /**
         * optional, identifies the domain of the category, i.e. a
         * categorization taxonomy.
         *
         * @return The domain of the category, or QString::null if none is set (and for null objects)
         */
        QString domain() const;

        /**
         * Returns a description of the object for debugging purposes.
         *
         * @return debug string
         */
        QString debugInfo() const;

    private:

        Category(const QDomElement& element);
};

} // namespace RSS2
} // namespace LibSyndication

#endif // LIBSYNDICATION_RSS2_CATEGORY_H
