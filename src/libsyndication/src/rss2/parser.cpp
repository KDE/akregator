/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
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

LibSyndication::Document* Parser::parse(const LibSyndication::DocumentSource& source) const
{
    return new Document(Document::fromXML(source.asDomDocument()));
}

QString Parser::format() const
{
    return "rss2";
}

} // namespace RSS2
} // namespace LibSyndication
