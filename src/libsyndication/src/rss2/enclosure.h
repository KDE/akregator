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

#ifndef LIBSYNDICATION_RSS2_ENCLOSURE_H
#define LIBSYNDICATION_RSS2_ENCLOSURE_H

#include <ksharedptr.h>

class QDomDocument;
class QDomElement;
class QString;

namespace LibSyndication {
namespace RSS2 {

    class Enclosure
    {
        public:

        static const Enclosure& null();

        static Enclosure fromXML(const QDomElement& e);

        /** creates an null object (equivalent to @ref Enclosure::null()) */
        Enclosure();

        Enclosure(const Enclosure& other);
        virtual ~Enclosure();

        Enclosure& operator=(const Enclosure& other);
        bool operator==(const Enclosure& other) const;

        /** returns whether the object is a null object */
        bool isNull() const;

        /** returns the URL of the enclosure */
        QString url() const;

        /** returns the size of the enclosure in bytes */
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

        Enclosure(const QString& url, int length, const QString& type);

        static Enclosure* m_null;
        class EnclosurePrivate;
        KSharedPtr<EnclosurePrivate> d;
    };

} // namespace RSS2
} // namespace LibSyndication

#endif // LIBSYNDICATION_RSS2_ENCLOSURE_H
