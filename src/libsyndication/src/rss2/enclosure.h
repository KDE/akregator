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
        QDomElement toXML(QDomDocument document) const;

        /** creates an null object (equivalent to @ref Enclosure::null()) */
        Enclosure();

        Enclosure(const Enclosure& other);
        Enclosure(const QString& url, int length, const QString& type);
        virtual ~Enclosure();

        Enclosure& operator=(const Enclosure& other);
        bool operator==(const Enclosure& other) const;

        /** returns whether the object is a null object */
        bool isNull() const;

        /** the location of the enclosure. (REQUIRED)
            @param url the URL of the enclosure, must start with "http://". Entities must be resolved. */
        void setURL(const QString& url);

        /** returns the URL of the enclosure */
        QString url() const;

        /** the size of the linked file in bytes (REQUIRED) */
        void setLength(int length);

        /** returns the size of the enclosure in bytes */
        int length() const;

        /** sets the mime type of the enclosure (e.g. "audio/mpeg") (REQUIRED)*/
        void setType(const QString& type);

        /** returns the mime type of the enclosure */
        QString type() const;

        QString debugInfo() const;

        private:

        static Enclosure* m_null;
        class EnclosurePrivate;
        EnclosurePrivate* d;
    };

} // namespace RSS
} // namespace LibSyndication

#endif // LIBSYNDICATION_RSS2_ENCLOSURE_H
