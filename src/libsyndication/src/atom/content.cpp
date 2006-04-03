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

#include <tools.h>

#include <QByteArray>
#include <QDomElement>
#include <QString>
#include <QStringList>

namespace LibSyndication {
namespace Atom {

class Content::ContentPrivate
{
    public:
        
    ContentPrivate() : formatIdentified(false)
    {
    }
    mutable Format format;
    mutable bool formatIdentified;
};

Content::Content() : ElementWrapper(), d(new ContentPrivate)
{
}

Content::Content(const QDomElement& element) : ElementWrapper(element), d(new ContentPrivate)
{
}

Content::Content(const Content& other) : ElementWrapper(other), d(other.d)
{
}

Content::~Content()
{
}

Content& Content::operator=(const Content& other)
{
    ElementWrapper::operator=(other);
    d = other.d;
    return *this;
}

QString Content::type() const
{
    return attribute(QString::fromUtf8("type"));
}

QString Content::src() const
{
    return completeURI(attribute(QString::fromUtf8("src")));
}

QByteArray Content::asByteArray() const
{
    if (!isBinary())
        return QByteArray();
    return QByteArray::fromBase64(text().trimmed().toAscii());
}

static QStringList xmltypes;

Content::Format Content::mapTypeToFormat(const QString& typep,  const QString& src)
{
    QString type = typep;
    //"If neither the type attribute nor the src attribute is provided,
    //Atom Processors MUST behave as though the type attribute were
    //present with a value of "text""
    if (type.isNull() && src.isEmpty())
        type = QString::fromUtf8("text");

    if (type == QString::fromUtf8("html")
        || type == QString::fromUtf8("text/html"))
        return EscapedHTML;
    
    if (type == QString::fromUtf8("text")
        || (type.startsWith(QString::fromUtf8("text/"), Qt::CaseInsensitive)
        && !type.startsWith(QString::fromUtf8("text/xml"), Qt::CaseInsensitive))
       )
        return PlainText;
    
    if (xmltypes.isEmpty())
    {
        xmltypes.append(QString::fromUtf8("xhtml"));
        xmltypes.append(QString::fromUtf8("application/xhtml+xml"));
        // XML media types as defined in RFC3023:
        xmltypes.append(QString::fromUtf8("text/xml"));
        xmltypes.append(QString::fromUtf8("application/xml"));
        xmltypes.append(QString::fromUtf8("text/xml-external-parsed-entity"));
        xmltypes.append(QString::fromUtf8("application/xml-external-parsed-entity"));
        xmltypes.append(QString::fromUtf8("application/xml-dtd"));
    }
    
    if (xmltypes.contains(type)
        || type.endsWith(QString::fromUtf8("+xml"), Qt::CaseInsensitive)
        || type.endsWith(QString::fromUtf8("/xml"), Qt::CaseInsensitive))
        return XML;
    
    return Binary;
}

Content::Format Content::format() const
{
    if (d->formatIdentified == false)
    {
        d->format = mapTypeToFormat(type(), src());
        d->formatIdentified = true;
    }
    return d->format;
}

bool Content::isBinary() const
{
    return format() == Binary;
}

bool Content::isContained() const
{
    return src().isEmpty();
}

bool Content::isPlainText() const
{
    return format() == PlainText;
}

bool Content::isEscapedHTML() const
{
    return format() == EscapedHTML;
}

bool Content::isXML() const
{
    return format() == XML;
}

QString Content::asString() const
{
    Format f = format();
    
    if (f == PlainText)
    {
        return plainTextToHtml(text().simplified());
    }
    else if (f == EscapedHTML)
    {
        return text().simplified();
    }
    else if (f == XML)
    {
        return childNodesAsXML().simplified();
    }
    
    return QString();
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
