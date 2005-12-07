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
        static const TextInput& null();

        TextInput();
        TextInput(const TextInput& other);
        virtual ~TextInput();

        static TextInput fromXML(const QDomElement& e);

        TextInput& operator=(const TextInput& other);
        bool operator==(const TextInput& other) const;

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

        TextInput(const QString& title, const QString& description, const QString&  link, const QString& name);

        static TextInput* m_null;
        class TextInputPrivate;
        KSharedPtr<TextInputPrivate> d;
    };

} // namespace RSS2
} // namespace LibSyndication

#endif // LIBSYNDICATION_RSS2_TEXTINPUT_H
