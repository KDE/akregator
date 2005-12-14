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

#include "enclosure.h"

#include <qdom.h>
#include <qstring.h>

#include <kstaticdeleter.h>

namespace LibSyndication {
namespace RSS2 {


class Enclosure::EnclosurePrivate : public KShared
{
    public:

    QString url;
    int length;
    QString type;

    bool operator==(const EnclosurePrivate &other) const
    {
        return (url == other.url &&
                length == other.length &&
                type == other.type);
    }

};


Enclosure* Enclosure::m_null = 0;
static KStaticDeleter<Enclosure> enclosuresd;

const Enclosure& Enclosure::null()
{
    if (m_null == 0)
        enclosuresd.setObject(m_null, new Enclosure);

    return *m_null;
}


bool Enclosure::isNull() const
{
    return !d;
}

Enclosure Enclosure::fromXML(const QDomElement& e)
{
    QString url = e.attribute(QString::fromLatin1("url"));

    int length = -1;

    if (e.hasAttribute(QString::fromLatin1("length")))
    {
        bool ok;
        int c = e.attribute(QString::fromLatin1("length")).toInt(&ok);
        length = ok ? c : -1;
    }
   
    QString type = e.attribute(QString::fromLatin1("type"));

    return Enclosure(url, length, type);
}

Enclosure::Enclosure() : d(0)
{
}

Enclosure::Enclosure(const Enclosure& other) : d(0)
{
     *this = other;
}

Enclosure::Enclosure(const QString& url, int length, const QString& type) : d(new EnclosurePrivate)
{
    d->url = url;
    d->length = length;
    d->type = type;
}

Enclosure::~Enclosure()
{
}

Enclosure& Enclosure::operator=(const Enclosure& other)
{
    d = other.d;
    return *this;
}

bool Enclosure::operator==(const Enclosure &other) const
{
    if (!d || !other.d)
        return d == other.d;
    return *d == *other.d;
}

QString Enclosure::url() const
{
    return d ? d->url : QString::null;
}

int Enclosure::length() const
{
    return d ? d->length : -1;
}

QString Enclosure::type() const
{
    return d ? d->type : QString::null;
}

QString Enclosure::debugInfo() const
{
    QString info;
    info += "### Enclosure: ###################\n";
    info += "url: #" + url() + "#\n";
    info += "type: #" + type() + "#\n";
    info += "length: #" + QString::number(length()) + "#\n";
    info += "### Enclosure end ################\n";
    return info;
}

} // namespace RSS2
}  // namespace LibSyndication
