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
#include "../tools.h"

#include <QDomElement>
#include <QString>

namespace LibSyndication {

namespace Atom {

QString extractAtomText(const LibSyndication::ElementWrapper& parent, const QString& tagname)
{
    QString str;
    
    QDomElement el = parent.firstElementByTagNameNS(atom1Namespace(), tagname);
    
    QString type = el.attribute(QString::fromUtf8("type"), QString::fromUtf8("text"));
    
    if (type == QString::fromUtf8("text"))
    {
        str = plainTextToHtml(parent.extractElementTextNS(atom1Namespace(), tagname).simplified());
    }
    else if (type == QString::fromUtf8("html"))
    {
        str = parent.extractElementTextNS(atom1Namespace(), tagname).simplified();
    }
    else if (type == QString::fromUtf8("xhtml"))
    {
        str = ElementWrapper::childNodesAsXML(el).simplified();
    }
    
    return str;
}

} // namespace Atom
} // namespace LibSyndication


