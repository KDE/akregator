/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2006 Frank Osterfeld <frank.osterfeld@kdemail.net>
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

#include "constants.h"
#include "link.h"

#include <QDomElement>
#include <QString>

namespace LibSyndication {
namespace Atom {

Link::Link() : ElementWrapper()
{
}

Link::Link(const QDomElement& element) : ElementWrapper(element)
{
}

QString Link::href() const
{
    return element().attributeNS(Constants::atom1NameSpace(),
                                QString::fromLatin1("href"));
}

QString Link::rel() const
{
    return element().attributeNS(Constants::atom1NameSpace(),
    QString::fromLatin1("rel"));
}

QString Link::type() const
{
    return element().attributeNS(Constants::atom1NameSpace(),
    QString::fromLatin1("type"));
}

QString Link::hrefLanguage() const
{
    return element().attributeNS(Constants::atom1NameSpace(),
    QString::fromLatin1("hreflang"));
}

QString Link::title() const
{
    return element().attributeNS(Constants::atom1NameSpace(),
    QString::fromLatin1("title"));
}

uint Link::length() const
{
    QString lengthStr = element().attributeNS(Constants::atom1NameSpace(),
                        QString::fromLatin1("length"));
    
    bool ok;
    uint c = lengthStr.toUInt(&ok);
    return ok ? c : 0;
}

QString Link::debugInfo() const
{
    return "TODO";
}

} // namespace Atom
} //namespace LibSyndication
