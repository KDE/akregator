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
#include "resource.h"
#include "resourcewrapper.h"


namespace LibSyndication {
namespace RDF {

class ResourceWrapper::ResourceWrapperPrivate : public KShared
{
    public:
        
    ResourcePtr resource;
};

ResourceWrapper::ResourceWrapper() : d(new ResourceWrapperPrivate)
{
    d->resource = new Resource();
}

ResourceWrapper::ResourceWrapper(const ResourceWrapper& other)
{
    *this = other;
}

ResourceWrapper::ResourceWrapper(ResourcePtr resource) : d(new ResourceWrapperPrivate)
{
    d->resource = resource;
}

ResourceWrapper::~ResourceWrapper()
{
}

ResourceWrapper& ResourceWrapper::operator=(const ResourceWrapper& other)
{
    d = other.d;
    return *this;
}

bool ResourceWrapper::operator==(const ResourceWrapper& other) const
{
    return *(d->resource) == *(other.d->resource);
}

bool ResourceWrapper::isNull() const
{
    return d->resource->isNull();
}

ResourcePtr ResourceWrapper::resource() const
{
    return d->resource;
}

} // namespace RDF
} // namespace LibSyndication
