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

#include <documentsource.h>

#include "document.h"
#include "parser.h"

#include <QDomDocument>
#include <QDomElement>
#include <QString>

namespace LibSyndication {
namespace RSS2 {

class Parser::ParserPrivate {};
    
bool Parser::accept(const LibSyndication::DocumentSource& source) const
{
    QDomDocument doc = source.asDomDocument();
    if (doc.isNull())
        return false;
    
    QDomNode root = doc.namedItem(QString::fromUtf8("rss")).toElement();

    return !root.isNull();
}

LibSyndication::SpecificDocumentPtr Parser::parse(const LibSyndication::DocumentSource& source) const
{
    return DocumentPtr(new Document(Document::fromXML(source.asDomDocument())));
}

QString Parser::format() const
{
    return QString::fromUtf8("rss2");
}

Parser::Parser() {}
Parser::Parser(const Parser& other) : AbstractParser(other) {}
Parser::~Parser() {}
Parser& Parser::operator=(const Parser& /*other*/) { return *this; }

} // namespace RSS2
} // namespace LibSyndication
