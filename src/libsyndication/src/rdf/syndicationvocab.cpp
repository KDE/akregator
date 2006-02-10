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
    PropertyPtr updatePeriod;
    PropertyPtr updateFrequency;
    PropertyPtr updateBase;
};

SyndicationVocab::SyndicationVocab() : d(new SyndicationVocabPrivate)
{
    QString ns = QString::fromUtf8("http://purl.org/rss/1.0/modules/syndication/");
    
    d->namespaceURI = ns;
    
    d->updatePeriod = new Property(ns + QString::fromUtf8("updatePeriod"));
    d->updateFrequency = new Property(ns + QString::fromUtf8("updateFrequency"));
    d->updateBase = new Property(ns + QString::fromUtf8("updateBase"));
    
}

SyndicationVocab::~SyndicationVocab()
{
    delete d;
    d = 0;
}

/** @internal */
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

PropertyPtr SyndicationVocab::updatePeriod() const
{
    return d->updatePeriod;
}

PropertyPtr SyndicationVocab::updateFrequency() const
{
    return d->updateFrequency;
}

PropertyPtr SyndicationVocab::updateBase() const
{
    return d->updateBase;
}

} // namespace RDF
} // namespace LibSyndication
