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

#ifndef LIBSYNDICATION_RSS2_CLOUD_H
#define LIBSYNDICATION_RSS2_CLOUD_H

class QDomDocument;
class QDomElement;
class QString;

namespace LibSyndication {
namespace RSS2
{
    class Cloud
    {
        public:

        static const Cloud& null();

        static Cloud fromXML(const QDomElement& e);
        QDomElement toXML(QDomDocument document) const;

        Cloud();
        Cloud(const Cloud& other);
        virtual ~Cloud();

        Cloud& operator=(const Cloud& other);
        bool operator==(const Cloud& other) const;

        void setDomain(const QString& domain);
        QString domain() const;
        void setPort(int port);
        int port() const;

        void setPath(const QString& path);
        QString path() const;
        void setRegisterProcedure(const QString& registerProcedure);
        QString registerProcedure() const;
        void setProtocol(const QString& protocol);
        QString protocol() const;

        bool isNull() const;

        QString debugInfo() const;

        private:

        static Cloud* m_null;

        class CloudPrivate;
        CloudPrivate* d;
    };

} // namespace RSS2
} // namespace LibSyndication

#endif // LIBSYNDICATION_RSS2_CLOUD_H
