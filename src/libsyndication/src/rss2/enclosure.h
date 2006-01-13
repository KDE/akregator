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

#ifndef LIBSYNDICATION_RSS2_ENCLOSURE_H
#define LIBSYNDICATION_RSS2_ENCLOSURE_H

#include "elementwrapper.h"

class QDomElement;
class QString;

namespace LibSyndication {
namespace RSS2 {

/**
 * Describes a media object that is "attached" to the item.
 * The most common use case for enclosures are podcasts:
 * An audio file is attached to the feed that can be
 * automatically downloaded by a podcast client.
 * 
 * @author Frank Osterfeld
 */
class Enclosure : public ElementWrapper
{
    public:

        /**
         * Parses an enclosure object from an <enclosure> XML element.
         *
         * @param e The <enclosure> element to parse the enclosure from
         * @return the enclosure parsed from XML, or a null object
         *         if parsing failed.
         */
        static Enclosure fromXML(const QDomElement& e);

        /**
         * Default constructor, creates a null object, for which isNull() is 
         * @c true.
         */
        Enclosure();

       
        /** 
         * returns the URL of the enclosure 
         */
        QString url() const;
    
        /**
         * returns the size of the enclosure in bytes
         */
        int length() const;
    
        /**
         * returns the mime type of the enclosure
         * (e.g. "audio/mpeg") 
         */
        QString type() const;
    
        /**
         * Returns a description of the object for debugging purposes.
         *
         * @return debug string
         */
        QString debugInfo() const;

    private:

        Enclosure(const QDomElement& element);
};

} // namespace RSS2
} // namespace LibSyndication

#endif // LIBSYNDICATION_RSS2_ENCLOSURE_H
