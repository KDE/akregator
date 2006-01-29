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

#include "abstractdocument.h"
#include "abstractparser.h"
#include "documentsource.h"
#include "documentvisitor.h"
#include "parsercollection.h"
#include "feed.h"
#include "atom/document.h"
#include "atom/parser.h"
#include "rdf/document.h"
#include "rdf/parser.h"
#include "rss2/document.h"
#include "rss2/parser.h"
#include "mapper/feedatomimpl.h"
#include "mapper/feedrdfimpl.h"
#include "mapper/feedrss2impl.h"

#include <QDomDocument>
#include <QHash>
#include <QString>

#include <kstaticdeleter.h>

namespace LibSyndication {

static KStaticDeleter<ParserCollection> parserregistrysd;

ParserCollection* ParserCollection::m_self = 0;

class ParserCollection::ParserCollectionPrivate
{
    public:
    QHash<QString, AbstractParser*> parsers;
    ErrorCode lastError;
    
    ParserCollectionPrivate(ParserCollection* reg) : p(reg)
    {
        docVisitor = new DocVisitor;
        lastError = LibSyndication::Success;
    }
    
    ~ParserCollectionPrivate()
    {
        delete docVisitor;
    }
    
    class DocVisitor : public LibSyndication::DocumentVisitor
    {
        public:
            
        FeedPtr createFeed(AbstractDocumentPtr ptr)
        {
            m_ptr = ptr;
            visit(ptr.data());
            return feedptr;
        }
        
        bool visitAtomFeedDocument(LibSyndication::Atom::FeedDocument* document)
        {
            LibSyndication::Atom::FeedDocumentPtr tptr = 
                LibSyndication::Atom::FeedDocumentPtr::staticCast(m_ptr);
            
            feedptr = new FeedAtomImpl(tptr);
            return true;
        }
        
        bool visitAtomEntryDocument(LibSyndication::Atom::EntryDocument* /*document*/)
        {
            // TODO
            return false;
        }
        
        bool visitRDFDocument(LibSyndication::RDF::Document* document)
        {
            LibSyndication::RDF::DocumentPtr tptr = 
                    LibSyndication::RDF::DocumentPtr::staticCast(m_ptr);
            
            feedptr = new FeedRDFImpl(tptr);
            return true;
        }
        
        bool visitRSS2Document(LibSyndication::RSS2::Document* document)
        {
            LibSyndication::RSS2::DocumentPtr tptr = 
                    LibSyndication::RSS2::DocumentPtr::staticCast(m_ptr);
            
            feedptr = new FeedRSS2Impl(tptr);
            return true;
        }
        
        FeedPtr feedptr;
        AbstractDocumentPtr m_ptr;
    };
    
    DocVisitor* docVisitor;
    
    private:
        
    ParserCollection* p;
};

ParserCollection* ParserCollection::self()
{
    if (m_self == 0)
        parserregistrysd.setObject(m_self, new ParserCollection);
    return m_self;
}

ParserCollection::ParserCollection()
{
    d = new ParserCollectionPrivate(this);
    registerParser(new LibSyndication::RSS2::Parser);
    registerParser(new LibSyndication::RDF::Parser);
    registerParser(new LibSyndication::Atom::Parser);
}

ParserCollection::~ParserCollection()
{
    QList<AbstractParser*> list = d->parsers.values();
    QList<AbstractParser*>::ConstIterator it = list.begin();
    QList<AbstractParser*>::ConstIterator end = list.end();
    
    for ( ; it != end; ++it)
        delete *it;
    
    delete d;
    d = 0;
}

bool ParserCollection::registerParser(AbstractParser* parser)
{
    if (d->parsers.contains(parser->format()))
        return false;

    d->parsers.insert(parser->format(), parser);
    return true;
}

FeedPtr ParserCollection::parse(const DocumentSource& source, const QString& formatHint)
{
    d->lastError = LibSyndication::Success;

    if (d->parsers.contains(formatHint))
    {
        if (d->parsers[formatHint]->accept(source))
        {
            AbstractDocumentPtr doc = d->parsers[formatHint]->parse(source);
            if (!doc)
            {
                d->lastError = InvalidFormat;
                return 0;
            }
            
            return d->docVisitor->createFeed(doc);
        }
    }

    Q_FOREACH (AbstractParser* i, d->parsers)
    {
        if (i->accept(source))
        {
            AbstractDocumentPtr doc = i->parse(source);
            if (!doc)
            {
                d->lastError = InvalidFormat;
                return 0;
            }
            
            return d->docVisitor->createFeed(doc);
        }
    }
    if (source.asDomDocument().isNull())
        d->lastError = InvalidXml;
    else
        d->lastError = XmlNotAccepted;
    
    return 0;
}

LibSyndication::ErrorCode ParserCollection::lastError() const
{
    return d->lastError;
}

} // namespace LibSyndication
