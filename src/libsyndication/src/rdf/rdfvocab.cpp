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

#include "model.h"
#include "property.h"
#include "rdfvocab.h"

#include <kstaticdeleter.h>

#include <QString>

namespace LibSyndication {
namespace RDF {

class RDFVocab::RDFVocabPrivate
{
    public:
    
        QString namespaceURI;
        ResourcePtr seq;
        PropertyPtr type;
        PropertyPtr li;
};

static KStaticDeleter<RDFVocab> rdfvocabsd;

RDFVocab* RDFVocab::m_self = 0;

RDFVocab* RDFVocab::self()
{
    if (m_self == 0)
        rdfvocabsd.setObject(m_self, new RDFVocab);
    return m_self;
}

RDFVocab::RDFVocab() : d(new RDFVocabPrivate)
{
    QString ns = QString::fromLatin1("http://www.w3.org/1999/02/22-rdf-syntax-ns#");

    d->namespaceURI = ns;

    d->seq = new Resource(ns + QString::fromLatin1("Seq"));
    d->type = new Property(ns + QString::fromLatin1("type"));
    d->li = new Property(ns + QString::fromLatin1("li"));
}

RDFVocab::~RDFVocab()
{
    delete d;
    d = 0;
}

ResourcePtr RDFVocab::seq()
{
    return d->seq;
}

PropertyPtr RDFVocab::type()
{
    return d->type;
}

PropertyPtr RDFVocab::li()
{
    return d->li;
}

QString RDFVocab::namespaceURI()
{
    return d->namespaceURI;
}

} // namespace RDF
} // namespace LibSyndication
