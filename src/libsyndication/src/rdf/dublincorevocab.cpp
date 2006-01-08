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

#include "dublincorevocab.h"
#include "property.h"

#include <kstaticdeleter.h>

#include <QString>

namespace LibSyndication {
namespace RDF {

class DublinCoreVocab::DublinCoreVocabPrivate
{
    public:
        
    QString namespaceURI;
    Property contributor;
    Property coverage;
    Property creator;
    Property date;
    Property description;
    Property format;
    Property identifier;
    Property language;
    Property publisher;
    Property relation;
    Property rights;
    Property source;
    Property subject;
    Property title;
    Property type;
};

DublinCoreVocab::DublinCoreVocab() : d(new DublinCoreVocabPrivate)
{
    QString ns = QString::fromLatin1("http://purl.org/dc/elements/1.1/");
    
    d->namespaceURI = ns;
    
    d->contributor = Property(ns + QString::fromLatin1("contributor"));
    d->coverage = Property(ns + QString::fromLatin1("coverage"));
    d->creator = Property(ns + QString::fromLatin1("creator"));
    d->date = Property(ns + QString::fromLatin1("date"));
    d->description = Property(ns + QString::fromLatin1("description"));
    d->format = Property(ns + QString::fromLatin1("format"));
    d->identifier = Property(ns + QString::fromLatin1("identifier"));
    d->language = Property(ns + QString::fromLatin1("language"));
    d->publisher = Property(ns + QString::fromLatin1("publisher"));
    d->relation = Property(ns + QString::fromLatin1("relation"));
    d->rights = Property(ns + QString::fromLatin1("rights"));
    d->source = Property(ns + QString::fromLatin1("source"));
    d->subject = Property(ns + QString::fromLatin1("subject"));
    d->title = Property(ns + QString::fromLatin1("title"));
    d->type = Property(ns + QString::fromLatin1("type"));
    
}

DublinCoreVocab::~DublinCoreVocab()
{
    delete d;
    d = 0;
}

static KStaticDeleter<DublinCoreVocab> dublincoresd;

DublinCoreVocab* DublinCoreVocab::m_self = 0;

DublinCoreVocab* DublinCoreVocab::self()
{
    if (m_self == 0)
        dublincoresd.setObject(m_self, new DublinCoreVocab);
    return m_self;
}
        
const QString& DublinCoreVocab::namespaceURI() const
{
    return d->namespaceURI;
}

const Property& DublinCoreVocab::contributor() const
{
    return d->contributor;
}

const Property& DublinCoreVocab::creator() const
{
    return d->creator;
}

const Property& DublinCoreVocab::coverage() const
{
    return d->coverage;
}

const Property& DublinCoreVocab::date() const
{
    return d->date;
}

const Property& DublinCoreVocab::description() const
{
    return d->description;
}

const Property& DublinCoreVocab::format() const
{
    return d->format;
}

const Property& DublinCoreVocab::identifier() const
{
    return d->identifier;
}

const Property& DublinCoreVocab::language() const
{
    return d->language;
}

const Property& DublinCoreVocab::publisher() const
{
    return d->publisher;
}

const Property& DublinCoreVocab::relation() const
{
    return d->relation;
}

const Property& DublinCoreVocab::rights() const
{
    return d->rights;
}

const Property& DublinCoreVocab::source() const
{
    return d->source;
}

const Property& DublinCoreVocab::subject() const
{
    return d->subject;
}

const Property& DublinCoreVocab::title() const
{
    return d->title;
}

const Property& DublinCoreVocab::type() const
{
    return d->type;
}

} // namespace RDF
} // namespace LibSyndication
