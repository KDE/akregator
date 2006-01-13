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

#include "../documentsource.h"

#include "constants.h"
#include "document.h"
#include "parser.h"

#include <QDomDocument>
#include <QDomElement>
#include <QString>

namespace LibSyndication {
namespace Atom {

bool Parser::accept(const LibSyndication::DocumentSource& source) const
{
    QDomDocument doc = source.asDomDocument();
    if (doc.isNull())
        return false;
    
    QDomElement feed = doc.namedItem(QString::fromLatin1("feed")).toElement();
    bool feedValid = !feed.isNull() && feed.namespaceURI() == Constants::atom1NameSpace();

    if (feedValid)
        return true;

    QDomElement entry = doc.namedItem(QString::fromLatin1("entry")).toElement();
    bool entryValid = !entry.isNull() && entry.namespaceURI() == Constants::atom1NameSpace();

    return entryValid;
}

LibSyndication::AbstractDocument* Parser::parse(const LibSyndication::DocumentSource& source) const
{
    QDomDocument doc = source.asDomDocument();

    if (doc.isNull())
        return 0;

    
    QDomElement feed = doc.namedItem(QString::fromLatin1("feed")).toElement();
    bool feedValid = !feed.isNull() && feed.namespaceURI() == Constants::atom1NameSpace();

    if (feedValid)
    {
        return new FeedDocument(feed);
    }

    QDomElement entry = doc.namedItem(QString::fromLatin1("entry")).toElement();
    bool entryValid = !entry.isNull() && entry.namespaceURI() == Constants::atom1NameSpace();

    if (entryValid)
    {
        return new EntryDocument(entry);
    }

    return 0;
}

QString Parser::format() const
{
    return QString::fromLatin1("atom");
}

} // namespace Atom
} // namespace LibSyndication
