/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
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
