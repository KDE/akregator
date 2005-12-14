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

#include "cloud.h"

#include <QDomDocument>
#include <QDomElement>
#include <QString>

#include <kstaticdeleter.h>

namespace LibSyndication {
namespace RSS2 {

class Cloud::CloudPrivate : public KShared
{
    public:

    QString domain;
    int port;
    QString path;
    QString registerProcedure;
    QString protocol;

    bool operator==(const CloudPrivate &other) const
    {
        return (domain == other.domain && port == other.port && path == other.path && registerProcedure == other.registerProcedure && protocol == other.protocol);
    }
};

Cloud* Cloud::m_null = 0;
static KStaticDeleter<Cloud> cloudsd;

const Cloud& Cloud::null()
{
    if (m_null == 0)
        cloudsd.setObject(m_null, new Cloud);

    return *m_null;
}

Cloud Cloud::fromXML(const QDomElement& e)
{
    QString domain = e.attribute(QString::fromLatin1("domain"));
    QString path = e.attribute(QString::fromLatin1("path"));

    int port = -1;
    if (e.hasAttribute(QString::fromLatin1("port")))
    {
        bool ok;
        int c = e.attribute(QString::fromLatin1("port")).toInt(&ok);
        port = ok ? c : -1;
    }

    QString registerProcedure = e.attribute(QString::fromLatin1("registerProcedure"));

    QString protocol = e.attribute(QString::fromLatin1("protocol"));

    return Cloud(domain, path, registerProcedure, protocol, port);
}

bool Cloud::isNull() const
{
    return !d;
}

Cloud::Cloud() : d(0)
{
}

Cloud::Cloud(const QString& domain, const QString& path, const QString& registerProcedure, const QString& protocol, int port) : d(new CloudPrivate)
{
    d->domain = domain;
    d->path = path;
    d->registerProcedure = registerProcedure;
    d->protocol = protocol;
    d->port = port;
}

Cloud::Cloud(const Cloud& other) : d(0)
{
    *this = other;
}

Cloud::~Cloud()
{
}

Cloud& Cloud::operator=(const Cloud& other)
{
    d = other.d;
    return *this;
}

bool Cloud::operator==(const Cloud& other) const
{
    if (!d || !other.d)
        return d == other.d;
    return *d == *other.d;
}

QString Cloud::domain() const
{
    return d ? d->domain : QString::null;
}

int Cloud::port() const
{
   return d ? d->port : -1;
}

QString Cloud::path() const
{
    return d ? d->path : QString::null;
}

QString Cloud::registerProcedure() const
{
    return d ? d->registerProcedure : QString::null;
}

QString Cloud::protocol() const
{
    return d ? d->protocol : QString::null;
}

QString Cloud::debugInfo() const
{
    QString info;
    info += "### Cloud: ###################\n";
    info += "domain: #" + domain() + "#\n";
    info += "port: #" + QString::number(port()) + "#\n";
    info += "path: #" + path() + "#\n";
    info += "registerProcedure: #" + registerProcedure() + "#\n";
    info += "protocol: #" + protocol() + "#\n";
    info += "### Cloud end ################\n";
    return info;
}


} // namespace RSS2
} // namespace LibSyndication

