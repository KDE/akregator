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
#include "tools.h"

#include "../elementwrapper.h"

#include <QDomElement>
#include <QString>

namespace LibSyndication {

namespace Atom {

QString extractAtomText(const LibSyndication::ElementWrapper& parent, const QString& tagname)
{
    QString str;
    
    QString type = parent.element().attribute(QString::fromUtf8("type"));
    
    if (type.isEmpty() || type == QString::fromUtf8("text"))
    {
        str = parent.extractElementTextNS(Constants::atom1NameSpace(), tagname);
        str = str.replace("<", "&lt;");
        str = str.replace(">", "&gt;");
        str = str.replace("&", "&amp;");
        str = str.replace("\"", "&quot;");
    }
    else if (type == QString::fromUtf8("html"))
    {
        str = parent.extractElementTextNS(Constants::atom1NameSpace(), tagname);
    }
    else if (type == QString::fromUtf8("xhtml"))
    {
        QDomElement el = parent.firstElementByTagNameNS(Constants::atom1NameSpace(), tagname);
        str = ElementWrapper::childNodesAsXML(el);
    }
    
    return str;
}

} // namespace Atom
} // namespace LibSyndication


