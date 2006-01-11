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

#include "contentvocab.h"
#include "property.h"

#include <kstaticdeleter.h>

#include <QString>

namespace LibSyndication {
namespace RDF {

class ContentVocab::ContentVocabPrivate
{
    public:
        
    QString namespaceURI;
    PropertyPtr encoded;
};

ContentVocab::ContentVocab() : d(new ContentVocabPrivate)
{
    QString ns = QString::fromLatin1("http://purl.org/rss/1.0/modules/content/");
    
    d->namespaceURI = ns;
    
    d->encoded = new Property(ns + QString::fromLatin1("encoded"));
    
}

ContentVocab::~ContentVocab()
{
    delete d;
    d = 0;
}

static KStaticDeleter<ContentVocab> contentvocabsd;

ContentVocab* ContentVocab::m_self = 0;

ContentVocab* ContentVocab::self()
{
    if (m_self == 0)
        contentvocabsd.setObject(m_self, new ContentVocab);
    return m_self;
}
        
const QString& ContentVocab::namespaceURI() const
{
    return d->namespaceURI;
}

PropertyPtr ContentVocab::encoded() const
{
    return d->encoded;
}

} // namespace RDF
} // namespace LibSyndication
