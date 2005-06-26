/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "enclosure.h"
#include "tools_p.h"

#include <qdom.h>
#include <qstring.h>

#include <kdebug.h>

namespace RSS
{


class Enclosure::EnclosurePrivate : public Shared
{
    public:

    bool isNull;
    QString url;
    int length;
    QString type;

    bool operator==(const EnclosurePrivate &other) const
    {
        return ( isNull == other.isNull || (url == other.url &&
                length == other.length &&
                type == other.type));
    }
};


Enclosure Enclosure::fromXML(const QDomElement& e)
{
    QString url, type;
    int length = -1;

    if (e.hasAttribute(QString::fromLatin1("url")))
        url = e.attribute(QString::fromLatin1("url"));
    
    if (e.hasAttribute(QString::fromLatin1("length")))
    {
        bool ok;
        int c = e.attribute(QString::fromLatin1("length")).toInt(&ok);
        length = ok ? c : -1;
    }
    if (e.hasAttribute(QString::fromLatin1("type")))
        type = e.attribute(QString::fromLatin1("type"));

    return Enclosure(url, length, type);
}

QDomElement Enclosure::toXML(QDomDocument document) const
{
    QDomElement e = document.createElement(QString::fromLatin1("enclosure"));
    if (!d->url.isNull())
        e.setAttribute(QString::fromLatin1("url"), d->url);
    if (d->length != -1)
        e.setAttribute(QString::fromLatin1("length"), QString::number(d->length));
    if (!d->type.isNull())
        e.setAttribute(QString::fromLatin1("type"), d->type);

    return e;
}

Enclosure::Enclosure() : d(new EnclosurePrivate)
{
    d->isNull = true;
    d->length = -1;
}

Enclosure::Enclosure(const Enclosure& other) : d(0)
{
     *this = other;
}

Enclosure::Enclosure(const QString& url, int length, const QString& type) : d(new EnclosurePrivate)
{
    d->isNull = false;
    d->url = url;
    d->length = length;
    d->type = type;
}

Enclosure::~Enclosure()
{
    if (d->deref())
    {
        delete d;
        d = 0;
    }
}

Enclosure& Enclosure::operator=(const Enclosure& other)
{
    if (d != other.d)
    {
        other.d->ref();
        if (d && d->deref())
            delete d;
        d = other.d;
    }
    return *this;
}

bool Enclosure::operator==(const Enclosure &other) const
{
    return *d == *other.d;
}

bool Enclosure::isNull() const
{
    return d->isNull;
}

QString Enclosure::url() const
{
    return d->url;
}

int Enclosure::length() const
{
    return d->length;
}

QString Enclosure::type() const
{
    return d->type;
}


} // namespace RSS
