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

#include "dublincore.h"
#include "dublincorevocab.h"
#include "property.h"
#include "statement.h"

#include <QString>

namespace LibSyndication {
namespace RDF {
    
class DublinCore::DublinCorePrivate : public KShared
{
    public:
    
    Resource resource;

    bool operator==(const DublinCorePrivate& other) const
    {
        return resource == other.resource;
    }

};

DublinCore::DublinCore() : d(new DublinCorePrivate)
{
}

DublinCore::DublinCore(const DublinCore& other) : d(0)
{
    *this = other;
}

DublinCore::DublinCore(const Resource& resource) : d(new DublinCorePrivate)
{
    d->resource = resource;
}

DublinCore::~DublinCore()
{
}

        
DublinCore& DublinCore::operator=(const DublinCore& other)
{
    d = other.d;
    return *this;
}

bool DublinCore::operator==(const DublinCore& other) const
{
    return *d == *(other.d);
}


QString DublinCore::contributor() const
{
    return d->resource.property(DublinCoreVocab::self()->contributor()).asString();
}

QString DublinCore::coverage() const
{
    return d->resource.property(DublinCoreVocab::self()->coverage()).asString();
}

QString DublinCore::creator() const
{
    return d->resource.property(DublinCoreVocab::self()->creator()).asString();
}

time_t DublinCore::date() const
{
    //TODO: parse date
    return 0;
}

QString DublinCore::description() const
{
    return d->resource.property(DublinCoreVocab::self()->description()).asString();
}

QString DublinCore::format() const
{
    return d->resource.property(DublinCoreVocab::self()->format()).asString();
}

QString DublinCore::identifier() const
{
    return d->resource.property(DublinCoreVocab::self()->identifier()).asString();
}

QString DublinCore::language() const
{
    return d->resource.property(DublinCoreVocab::self()->language()).asString();
}

QString DublinCore::publisher() const
{
    return d->resource.property(DublinCoreVocab::self()->publisher()).asString();
}

QString DublinCore::relation() const
{
    return d->resource.property(DublinCoreVocab::self()->relation()).asString();
}

QString DublinCore::rights() const
{
    return d->resource.property(DublinCoreVocab::self()->rights()).asString();
}

QString DublinCore::source() const
{
    return d->resource.property(DublinCoreVocab::self()->source()).asString();
}

QString DublinCore::subject() const
{
    return d->resource.property(DublinCoreVocab::self()->subject()).asString();
}

QString DublinCore::title() const
{
    return d->resource.property(DublinCoreVocab::self()->title()).asString();
}

QString DublinCore::type() const
{
    return d->resource.property(DublinCoreVocab::self()->type()).asString();
}

QString DublinCore::debugInfo() const
{
    QString info;
    if (!contributor().isNull())
        info += QString("dc:contributor: #%1#\n").arg(contributor());
    if (!coverage().isNull())
        info += QString("dc:coverage: #%1#\n").arg(coverage());
    if (!creator().isNull())
        info += QString("dc:creator: #%1#\n").arg(creator());
    info += QString("dc:date: #%1#\n").arg(QString::number(date()));
    if (!description().isNull())
        info += QString("dc:description: #%1#\n").arg(description());
    if (!format().isNull())
        info += QString("dc:format: #%1#\n").arg(format());
    if (!identifier().isNull())
        info += QString("dc:identifier: #%1#\n").arg(identifier());
    if (!language().isNull())
        info += QString("dc:language: #%1#\n").arg(language());
    if (!publisher().isNull())
        info += QString("dc:publisher: #%1#\n").arg(publisher());
    if (!relation().isNull())
        info += QString("dc:relation: #%1#\n").arg(relation());
    if (!rights().isNull())
        info += QString("dc:rights: #%1#\n").arg(rights());
    if (!source().isNull())
        info += QString("dc:source: #%1#\n").arg(source());
    if (!subject().isNull())
        info += QString("dc:subject: #%1#\n").arg(subject());
    if (!title().isNull())
        info += QString("dc:title: #%1#\n").arg(title());
    if (!type().isNull())
        info += QString("dc:type: #%1#\n").arg(type());
    return info;
}

} // namespace RDF
} // namespace LibSyndication

