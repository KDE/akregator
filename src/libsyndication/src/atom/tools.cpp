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

#include "tools.h"

#include <QDomDocument>
#include <QString>

namespace LibSyndication {
namespace Atom {

bool Tools::isRelativeURL(const QString& url)
{
    return false; // TODO
}
    
QDomDocument Tools::convertAtom(const QDomDocument& document)
{
    QString src = document.toString();
    
    src = src.replace("http://purl.org/atom/ns#", "http://www.w3.org/2005/Atom");
    src.replace(QRegExp("(<[^=>]*)issued([^>]*>)"), "\\1published\\2");
    src.replace(QRegExp("(<[^=>]*)modified([^>]*>)"), "\\1updated\\2");
    src.replace(QRegExp("(<[^=>]*generator[^>]*)url(\\s*=*>)"), "\\1uri\\2");
    src.replace(QRegExp("(<[^=>]*)url([^>]*>)"), "\\1uri\\2");
    src.replace(QRegExp("(<[^=>]*)copyright([^>]*>)"), "\\1rights\\2");
    src.replace(QRegExp("(<[^=>]*)tagline([^>]*>)"), "\\1subtitle\\2");
}

} // namespace Atom
} // namespace LibSyndication
