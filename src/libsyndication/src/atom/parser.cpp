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
#include <QRegExp>
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

LibSyndication::AbstractDocumentPtr Parser::parse(const LibSyndication::DocumentSource& source) const
{
    QDomDocument doc = source.asDomDocument();

    if (doc.isNull())
        return 0;

    
    QDomElement feed = doc.namedItem(QString::fromLatin1("feed")).toElement();
    
    bool feedValid = !feed.isNull();

    if (feedValid && feed.attribute(QString::fromLatin1("version"))
        == QString::fromLatin1("0.3"))
    {
        doc = convertAtom0_3(doc);
        feed = doc.namedItem(QString::fromLatin1("feed")).toElement();
        
    }

    feedValid = !feed.isNull() && feed.namespaceURI() == Constants::atom1NameSpace();
    
    if (feedValid)
    {
        FeedDocumentPtr ptr = new FeedDocument(feed);
        return LibSyndication::AbstractDocumentPtr::staticCast(ptr);
    }

    QDomElement entry = doc.namedItem(QString::fromLatin1("entry")).toElement();
    bool entryValid = !entry.isNull() && entry.namespaceURI() == Constants::atom1NameSpace();

    if (entryValid)
    {
        EntryDocumentPtr ptr = new EntryDocument(feed);
        return LibSyndication::AbstractDocumentPtr::staticCast(ptr);
    }

    return 0;
}

QString Parser::format() const
{
    return QString::fromLatin1("atom");
}

QDomDocument Parser::convertAtom0_3(const QDomDocument& doc)
{
    QString str = doc.toString();
    
    str.replace(QRegExp("(<[^>]*)http://purl.org/atom/ns#([^>]*>)"), "\\1http://www.w3.org/2005/Atom\\2");
    str.replace(QRegExp("(<[^=>]*)issued([^>]*>)"), "\\1published\\2");
    str.replace(QRegExp("(<[^=>]*)modified([^>]*>)"), "\\1updated\\2");
    str.replace(QRegExp("(<[^=>]*generator[^>]*)url(\\s*=*>)"), "\\1uri\\2");
    str.replace(QRegExp("(<[^=>]*)url([^>]*>)"), "\\1uri\\2");
    str.replace(QRegExp("(<[^=>]*)copyright([^>]*>)"), "\\1rights\\2");
    str.replace(QRegExp("(<[^=>]*)tagline([^>]*>)"), "\\1subtitle\\2");
    
    return DocumentSource(str.toUtf8()).asDomDocument();
}

} // namespace Atom
} // namespace LibSyndication
