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
#ifndef LIBSYNDICATION_RDF_TEXTINPUT_H
#define LIBSYNDICATION_RDF_TEXTINPUT_H

#include "resourcewrapper.h"

namespace LibSyndication {
namespace RDF {

/**
 * "The textinput element affords a method for submitting form data to an
 * arbitrary URL - usually located at the parent website.
 * The field is typically used as a search box or subscription form"
 * 
 * @author Frank Osterfeld
 */
class KDE_EXPORT TextInput : public ResourceWrapper
{
    
    public:
        
        /**
         * creates an object wrapping a null resource. 
         * isNull() is @c true.
         */
        TextInput();
        
        /**
         * creates a text input object wrapping a rss:textinput resource
         * 
         * @param resource resource to wrap, must be of type
         * rss:textinput, otherwise this object will not return
         * useful information.
         */
        TextInput(ResourcePtr resource);
         
        
        /**
         * virtual destructor
         */
        virtual ~TextInput();
        
        /**
         * A descriptive title for the textinput field. For example: "Subscribe"
         * or "Search!"
         * 
         * @return title of the text input, or a null string if not specified
         */
        QString title() const;
        
        /**
         * A brief description of the textinput field's purpose. For example:
         * "Subscribe to our newsletter for..." or "Search our site's archive 
         * of..."
         *
         * 
         * @return description TODO: specify format (HTML/plain text), or
         * a null string if not specified. 
         */
        QString description() const;
        
        /**
         * The URL to which a textinput submission will be directed (using GET).
         * 
         * @return URL, or a null string if not specified
         */
        QString link() const;
        
        /**
         * The text input field's (variable) name.
         * 
         * @return name, or a null string if not specified
         */
        QString name() const;
        
        /**
        * Returns a description of the text input for debugging purposes.
        *
        * @return debug string
        */
        QString debugInfo() const;
};

} // namespace RDF
} // namespace LibSyndication

#endif //  LIBSYNDICATION_RDF_TEXTINPUT_H
