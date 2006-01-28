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


Cloud::Cloud() : ElementWrapper()
{
}

Cloud::Cloud(const QDomElement& element) : ElementWrapper(element)
{
}

QString Cloud::domain() const
{
    return element().attribute(QString::fromUtf8("domain"));
}

int Cloud::port() const
{
    if (element().hasAttribute(QString::fromUtf8("port")))
    {
        bool ok;
        int c = element().attribute(QString::fromUtf8("port")).toInt(&ok);
        return ok ? c : -1;
    }
    
    return -1;
}

QString Cloud::path() const
{
    return element().attribute(QString::fromUtf8("path"));
}

QString Cloud::registerProcedure() const
{
    return element().attribute(QString::fromUtf8("registerProcedure"));
}

QString Cloud::protocol() const
{
    
    return element().attribute(QString::fromUtf8("protocol"));
}

QString Cloud::debugInfo() const
{
    QString info;
    info += "### Cloud: ###################\n";
    if (!domain().isNull())
        info += "domain: #" + domain() + "#\n";
    if (port() != -1)
        info += "port: #" + QString::number(port()) + "#\n";
    if (!path().isNull())
        info += "path: #" + path() + "#\n";
    if (!registerProcedure().isNull())
        info += "registerProcedure: #" + registerProcedure() + "#\n";
    if (!protocol().isNull())
        info += "protocol: #" + protocol() + "#\n";
    info += "### Cloud end ################\n";
    return info;
}


} // namespace RSS2
} // namespace LibSyndication

