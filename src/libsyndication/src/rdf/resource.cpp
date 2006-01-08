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
#include "resource.h"
#include "statement.h"

#include <krandom.h>

#include <QString>

namespace LibSyndication {
namespace RDF {

class Resource::ResourcePrivate : public KShared
{
    public:
    
        QString uri;
        Model model;
        bool isAnon;
    
        bool operator==(const ResourcePrivate& other) const
        {
            return uri == other.uri && isAnon == other.isAnon;
        }
};

Resource::Resource() : d(0)
{
}

Resource::Resource(const Resource& other) : Node(other)
{
    *this = other;
}

Resource::Resource(const QString& uri, const Model& model) : d(new ResourcePrivate)
{
    if (uri.isNull())
    {
        d->uri = KRandom::randomString(10);
        d->isAnon = true;
    }
    else
    {
        d->uri = uri;
        d->isAnon = false;
    }

    d->model = model;
}

Resource::~Resource()
{
}

Resource& Resource::operator=(const Resource& other)
{
    d = other.d;
    return *this;
}

bool Resource::operator==(const Node& other) const
{
    const Resource* o2 = dynamic_cast<const Resource*>(&other);
    if (!o2)
        return false;

    if (!d || !o2->d)
        return d == o2->d;
    return *d == *(o2->d);
}

bool Resource::hasProperty(const Property& property) const
{
    return d ? d->model.resourceHasProperty(*this, property) : false;
}

Statement Resource::property(const Property& property) const
{
    return d ? d->model.resourceProperty(*this, property) : Statement();
}

Resource* Resource::clone() const
{
    return new Resource(*this);
}

bool Resource::isNull() const
{
    return d == (ResourcePrivate*)0;
}

Model Resource::model() const
{
    return d ? d->model : Model();
}

bool Resource::isResource() const
{
    return true;
}

bool Resource::isLiteral() const
{
    return false;
}

bool Resource::isAnon() const
{
    return d ? d->isAnon : false;
}

bool Resource::isSequence() const
{
    return false;
}

QString Resource::uri() const
{
    return d ? d->uri : QString::null;
}

} // namespace RDF
} // namespace LibSyndication
