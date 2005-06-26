/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef LIBRSS_RSS_ENCLOSURE_H
#define LIBRSS_RSS_ENCLOSURE_H

#include "global.h"

class QDomDocument;
class QDomElement;
class QString;

namespace RSS
{
    class KDE_EXPORT Enclosure
    {
        public:

        static Enclosure fromXML(const QDomElement& e);
        QDomElement toXML(QDomDocument document) const;

        Enclosure();
        Enclosure(const Enclosure& other);
        Enclosure(const QString& url, int length, const QString& type);
        virtual ~Enclosure();
        
        bool isNull() const;

        Enclosure& operator=(const Enclosure& other);
        bool operator==(const Enclosure& other) const;

        /** returns the URL of the enclosure */
        QString url() const;

       /** returns the size of the enclosure in bytes */
        int length() const;

        /** returns the mime type of the enclosure */
        QString type() const;

        private:

        class EnclosurePrivate;
        EnclosurePrivate* d;
    };

} // namespace RSS
#endif // LIBRSS_RSS_ENCLOSURE_H
