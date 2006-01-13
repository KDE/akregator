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

#ifndef LIBSYNDICATION_RSS2_IMAGE_H
#define LIBSYNDICATION_RSS2_IMAGE_H

#include "../elementwrapper.h"

class QDomElement;
class QString;

namespace LibSyndication {
namespace RSS2 {

class Image : public ElementWrapper
{
    public:

        /**
         * Parses an image object from an <image> XML element.
         *
         * @param e The <image> element to parse the image from
         * @return the image parsed from XML, or a null object
         *         if parsing failed.
         */
        static Image fromXML(const QDomElement& e);

        /**
         * Default constructor, creates a null object, for which isNull() is
         * @c true.
         */
        Image();

        /**
         * the URL of a GIF, JPEG or PNG image 
         *
         * @return TODO
         */
        QString url() const;
    
        /** 
         * Describes the image, can be used in the ALT attribute of the 
         * HTML <img> tag when the channel is rendered in HTML.
         *
         * @return TODO
         */
        QString title() const;
    
        /**
         * The URL of the site, when the channel is rendered, the image should
         * be a link to the site.
         *
         * @return TODO
         */
        QString link() const;
    
        /**
         * The width of the image. If the feed itself doesn't specify a width,
         * this method returns 88, the default value.
         *
         * @return image width in pixels.
         */
        int width() const;
    
        /**
         * The height of the image. If the feed itself doesn't specify a height,
         * this method returns 31, the default value.
         *
         * @return image height in pixels.
         */
        int height() const;
    
        /**
         * optional text that can be included in the TITLE attribute of the link
         * formed around the image in HTML rendering.
         *
         * @return TODO
         */
        QString description() const;
    
        /**
         * Returns a description of the object for debugging purposes.
         *
         * @return debug string
         */
        QString debugInfo() const;

    private:
    
        Image(const QDomElement& el);
};

} // namespace RSS2
}  // namespace LibSyndication

#endif // LIBSYNDICATION_RSS2_IMAGE_H
