/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef LIBSYNDICATION_RSS2_CATEGORY_H
#define LIBSYNDICATION_RSS2_CATEGORY_H

#include <ksharedptr.h>

class QDomDocument;
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
class Category
{
    public:

        /**
         * static null object. See also Category() and isNull().
         *
         * @return reference to a static null object
         */
        static const Category& null();

        /**
         * Parses a category from an <category> XML element.
         *
         * @param e The <category> element to parse the category from
         * @return the category parsed from XML, or a null object
         *         if parsing failed.
         */
        static Category fromXML(const QDomElement& e);


        /**
         * Default constructor, creates a null object, which is equal
         * to Category::null() and for which isNull() is @c true.
         */
        Category();

        /**
         * Copy constructor, creates a copy of @c other.
         * The d pointer is shared, so this is a cheap operation.
         *
         * @param other the object to be copied
         */
        Category(const Category& other);

        /**
         * Destructor.
         */
        virtual ~Category();

        /**
         * Assigns the values of @c other. The d pointer is shared, so
         * this is a cheap operation.
         *
         * @param other The object to assign
         * @return a reference to this object
         */
        Category& operator=(const Category& other);

        /**
         * Checks whether this category is equal to another.
         * Categories are equal if category() and domain() are equal.
         *
         * @param other another category
         * @return whether this object is equal to @c other or not
         */
        bool operator==(const Category& other) const;

        /**
         * Name of the category. This is both to be used as identifier and as
         * human-readable string. It can bea forward-slash-separated string 
         * to identify a hierarchic location in the domain indicated by
         * domain(). Examples: "General", "Programming", "Funny",
         * "Books/History".
         *
         * @return The category identifier/name as string
         * 
         */
        QString category() const;

        /**
         * optional, identifies the domain of the category, i.e. a
         * categorization taxonomy.
         *
         * @return The domain of the category, or QString::null if none is set
         */
        QString domain() const;

        /**
         * returns whether this object is a null object. (For a null object,
         * domain() and category() return both QString::null)
         *
         * @return @c true, if this is a null object, otherwise @c false
         */
        bool isNull() const;

        /**
         * Returns a description of the object for debugging purposes.
         *
         * @return debug string
         */
        QString debugInfo() const;

    private:

        Category(const QString& category, const QString& domain);

        static Category* m_null;

        class CategoryPrivate;
        KSharedPtr<CategoryPrivate> d;

};

} // namespace RSS2
} // namespace LibSyndication

#endif // LIBSYNDICATION_RSS2_CATEGORY_H
