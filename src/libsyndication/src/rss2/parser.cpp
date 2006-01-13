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

#include "../documentsource.h"

#include "document.h"
#include "parser.h"

#include <QDomDocument>
#include <QDomElement>
#include <QString>

namespace LibSyndication {
namespace RSS2 {

bool Parser::accept(const LibSyndication::DocumentSource& source) const
{
    QDomDocument doc = source.asDomDocument();
    if (doc.isNull())
        return false;
    
    QDomNode root = doc.namedItem(QString::fromLatin1("rss"));

    return (root.isElement() && root.toElement().hasAttribute(QString::fromLatin1("version")));
}

Document* Parser::parse(const LibSyndication::DocumentSource& source) const
{
    return new Document(Document::fromXML(source.asDomDocument()));
}

QString Parser::format() const
{
    return QString::fromLatin1("rss2");
}

} // namespace RSS2
} // namespace LibSyndication
