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

class QString;
class QDomDocument;
class QDomElement;

namespace LibSyndication {
namespace RSS2 {

    class TextInput
    {
        public:
        static const TextInput& null();

        TextInput();
        TextInput(const TextInput& other);
        virtual ~TextInput();

        static TextInput fromXML(const QDomElement& e);
        QDomElement toXML(QDomDocument document) const;

        TextInput& operator=(const TextInput& other);
        bool operator==(const TextInput& other) const;

        bool isNull() const;

        void setTitle(const QString& title);
        QString title() const;

        void setName(const QString& name);
        QString name() const;

        void setDescription(const QString& description);
        QString description() const;

        void setLink(const QString& link);
        QString link() const;

        QString debugInfo() const;

        private:

        static TextInput* m_null;
        class TextInputPrivate;
        TextInputPrivate* d;
    };

} // namespace RSS2
} // namespace LibSyndication

#endif // LIBSYNDICATION_RSS2_TEXTINPUT_H
