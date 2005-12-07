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

#include "cloud.h"
#include "../shared.h"

#include <QDomDocument>
#include <QDomElement>
#include <QString>

#include <kstaticdeleter.h>

namespace LibSyndication {
namespace RSS2 {

class Cloud::CloudPrivate : public LibSyndication::Shared
{
    public:
    bool isNull;
    QString domain;
    int port;
    QString path;
    QString registerProcedure;
    QString protocol;

    bool operator==(const CloudPrivate &other) const
    {
        return (isNull && other.isNull) || (domain == other.domain && port == other.port && path == other.path && registerProcedure == other.registerProcedure && protocol == other.protocol);
    }

    static CloudPrivate* copyOnWrite(CloudPrivate* ep)
    {
        if (ep->count > 1)
        {
            ep->deref();
            ep = new CloudPrivate(*ep);
        }
        return ep;
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
    Cloud obj;

    if (e.hasAttribute(QString::fromLatin1("domain")))
        obj.setDomain(e.attribute(QString::fromLatin1("domain")));
    if (e.hasAttribute(QString::fromLatin1("path")))
        obj.setPath(e.attribute(QString::fromLatin1("path")));
    if (e.hasAttribute(QString::fromLatin1("port")))
    {
        bool ok;
        int c = e.attribute(QString::fromLatin1("port")).toInt(&ok);
        obj.setPort(ok ? c : -1);
    }
    if (e.hasAttribute(QString::fromLatin1("registerProcedure")))
        obj.setRegisterProcedure(e.attribute(QString::fromLatin1("registerProcedure")));
    if (e.hasAttribute(QString::fromLatin1("protocol")))
        obj.setRegisterProcedure(e.attribute(QString::fromLatin1("protocol")));

    return obj;
}

bool Cloud::isNull() const
{
    return d->isNull;
}

Cloud::Cloud() : d(new CloudPrivate)
{
    d->port = -1;
    d->isNull = true;
}

Cloud::Cloud(const Cloud& other) : d(0)
{
    *this = other;
}

Cloud::~Cloud()
{
    if (d->deref())
    {
        delete d;
        d = 0;
    }
}

Cloud& Cloud::operator=(const Cloud& other)
{
    if (d != other.d)
    {
        other.d->ref();
        if (d && d->deref())
            delete d;
        d = other.d;
    }
    return *this;
}

bool Cloud::operator==(const Cloud& other) const
{
    return *d == *other.d;
}

void Cloud::setDomain(const QString& domain)
{
    d = CloudPrivate::copyOnWrite(d);
    d->isNull = false;
    d->domain = domain;
}

QString Cloud::domain() const
{
    return !d->isNull ? d->domain : QString::null;
}

void Cloud::setPort(int port)
{
    d = CloudPrivate::copyOnWrite(d);
    d->isNull = false;
    d->port = port;
}

int Cloud::port() const
{
   return !d->isNull ? d->port : -1;
}

void Cloud::setPath(const QString& path)
{
    d = CloudPrivate::copyOnWrite(d);
    d->isNull = false;
    d->path = path;
}

QString Cloud::path() const
{
    return !d->isNull ? d->path : QString::null;
}

void Cloud::setRegisterProcedure(const QString& registerProcedure)
{
    d = CloudPrivate::copyOnWrite(d);
    d->isNull = false;
    d->registerProcedure = registerProcedure;
}

QString Cloud::registerProcedure() const
{
    return !d->isNull ? d->registerProcedure : QString::null;
}

void Cloud::setProtocol(const QString& protocol)
{
    d = CloudPrivate::copyOnWrite(d);
    d->isNull = false;
    d->protocol = protocol;
}

QString Cloud::protocol() const
{
    return !d->isNull ? d->protocol : QString::null;
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

