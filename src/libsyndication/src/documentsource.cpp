/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>
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

#include "documentsource.h"

#include <QByteArray>
#include <qdom.h>
#include <qxml.h>

namespace LibSyndication {

class DocumentSource::DocumentSourcePrivate : public KShared
{
    public:
    QByteArray array;
    mutable QDomDocument domDoc;
    mutable bool parsed;
};

DocumentSource::DocumentSource() : d(new DocumentSourcePrivate)
{
    d->parsed = true;
}


DocumentSource::DocumentSource(const QByteArray& source) : d(new DocumentSourcePrivate)
{
    d->array = source;
    d->parsed = false;
}

DocumentSource::DocumentSource(const DocumentSource& other) : d(0)
{
     *this = other;
}

DocumentSource::~DocumentSource()
{
}

DocumentSource& DocumentSource::operator=(const DocumentSource& other)
{
    d = other.d;
    return *this;
}

QByteArray DocumentSource::asByteArray() const
{
    return d->array;
}

QDomDocument DocumentSource::asDomDocument() const
{
    if (!d->parsed)
    {
        QXmlInputSource source;
        source.setData(d->array);
        
        QXmlSimpleReader reader;
        reader.setFeature("http://xml.org/sax/features/namespaces", true);
        QDomDocument doc;
        
        d->domDoc.setContent(&source, &reader);
        d->parsed = true;
    }

    return d->domDoc;
}

} // namespace LibSyndication
