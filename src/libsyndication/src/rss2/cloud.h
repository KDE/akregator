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
   * @link http://blogs.law.harvard.edu/tech/soapMeetsRss#rsscloudInterface
   *
   * (Note: This explanation was taken from 
   * @link http://blogs.law.harvard.edu/tech/rss)
   *
   * @author Frank Osterfeld
   */
class Cloud
{
    public:

        /**
         * static null object. See also Cloud() and isNull().
         *
         * @return reference to a static null object
         */
        static const Cloud& null();

        /**
         * Parses a cloud object from an <cloud> XML element.
         *
         * @param e The <cloud> element to parse the cloud from
         * @return the cloud parsed from XML, or a null object
         *         if parsing failed.
         */
        static Cloud fromXML(const QDomElement& e);

        /**
         * Default constructor, creates a null object, which is equal
         * to Cloud::null() and for which isNull() is @c true.
         */
        Cloud();

        /**
         * Copy constructor, creates a copy of @c other.
         * The d pointer is shared, so this is a cheap operation.
         *
         * @param other the object to be copied
         */
        Cloud(const Cloud& other);

        /**
         * Destructor.
         */
        virtual ~Cloud();

        /**
         * Assigns the values of @c other. The d pointer is shared, so
         * this is a cheap operation.
         *
         * @param other The object to assign
         * @return a reference to this object
         */
        Cloud& operator=(const Cloud& other);

        /**
         * Checks whether this cloud is equal to another.
         * Cloud are equal if all properties are equal.
         *
         * @param other another cloud
         * @return whether this object is equal to @c other or not
         */
        bool operator==(const Cloud& other) const;

        QString domain() const;

        int port() const;

        QString path() const;

        QString registerProcedure() const;

        QString protocol() const;

        /**
         * returns whether this object is a null object.
         *
         * @return @c true, if this is a null object, otherwise @c false
         */
        bool isNull() const;

        /**
         * Returns a description of the object for debugging purposes.
         *
         * @return debug string
         */
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
