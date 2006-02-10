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
    PropertyPtr contributor;
    PropertyPtr coverage;
    PropertyPtr creator;
    PropertyPtr date;
    PropertyPtr description;
    PropertyPtr format;
    PropertyPtr identifier;
    PropertyPtr language;
    PropertyPtr publisher;
    PropertyPtr relation;
    PropertyPtr rights;
    PropertyPtr source;
    PropertyPtr subject;
    PropertyPtr title;
    PropertyPtr type;
};

DublinCoreVocab::DublinCoreVocab() : d(new DublinCoreVocabPrivate)
{
    QString ns = QString::fromUtf8("http://purl.org/dc/elements/1.1/");
    
    d->namespaceURI = ns;
    
    d->contributor = new Property(ns + QString::fromUtf8("contributor"));
    d->coverage = new Property(ns + QString::fromUtf8("coverage"));
    d->creator = new Property(ns + QString::fromUtf8("creator"));
    d->date = new Property(ns + QString::fromUtf8("date"));
    d->description = new Property(ns + QString::fromUtf8("description"));
    d->format = new Property(ns + QString::fromUtf8("format"));
    d->identifier = new Property(ns + QString::fromUtf8("identifier"));
    d->language = new Property(ns + QString::fromUtf8("language"));
    d->publisher = new Property(ns + QString::fromUtf8("publisher"));
    d->relation = new Property(ns + QString::fromUtf8("relation"));
    d->rights = new Property(ns + QString::fromUtf8("rights"));
    d->source = new Property(ns + QString::fromUtf8("source"));
    d->subject = new Property(ns + QString::fromUtf8("subject"));
    d->title = new Property(ns + QString::fromUtf8("title"));
    d->type = new Property(ns + QString::fromUtf8("type"));
    
}

DublinCoreVocab::~DublinCoreVocab()
{
    delete d;
    d = 0;
}

/** @internal */
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

PropertyPtr DublinCoreVocab::contributor() const
{
    return d->contributor;
}

PropertyPtr DublinCoreVocab::creator() const
{
    return d->creator;
}

PropertyPtr DublinCoreVocab::coverage() const
{
    return d->coverage;
}

PropertyPtr DublinCoreVocab::date() const
{
    return d->date;
}

PropertyPtr DublinCoreVocab::description() const
{
    return d->description;
}

PropertyPtr DublinCoreVocab::format() const
{
    return d->format;
}

PropertyPtr DublinCoreVocab::identifier() const
{
    return d->identifier;
}

PropertyPtr DublinCoreVocab::language() const
{
    return d->language;
}

PropertyPtr DublinCoreVocab::publisher() const
{
    return d->publisher;
}

PropertyPtr DublinCoreVocab::relation() const
{
    return d->relation;
}

PropertyPtr DublinCoreVocab::rights() const
{
    return d->rights;
}

PropertyPtr DublinCoreVocab::source() const
{
    return d->source;
}

PropertyPtr DublinCoreVocab::subject() const
{
    return d->subject;
}

PropertyPtr DublinCoreVocab::title() const
{
    return d->title;
}

PropertyPtr DublinCoreVocab::type() const
{
    return d->type;
}

} // namespace RDF
} // namespace LibSyndication
