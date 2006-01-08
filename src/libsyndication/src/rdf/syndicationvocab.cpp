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

#include "property.h"
#include "syndicationvocab.h"

#include <kstaticdeleter.h>

#include <QString>

namespace LibSyndication {
namespace RDF {

class SyndicationVocab::SyndicationVocabPrivate
{
    public:
        
    QString namespaceURI;
    Property updatePeriod;
    Property updateFrequency;
    Property updateBase;
};

SyndicationVocab::SyndicationVocab() : d(new SyndicationVocabPrivate)
{
    QString ns = QString::fromLatin1("http://purl.org/rss/1.0/modules/syndication/");
    
    d->namespaceURI = ns;
    
    d->updatePeriod = Property(ns + QString::fromLatin1("updatePeriod"));
    d->updateFrequency = Property(ns + QString::fromLatin1("updateFrequency"));
    d->updateBase = Property(ns + QString::fromLatin1("updateBase"));
    
}

SyndicationVocab::~SyndicationVocab()
{
    delete d;
    d = 0;
}

static KStaticDeleter<SyndicationVocab> syndicationvocabsd;

SyndicationVocab* SyndicationVocab::m_self = 0;

SyndicationVocab* SyndicationVocab::self()
{
    if (m_self == 0)
        syndicationvocabsd.setObject(m_self, new SyndicationVocab);
    return m_self;
}
        
const QString& SyndicationVocab::namespaceURI() const
{
    return d->namespaceURI;
}

const Property& SyndicationVocab::updatePeriod() const
{
    return d->updatePeriod;
}

const Property& SyndicationVocab::updateFrequency() const
{
    return d->updateFrequency;
}

const Property& SyndicationVocab::updateBase() const
{
    return d->updateBase;
}

} // namespace RDF
} // namespace LibSyndication
