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

#include "content.h"
#include "contentvocab.h"
#include "property.h"
#include "statement.h"

#include <QString>

namespace LibSyndication {
namespace RDF {
    
class Content::ContentPrivate : public KShared
{
    public:
    
    ResourcePtr resource;

    bool operator==(const ContentPrivate& other) const
    {
        return *resource == *(other.resource);
    }

};

Content::Content() : d(new ContentPrivate)
{
}

Content::Content(const Content& other) : d(0)
{
    *this = other;
}

Content::Content(ResourcePtr resource) : d(new ContentPrivate)
{
    d->resource = resource;
}

Content::~Content()
{
}

        
Content& Content::operator=(const Content& other)
{
    d = other.d;
    return *this;
}

bool Content::operator==(const Content& other) const
{
    return *d == *(other.d);
}


QString Content::encoded() const
{
    // TODO: "normalize" content (make HTML out of it etc.)
    return d->resource->property(ContentVocab::self()->encoded())->asString();
}

QString Content::debugInfo() const
{
    QString info;
    if (!encoded().isNull())
        info += QString("content:encoded: #%1#\n").arg(encoded());
    return info;
}

} // namespace RDF
} // namespace LibSyndication

