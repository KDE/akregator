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

#ifndef LIBSYNDICATION_IMAGE_H
#define LIBSYNDICATION_IMAGE_H

#include <ksharedptr.h>

namespace LibSyndication {

class Image;
typedef KSharedPtr<Image> ImagePtr;

/**
 * This class represents an image file on the web.
 * It is usually some kind of feed logo which can be displayed when showing the
 * feed description.
 *
 * @author Frank Osterfeld
 */
class KDE_EXPORT Image : public KShared
{
    public:

        /** 
         * destructor
         */
        virtual ~Image() {}
        
        /**
         * returns whether this image is a null object.
         */
        virtual bool isNull() const = 0;
        
        /**
         * the URL of a GIF, JPEG or PNG image 
         */
        virtual QString url() const = 0;
        
        /** 
         * Describes the image, can be used in the ALT attribute of the 
         * HTML &lt;img> tag when the channel is rendered in HTML.
         *
         * @return TODO: specify format
         */
        virtual QString title() const = 0;
        
            
        /**
         * The URL of the site, when the channel is rendered, the image should
         * be a link to the site. If not set, use Feed::link().
         *
         * @return the url the rendered image should link to, or QString::null
         * if not specified in the feed.
         */
        virtual QString link() const = 0;
        
        /**
         * optional text that can be included in the TITLE attribute of the link
         * formed around the image in HTML rendering.
         *
         * @return TODO: specify format (HTML etc.)
         */
        virtual QString description() const = 0;
        
        /**
         * The width of the image in pixels.
         *
         * @return image width in pixels or 0 if not specified in the feed.
         */
        virtual uint width() const = 0;
        
        
    
        /**
         * The height of the image in pixels
         *
         * @return image height in pixels or 0 of not specified in the feed.
         */ 
        virtual uint height() const = 0;
        
        /**
         * returns a description of the image for debugging purposes
         * 
         * @return debug string
         */
        virtual QString debugInfo() const;
};
    
} // namespace LibSyndication

#endif // LIBSYNDICATION_IMAGE_H
