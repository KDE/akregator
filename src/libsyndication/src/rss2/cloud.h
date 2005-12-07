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

#include <ksharedptr.h>

class QDomDocument;
class QDomElement;
class QString;

namespace LibSyndication {
namespace RSS2
{
    /**
     * Cloud information for an RSS channel.
     * It specifies a web service that supports the rssCloud interface which can
     * be implemented in HTTP-POST, XML-RPC or SOAP 1.1.
     * Its purpose is to allow processes to register with a cloud to be notified
     * of updates to the channel,
     * implementing a lightweight publish-subscribe protocol for RSS feeds.
     *
     * Example:
     *
     * Domain="rpc.sys.com", port="80", path="/RPC2"
     * registerProcedure="myCloud.rssPleaseNotify" protocol="xml-rpc"
     *
     * In this example, to request notification on the channel it appears in,
     * you would send an XML-RPC message to rpc.sys.com on port 80, with a path
     * of /RPC2. The procedure to call is myCloud.rssPleaseNotify.
     *
     * For more information on the rssCloud interface see
     * http://blogs.law.harvard.edu/tech/soapMeetsRss#rsscloudInterface
     *
     * (Note: This explanation was taken from 
     * @link http://blogs.law.harvard.edu/tech/rss)
     *
     * @author Frank Osterfeld
     */
    class Cloud
    {
        public:

        static const Cloud& null();

        static Cloud fromXML(const QDomElement& e);

        Cloud();

        Cloud(const Cloud& other);
        virtual ~Cloud();

        Cloud& operator=(const Cloud& other);
        bool operator==(const Cloud& other) const;

        QString domain() const;

        int port() const;

        QString path() const;

        QString registerProcedure() const;

        QString protocol() const;

        bool isNull() const;

        QString debugInfo() const;

        private:

        Cloud(const QString& domain, const QString& path, const QString& registerProcedure, const QString& protocol, int port);

        static Cloud* m_null;

        class CloudPrivate;
        KSharedPtr<CloudPrivate> d;
    };

} // namespace RSS2
} // namespace LibSyndication

#endif // LIBSYNDICATION_RSS2_CLOUD_H
