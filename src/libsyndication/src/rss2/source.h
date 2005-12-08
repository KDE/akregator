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

#ifndef LIBSYNDICATION_RSS2_SOURCE_H
#define LIBSYNDICATION_RSS2_SOURCE_H

#include <ksharedptr.h>

class QDomDocument;
class QDomElement;
class QString;

namespace LibSyndication {
namespace RSS2 {

/**
 * Class representing the <source> element in RSS2. 
 * "The purpose of this element is to propogate credit for links, to publicize
 * the sources of news items. It's used in the post command in the Radio
 * UserLand aggregator. It should be generated automatically when forwarding an
 * item from an aggregator to a weblog authoring tool."
 *
 * @author Frank Osterfeld
 */
class Source
{

    public:

        /**
         * static null object. See also Source() and isNull().
         *
         * @return reference to a static null object
         */
        static const Source& null();

        /**
         * Parses an source object from an <source> XML element.
         *
         * @param e The <source> element to parse the source from
         * @return the source parsed from XML, or a null object
         *         if parsing failed.
         */
        static Source fromXML(const QDomElement& e);

        /**
         * Default constructor, creates a null object, which is equal
         * to Source::null() and for which isNull() is @c true.
         */
        Source();

        /**
         * Copy constructor, creates a copy of @c other.
         * The d pointer is shared, so this is a cheap operation.
         *
         * @param other the object to be copied
         */
        Source(const Source& other);

        /**
         * Destructor.
         */
        virtual ~Source();

        /**
         * Assigns the values of @c other. The d pointer is shared, so
         * this is a cheap operation.
         *
         * @param other The object to assign
         * @return a reference to this object
         */
        Source& operator=(const Source& other);

        /**
         * Checks whether this source is equal to another.
         * Sources are equal if all properties are equal.
         *
         * @param other another source
         * @return whether this object is equal to @c other or not
         */
        bool operator==(const Source& other) const;

        /**
         * returns whether this object is a null object.
         *
         * @return @c true, if this is a null object, otherwise @c false
         */
        bool isNull() const;
    
        /** 
         * The name of the RSS channel that the item came from, derived from
         * its <title>. Example: "Tomalak's Realm"
         *
         * @return A string containing the source, or QString::null if not set
         * (and for null objects)
         */
        QString source() const;
    
        /**
         * Required attribute, links to the XMLization of the source.
         *
         * @return An url, or QString::null if not set (though required from
         * the spec), and for null objects
         */
        QString url() const;
    
    
        /**
         * Returns a description of the object for debugging purposes.
         *
         * @return debug string
         */
        QString debugInfo() const;

    private:

        Source(const QString& source, const QString& url);
        
        static Source* m_null;
        
        class SourcePrivate;
        KSharedPtr<SourcePrivate> d;
};

} // namespace RSS2
} // namespace LibSyndication

#endif // LIBSYNDICATION_RSS2_SOURCE_H
