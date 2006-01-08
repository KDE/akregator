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
#include "statement.h"
#include "syndication.h"
#include "syndicationvocab.h"

#include <QString>

namespace LibSyndication {
namespace RDF {
    
class Syndication::SyndicationPrivate : public KShared
{
    public:
    
    Resource resource;

    bool operator==(const SyndicationPrivate& other) const
    {
        return resource == other.resource;
    }

};

Syndication::Syndication() : d(new SyndicationPrivate)
{
}

Syndication::Syndication(const Syndication& other) : d(0)
{
    *this = other;
}

Syndication::Syndication(const Resource& resource) : d(new SyndicationPrivate)
{
    d->resource = resource;
}

Syndication::~Syndication()
{
}

        
Syndication& Syndication::operator=(const Syndication& other)
{
    d = other.d;
    return *this;
}

bool Syndication::operator==(const Syndication& other) const
{
    return *d == *(other.d);
}


Syndication::Period Syndication::updatePeriod() const
{
    // TODO
    return hourly;
}

int Syndication::updateFrequency() const
{
    // TODO
    return 0;
}

time_t Syndication::updateBase() const
{
    // TODO
    return 0;
}

QString Syndication::debugInfo() const
{
    QString info = "TODO";
    return info;
}

} // namespace RDF
} // namespace LibSyndication
