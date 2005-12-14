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

#include <ksharedptr.h>

class QDomDocument;
class QDomElement;
class QString;

namespace LibSyndication {
namespace RSS2 {

class Image
{
    public:

        /**
         * static null object. See also Cloud() and isNull().
         *
         * @return reference to a static null object
         */
        static const Image& null();

        /**
         * Parses an image object from an <image> XML element.
         *
         * @param e The <image> element to parse the image from
         * @return the image parsed from XML, or a null object
         *         if parsing failed.
         */
        static Image fromXML(const QDomElement& e);

        /**
         * Default constructor, creates a null object, which is equal
         * to Image::null() and for which isNull() is @c true.
         */
        Image();

        /**
         * Copy constructor, creates a copy of @c other.
         * The d pointer is shared, so this is a cheap operation.
         *
         * @param other the object to be copied
         */
        Image(const Image& other);

        /**
         * Destructor.
         */
        virtual ~Image();

        /**
         * Assigns the values of @c other. The d pointer is shared, so
         * this is a cheap operation.
         *
         * @param other The object to assign
         * @return a reference to this object
         */
        Image& operator=(const Image& other);

        /**
         * Checks whether this image is equal to another.
         * Images are equal if all properties are equal.
         *
         * @param other another image
         * @return whether this object is equal to @c other or not
         */
        bool operator==(const Image& other) const;

        /**
         * returns whether this object is a null object.
         *
         * @return @c true, if this is a null object, otherwise @c false
         */
        bool isNull() const;
    
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
    
        Image(const QString& url, const QString& title, const QString& link, const QString& description, int width, int height);
    
        static Image* m_null;
    
        class ImagePrivate;
        KSharedPtr<ImagePrivate> d;
};

} // namespace RSS2
}  // namespace LibSyndication

#endif // LIBSYNDICATION_RSS2_IMAGE_H
