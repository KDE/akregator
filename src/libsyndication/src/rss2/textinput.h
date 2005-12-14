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

#include <ksharedptr.h>

class QString;
class QDomDocument;
class QDomElement;

namespace LibSyndication {
namespace RSS2 {

/**
 * "The purpose of the <textInput> element is something of a mystery.
 * You can use it to specify a search engine box. Or to allow a
 * reader to provide feedback. Most aggregators ignore it."
 *
 */
class TextInput
{
    public:

        /**
         * static null object. See also TextInput() and isNull().
         *
         * @return reference to a static null object
         */
        static const TextInput& null();

        /**
         * Parses an textInput object from an <textInput> XML element.
         *
         * @param e The <textInput> element to parse the TextInput from
         * @return the TextInput parsed from XML, or a null object
         *         if parsing failed.
         */
        static TextInput fromXML(const QDomElement& e);

        /**
         * Default constructor, creates a null object, which is equal
         * to TextInput::null() and for which isNull() is @c true.
         */
        TextInput();

        /**
         * Copy constructor, creates a copy of @c other.
         * The d pointer is shared, so this is a cheap operation.
         *
         * @param other the object to be copied
         */
        TextInput(const TextInput& other);

        /**
         * Destructor.
         */
        virtual ~TextInput();


        /**
         * Assigns the values of @c other. The d pointer is shared, so
         * this is a cheap operation.
         *
         * @param other The object to assign
         * @return a reference to this object
         */
        TextInput& operator=(const TextInput& other);

        /**
         * Checks whether this TextInput is equal to another.
         * TextInputs are equal if all properties are equal.
         *
         * @param other another TextInput
         * @return whether this object is equal to @c other or not
         */
        bool operator==(const TextInput& other) const;

        /**
         * returns whether this object is a null object.
         *
         * @return @c true, if this is a null object, otherwise @c false
         */
        bool isNull() const;
 
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

    private:

        TextInput(const QString& title, const QString& description, const QString& link, const QString& name);
    
        static TextInput* m_null;
        class TextInputPrivate;
        KSharedPtr<TextInputPrivate> d;
};

} // namespace RSS2
} // namespace LibSyndication

#endif // LIBSYNDICATION_RSS2_TEXTINPUT_H
