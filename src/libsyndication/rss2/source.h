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

#ifndef LIBSYNDICATION_RSS2_SOURCE_H
#define LIBSYNDICATION_RSS2_SOURCE_H

#include <elementwrapper.h>

class QDomElement;
class QString;

namespace LibSyndication {
namespace RSS2 {

/**
 * Class representing the @c &lt;source&gt; element in RSS2. 
 * "The purpose of this element is to propogate credit for links, to publicize
 * the sources of news items. It's used in the post command in the Radio
 * UserLand aggregator. It should be generated automatically when forwarding an
 * item from an aggregator to a weblog authoring tool."
 *
 * @author Frank Osterfeld
 */
class KDE_EXPORT Source : public ElementWrapper
{

    public:

        /**
         * Default constructor, creates a null object, for which isNull() is
         * @c true.
         */
        Source();
        
        /**
         * Creates a source object wrapping a @c &lt;source&gt; XML element.
         *
         * @param element The @c &lt;source&gt; element to wrap
         */
        Source(const QDomElement& element);
        
        /** 
         * The name of the RSS channel that the item came from, derived from
         * its &lt;title>. Example: "Tomalak's Realm"
         *
         * @return A string containing the source, or a null string if not set
         * (and for null objects)
         */
        QString source() const;
    
        /**
         * Required attribute, links to the XMLization of the source.
         *
         * @return A URL, or a null string if not set (though required from
         * the spec), and for null objects
         */
        QString url() const;
    
    
        /**
         * Returns a description of the object for debugging purposes.
         *
         * @return debug string
         */
        QString debugInfo() const;
};

} // namespace RSS2
} // namespace LibSyndication

#endif // LIBSYNDICATION_RSS2_SOURCE_H
