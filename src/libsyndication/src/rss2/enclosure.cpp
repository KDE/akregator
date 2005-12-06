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

#include "../shared.h"

#include "enclosure.h"

#include <qdom.h>
#include <qstring.h>

#include <kstaticdeleter.h>

namespace LibSyndication {
namespace RSS2 {


class Enclosure::EnclosurePrivate : public LibSyndication::Shared
{
    public:
    bool isNull;
    QString url;
    int length;
    QString type;

    bool operator==(const EnclosurePrivate &other) const
    {
        return (isNull && other.isNull) || (
                url == other.url &&
                length == other.length &&
                type == other.type);
    }

    static EnclosurePrivate* copyOnWrite(EnclosurePrivate* ep)
    {
        if (ep->count > 1)
        {
            ep->deref();
            ep = new EnclosurePrivate(*ep);
        }
        return ep;
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
    return d->isNull;
}

Enclosure Enclosure::fromXML(const QDomElement& e)
{
    Enclosure enc;

    if (e.hasAttribute(QString::fromLatin1("url")))
        enc.setURL(e.attribute(QString::fromLatin1("url")));
    if (e.hasAttribute(QString::fromLatin1("length")))
    {
        bool ok;
        int c = e.attribute(QString::fromLatin1("length")).toInt(&ok);
        enc.setLength(ok ? c : -1);
    }
    if (e.hasAttribute(QString::fromLatin1("type")))
        enc.setType(e.attribute(QString::fromLatin1("type")));
    return enc;
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

void Enclosure::setURL(const QString& url)
{
    d = EnclosurePrivate::copyOnWrite(d);
    d->isNull = false;
    d->url = url;
}

QString Enclosure::url() const
{
    return !d->isNull ? d->url : QString::null;
}

void Enclosure::setLength(int length)
{
    d = EnclosurePrivate::copyOnWrite(d);
    d->isNull = false;
    d->length = length;
}

int Enclosure::length() const
{
    return !d->isNull ? d->length : -1;
}

void Enclosure::setType(const QString& type)
{
    d = EnclosurePrivate::copyOnWrite(d);
    d->isNull = false;
    d->type = type;
}

QString Enclosure::type() const
{
    return !d->isNull ? d->type : QString::null;
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
