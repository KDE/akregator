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

#include "document.h"
#include "model.h"
#include "modelmaker.h"
#include "parser.h"
#include "property.h"
#include "rdfvocab.h"
#include "resource.h"
#include "rssvocab.h"
#include "statement.h"

#include "../documentsource.h"

#include <QDomDocument>
#include <QDomNodeList>
#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>


namespace LibSyndication {
namespace RDF {

class Parser::ParserPrivate
{
    public:
    static void map09to10(Model model);
};

bool Parser::accept(const DocumentSource& source) const
{
    QDomDocument doc = source.asDomDocument();
    
    if (doc.isNull())
        return false;
    QDomElement root = doc.documentElement();
    
    if (!root.isElement())
        return false;
    
    return root.namespaceURI() == RDFVocab::self()->namespaceURI();
}

LibSyndication::AbstractDocumentPtr Parser::parse(const DocumentSource& source) const
{
    QDomDocument doc = source.asDomDocument();
    
    if (doc.isNull())
        return LibSyndication::AbstractDocumentPtr(new Document());
    
    ModelMaker maker;
    Model model = maker.createFromXML(doc);
    
    bool is09 = !model.resourcesWithType(RSS09Vocab::self()->channel()).isEmpty();
    
    if (is09)
        ParserPrivate::map09to10(model);
    
    QList<ResourcePtr> channels = model.resourcesWithType(RSSVocab::self()->channel());
    
    if (channels.isEmpty())
        return LibSyndication::AbstractDocumentPtr(new Document());
  
    return DocumentPtr(new Document(*(channels.begin())));
}

void Parser::ParserPrivate::map09to10(Model model)
{
    QHash<QString, PropertyPtr> hash;
    
    hash.insert(RSS09Vocab::self()->title()->uri(), RSSVocab::self()->title());
    hash.insert(RSS09Vocab::self()->description()->uri(), RSSVocab::self()->description());
    hash.insert(RSS09Vocab::self()->link()->uri(), RSSVocab::self()->link());
    hash.insert(RSS09Vocab::self()->name()->uri(), RSSVocab::self()->name());
    hash.insert(RSS09Vocab::self()->url()->uri(), RSSVocab::self()->url());
    hash.insert(RSS09Vocab::self()->image()->uri(), RSSVocab::self()->image());
    hash.insert(RSS09Vocab::self()->textinput()->uri(), RSSVocab::self()->textinput());
        
    QStringList uris09 = RSS09Vocab::self()->properties();
    
    // map statement predicates to RSS 1.0
    
    QList<StatementPtr> statements = model.statements();
    QList<StatementPtr>::ConstIterator it = statements.begin();
    QList<StatementPtr>::ConstIterator end = statements.end();
    
    for ( ; it != end; ++it)
    {
        StatementPtr stmt = *it;
        
        QString predUri = stmt->predicate()->uri();
        if (uris09.contains(predUri))
        {
        model.addStatement(stmt->subject(), hash[predUri], stmt->object());
        }
    }
    // map channel type
    QList<ResourcePtr> channels = model.resourcesWithType(RSS09Vocab::self()->channel());
    
    ResourcePtr channel;
    
    if (!channels.isEmpty())
    {
        channel = *(channels.begin());
        
        model.removeStatement(channel, RDFVocab::self()->type(), RSS09Vocab::self()->channel());
        model.addStatement(channel, RDFVocab::self()->type(), RSSVocab::self()->channel());
        
        // add Sequence of items as used in RSS 1.0
        SequencePtr seq = model.createSequence();
        model.addStatement(channel, RSSVocab::self()->items(), seq);
        QList<ResourcePtr> items = model.resourcesWithType(RSS09Vocab::self()->item());
        
        QList<ResourcePtr>::ConstIterator it2 = items.begin();
        QList<ResourcePtr>::ConstIterator end2 = items.end();

        for ( ; it2 != end2; ++it2)
        {
            seq->append(*it2);
            model.addStatement(seq, RDFVocab::self()->li(), *it2);
        }
    }
}

Parser::Parser() {}
Parser::~Parser() {}
Parser::Parser(const Parser& other) : AbstractParser(other) {}
Parser& Parser::operator=(const Parser& /*other*/) { return *this; }
       
QString Parser::format() const
{
    return QString::fromUtf8("rdf");
}

        
} // namespace RDF
} // namespace LibSyndication
