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

#ifndef LIBSYNDICATION_ENCLOSURE_H
#define LIBSYNDICATION_ENCLOSURE_H

#include "sharedptr.h"

#include <kdepimmacros.h>

namespace LibSyndication {

class Enclosure;
typedef SharedPtr<Enclosure> EnclosurePtr;

/**
 * An enclosure describes a (media) file available on the net.
 *
 * Most of the time, enclosures are used for "podcasts", i.e. audio
 * files announced and distributed via syndication.
 *
 * @author Frank Osterfeld
 */
class KDE_EXPORT Enclosure
{
    public:

        /** 
         * destructor
         */
        virtual ~Enclosure();
        
        /**
         * returns whether this enclosure is a null object.
         */ 
        virtual bool isNull() const = 0;
        
        /**
         * The URL of the linked resource (required).
         *
         * @return 
         */
        virtual QString url() const = 0;
        
        /**
         * title of the enclosure. This is a human-readable description of the
         * linked file. If available, the title should be used in user interfaces
         * instead of the URL. If no title is set (e.g., RSS2 enclosures don't
         * have titles), use url() as fallback.
         * 
         * @return title describing the enclosure, or a null string if not
         * specified.
         */
        virtual QString title() const = 0;
        
        /**
         * mimetype of the enclosure.
         * TODO: link mimetype specs
         * 
         * Examples are @c "audio/mpeg" for MP3, or @c "application/pdf" for
         * PDF.
         * 
         * @return the mimetype of the file, or a null string if not
         * specified
         */
        virtual QString type() const = 0;
        
        /**
         * returns the length of the linked file in bytes
         * 
         * @return the length of the file in bytes, 0 if not specified
         */
        virtual uint length() const = 0;
        
        
        /** 
         * for audio/video files, the duration of the file in seconds
         * 
         * @return the duration of the file in seconds, or 0 if not specified
         */
        virtual uint duration() const = 0;
        
        /**
         * description of this enclosure for debugging purposes
         * 
         * @return debug string
         */
        virtual QString debugInfo() const;
};
    
} // namespace LibSyndication

#endif // LIBSYNDICATION_ENCLOSURE_H
