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

#include "content.h"

#include <QByteArray>
#include <QDomElement>
#include <QString>
#include <QStringList>

namespace LibSyndication {
namespace Atom {

Content::Content() : ElementWrapper()
{
}

Content::Content(const QDomElement& element) : ElementWrapper(element)
{
}

QString Content::type() const
{
    return element().attribute(QString::fromUtf8("type"));
}

QString Content::src() const
{
    return completeURI(element().attribute(QString::fromUtf8("src")));
}

QByteArray Content::asByteArray() const
{
    if (isText())
        return QByteArray();
    
    // TODO: return base64-encoded content
    return QByteArray();
}

Content::Format Content::format() const
{
    QString ctype = type();
    
    //"If neither the type attribute nor the src attribute is provided,
    //Atom Processors MUST behave as though the type attribute were
    //present with a value of "text""
    if (ctype.isNull() && src().isEmpty())
        ctype = QString::fromUtf8("text");

    if (ctype.isEmpty() 
            || ctype == QString::fromUtf8("text")
            || ctype == QString::fromUtf8("html")
            || (ctype.startsWith(QString::fromUtf8("text/"), Qt::CaseInsensitive)
            && !ctype.startsWith(QString::fromUtf8("text/xml"), Qt::CaseInsensitive))
           )
        return Text;
    
    QStringList xmltypes;
    xmltypes.append(QString::fromUtf8("xhtml"));
    // XML media types as defined in RFC3023:
    xmltypes.append(QString::fromUtf8("text/xml"));
    xmltypes.append(QString::fromUtf8("application/xml"));
    xmltypes.append(QString::fromUtf8("text/xml-external-parsed-entity"));
    xmltypes.append(QString::fromUtf8("application/xml-external-parsed-entity"));
    xmltypes.append(QString::fromUtf8("application/xml-dtd"));
    
    
    if (xmltypes.contains(ctype)
            || ctype.endsWith(QString::fromUtf8("+xml"), Qt::CaseInsensitive)
        || ctype.endsWith(QString::fromUtf8("/xml"), Qt::CaseInsensitive))
        return XML;
    
    return Binary;
}

bool Content::isBinary() const
{
    return format() == Binary;
}

bool Content::isContained() const
{
    return src().isEmpty();
}

bool Content::isText() const
{
    return format() == Text;
}

bool Content::isXML() const
{
    return format() == XML;
}

QString Content::asString() const
{
    QString str;

    if (isText())
    {
        str = element().text().simplified();
        str = str.replace("&", "&amp;");
        str = str.replace("\"", "&quot;");
        str = str.replace("<", "&lt;");
    }
    else if (isXML())
    {
        str = childNodesAsXML().simplified();
    }
    
    return str;
    
}

QString Content::debugInfo() const
{
    QString info;
    info += "### Content: ###################\n";
    info += "type: #" + type() + "#\n";
    if (!src().isNull())
        info += "src: #" + src() + "#\n";
    if (!isBinary())
        info += "content: #" + asString() + "#\n";
    else
    {
        info += "binary length: #" + QString::number(asByteArray().size()) + "#\n";
    }
    info += "### Content end ################\n";

    return info;
}

} // namespace Atom
} //namespace LibSyndication
