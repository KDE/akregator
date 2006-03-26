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
#include "content.h"
#include "document.h"
#include "parser.h"

#include <QDomAttr>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNamedNodeMap>
#include <QDomNode>
#include <QDomNodeList>

#include <QHash>
#include <QRegExp>
#include <QString>

namespace LibSyndication {
namespace Atom {

class Parser::ParserPrivate
{
    public:
    static QDomDocument convertAtom0_3(const QDomDocument& document);
    static QDomNode convertNode(QDomDocument& doc, const QDomNode& node, const QHash<QString, QString>& nameMapper);
};
        
bool Parser::accept(const LibSyndication::DocumentSource& source) const
{
    QDomDocument doc = source.asDomDocument();
    if (doc.isNull())
        return false;
    
    QDomElement feed = doc.namedItem(QString::fromUtf8("feed")).toElement();
    bool feedValid = !feed.isNull() && (feed.namespaceURI() == Constants::atom1Namespace() || feed.namespaceURI() == Constants::atom0_3Namespace());

    if (feedValid)
        return true;

    QDomElement entry = doc.namedItem(QString::fromUtf8("entry")).toElement();
    bool entryValid = !entry.isNull() && entry.namespaceURI() == Constants::atom1Namespace();

    return entryValid;
}

LibSyndication::AbstractDocumentPtr Parser::parse(const LibSyndication::DocumentSource& source) const
{
    QDomDocument doc = source.asDomDocument();

    if (doc.isNull())
    {
        // if this is not atom, return an invalid feed document
        return FeedDocumentPtr(new FeedDocument());
    }
    
    QDomElement feed = doc.namedItem(QString::fromUtf8("feed")).toElement();
    
    bool feedValid = !feed.isNull();

    if (feedValid && feed.attribute(QString::fromUtf8("version"))
        == QString::fromUtf8("0.3"))
    {
        doc = ParserPrivate::convertAtom0_3(doc);
        feed = doc.namedItem(QString::fromUtf8("feed")).toElement();
        
    }

    feedValid = !feed.isNull() && feed.namespaceURI() == Constants::atom1Namespace();
    
    if (feedValid)
    {
        return FeedDocumentPtr(new FeedDocument(feed));
    }

    QDomElement entry = doc.namedItem(QString::fromUtf8("entry")).toElement();
    bool entryValid = !entry.isNull() && entry.namespaceURI() == Constants::atom1Namespace();

    if (entryValid)
    {
        return EntryDocumentPtr(new EntryDocument(feed));
    }

    // if this is not atom, return an invalid feed document
    return FeedDocumentPtr(new FeedDocument());
}

QString Parser::format() const
{
    return QString::fromUtf8("atom");
}

QDomNode Parser::ParserPrivate::convertNode(QDomDocument& doc, const QDomNode& node, const QHash<QString, QString>& nameMapper)
{
    if (!node.isElement())
        return node.cloneNode(true);
    
    bool isAtom03Element = node.namespaceURI() == Constants::atom0_3Namespace();
    QDomElement oldEl = node.toElement();
            
    // use new namespace
    QString newNS = isAtom03Element ? Constants::atom1Namespace() : node.namespaceURI();
    
    QString newName = node.localName();
    
    // rename tags that are listed in the nameMapper
    if (isAtom03Element && nameMapper.contains(node.localName()))
        newName = nameMapper[node.localName()];
    
    QDomElement newEl = doc.createElementNS(newNS, newName);
    
    QDomNamedNodeMap attributes = oldEl.attributes();
    
    // copy over attributes
    for (int i = 0; i < attributes.count(); ++i)
    {
        QDomAttr attr = attributes.item(i).toAttr();
        if (attr.namespaceURI().isEmpty())
            newEl.setAttribute(attr.name(), attr.value());
        else
            newEl.setAttributeNS(attr.namespaceURI(), attr.name(), attr.value());
    }
    
    bool isTextConstruct = newNS == Constants::atom1Namespace() 
            && (newName == QString::fromUtf8("title")
            || newName == QString::fromUtf8("rights")
            || newName == QString::fromUtf8("subtitle")
            || newName == QString::fromUtf8("summary"));
    
    // for atom text constructs, map to new type schema (which only allows text, type, xhtml)
    
    if (isTextConstruct)
    {
        QString oldType = newEl.attribute(QString::fromUtf8("type"), QString::fromUtf8("text/plain") );
        QString newType;
        
        Content::Format format = Content::mapTypeToFormat(oldType);
        switch (format)
        {
            case Content::XML:
                newType = QString::fromUtf8("xhtml");
                break;
            case Content::EscapedHTML:
                newType = QString::fromUtf8("html");
                break;
            case Content::PlainText:
            case Content::Binary:
            default:
                newType = QString::fromUtf8("text");
                
        }
        
        newEl.setAttribute(QString::fromUtf8("type"), newType);
    }
    else
    {
        // for generator, rename the "url" attribute to "uri"
        
        bool isGenerator = newNS == Constants::atom1Namespace() && newName == QString::fromUtf8("generator");        
        if (isGenerator && newEl.hasAttribute(QString::fromUtf8("url")))
            newEl.setAttribute(QString::fromUtf8("uri"), newEl.attribute(QString::fromUtf8("url")));
    }
    
    // process child nodes recursively and append them
    QDomNodeList children = node.childNodes();
    for (int i = 0; i < children.count(); ++i)
    {
        newEl.appendChild(convertNode(doc, children.item(i), nameMapper));
    }
    
    return newEl;
}

QDomDocument Parser::ParserPrivate::convertAtom0_3(const QDomDocument& doc03)
{
    QDomDocument doc = doc03.cloneNode(false).toDocument();
    
    // these are the tags that were renamed in 1.0
    QHash<QString, QString> nameMapper;
    nameMapper.insert(QString::fromUtf8("issued"), QString::fromUtf8("published"));
    nameMapper.insert(QString::fromUtf8("modified"), QString::fromUtf8("updated"));
    nameMapper.insert(QString::fromUtf8("url"), QString::fromUtf8("uri"));
    nameMapper.insert(QString::fromUtf8("copyright"), QString::fromUtf8("rights"));
    nameMapper.insert(QString::fromUtf8("tagline"), QString::fromUtf8("subtitle"));
    
    QDomNodeList children = doc03.childNodes();
    
    for (int i = 0; i < children.count(); ++i)
    {
        doc.appendChild(convertNode(doc, children.item(i), nameMapper));
    }

    return doc;
}

Parser::Parser() {}
Parser::~Parser() {}
Parser::Parser(const Parser& other) : AbstractParser(other) {}
Parser& Parser::operator=(const Parser& /*other*/) { return *this; }

} // namespace Atom
} // namespace LibSyndication
