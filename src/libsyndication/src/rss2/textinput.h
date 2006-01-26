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
#ifndef LIBSYNDICATION_RSS2_TEXTINPUT_H
#define LIBSYNDICATION_RSS2_TEXTINPUT_H

#include "../elementwrapper.h"

class QString;
class QDomElement;

namespace LibSyndication {
namespace RSS2 {

/**
 * "The purpose of the <textInput> element is something of a mystery.
 * You can use it to specify a search engine box. Or to allow a
 * reader to provide feedback. Most aggregators ignore it."
 *
 * @author Frank Osterfeld
 */
class TextInput : public ElementWrapper
{
    public:

        /**
         * Default constructor, creates a null object, for which isNull() is
         * @c true.
         */
        TextInput();

        /**
         * Creates a TextInput object wrapping a @c <textInput> XML element.
         *
         * @param e The @c <textInput> element to wrap
         */
        TextInput(const QDomElement& element);


        /**
         * The label of the Submit button in the text input area.
         *
         * @return TODO
         */
        QString title() const;
    
        /**
         * The name of the text object in the text input area.
         *
         * @return TODO
         */
        QString name() const;
    
        /**
         * Explains the text input area.
         *
         * @return TODO
         */
        QString description() const;
    
        /**
         * The URL of the CGI script that processes text input requests.
         *
         * @return TODO
         */
        QString link() const;
    
    
        /**
         * Returns a description of the object for debugging purposes.
         *
         * @return debug string
         */
        QString debugInfo() const;
};

} // namespace RSS2
} // namespace LibSyndication

#endif // LIBSYNDICATION_RSS2_TEXTINPUT_H
