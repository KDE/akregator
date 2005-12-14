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

#include "constants.h"
#include "tools.h"

#include <QDomElement>
#include <QIODevice>
#include <QList>
#include <QString>
#include <QTextStream>

namespace LibSyndication {
namespace RSS2 {

QString Tools::extractElementText(const QDomNode& parent, const QString& tagName)
{

    QDomNode node = parent.namedItem(tagName);

    if (node.isNull() || !node.isElement())
        return QString::null;
    else
        return node.toElement().text();
}

QString Tools::childNodesAsXML(const QDomNode& parent)
{
    QDomNodeList list = parent.childNodes();
    QString str;
    QTextStream ts( &str, QIODevice::WriteOnly );
    for (int i = 0; i < list.count(); ++i)
        ts << list.item(i);
    return str.trimmed();
}

QString Tools::extractContent(const QDomNode& parent)
{
    QList<QDomElement> list = elementsByTagNameNS(parent, Constants::contentNameSpace(), QString::fromLatin1("encoded"));

    if (!list.isEmpty())
        return list.first().text();

    list = elementsByTagNameNS(parent, Constants::XHTMLNameSpace(), QString::fromLatin1("body"));

    if (!list.isEmpty())
        return childNodesAsXML(list.first());

    list = elementsByTagNameNS(parent, Constants::XHTMLNameSpace(), QString::fromLatin1("div"));

    if (!list.isEmpty())
        return childNodesAsXML(list.first());

    return QString::null;

}

QList<QDomElement> Tools::elementsByTagName(const QDomNode& parent, const QString& tagName)
{
    QList<QDomElement> elements;
    for (QDomNode n = parent.firstChild(); !n.isNull(); n = n.nextSibling())
    {
        if (n.isElement())
        {
            QDomElement e = n.toElement();
            if (e.tagName() == tagName)
                elements.append(e);
        }
    }
    return elements;
}

QList<QDomElement> Tools::elementsByTagNameNS(const QDomNode& parent, const QString& nsURI, const QString& localName)
{
    QList<QDomElement> elements;
    for (QDomNode n = parent.firstChild(); !n.isNull(); n = n.nextSibling())
    {
        if (n.isElement())
        {
            QDomElement e = n.toElement();
            if (e.localName() == localName && e.namespaceURI() == nsURI)
                elements.append(e);
        }
    }
    return elements;
}

} // namespace RSS2
} // namespace LibSyndication
