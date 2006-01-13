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

Cloud Cloud::fromXML(const QDomElement& e)
{
    return Cloud(e);
}

Cloud::Cloud() : ElementWrapper()
{
}

Cloud::Cloud(const QDomElement& element) : ElementWrapper(element)
{
}

QString Cloud::domain() const
{
    return element().attribute(QString::fromLatin1("domain"));
}

int Cloud::port() const
{
    if (element().hasAttribute(QString::fromLatin1("port")))
    {
        bool ok;
        int c = element().attribute(QString::fromLatin1("port")).toInt(&ok);
        return ok ? c : -1;
    }
    
    return -1;
}

QString Cloud::path() const
{
    return element().attribute(QString::fromLatin1("path"));
}

QString Cloud::registerProcedure() const
{
    return element().attribute(QString::fromLatin1("registerProcedure"));
}

QString Cloud::protocol() const
{
    
    return element().attribute(QString::fromLatin1("protocol"));
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

