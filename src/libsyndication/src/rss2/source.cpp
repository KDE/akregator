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

#include "source.h"
#include "tools.h"

#include <qdom.h>
#include <qstring.h>

#include <kstaticdeleter.h>

namespace LibSyndication {
namespace RSS2 {

struct Source::SourcePrivate : public Shared
{
    bool isNull;
    QString source;
    QString url;

    bool operator==(const SourcePrivate &other) const
    {
        return (isNull && other.isNull) || (source == other.source && url == other.url);
    }

    static SourcePrivate* copyOnWrite(SourcePrivate* ep)
    {
        if (ep->count > 1)
        {
            ep->deref();
            ep = new SourcePrivate(*ep);
        }
        return ep;
    }
};

Source* Source::m_null = 0;
static KStaticDeleter<Source> sourcesd;

const Source& Source::null()
{
    if (m_null == 0)
        sourcesd.setObject(m_null, new Source);

    return *m_null;
}

Source Source::fromXML(const QDomElement& e)
{
    Source obj;
    if (e.hasAttribute(QString::fromLatin1("url")))
        obj.setUrl(e.attribute(QString::fromLatin1("url")));
    obj.setSource(e.text());

    return obj;
}

bool Source::isNull() const
{
    return d->isNull;
}

Source::Source() : d(new SourcePrivate)
{
    d->isNull = true;
}

Source::Source(const Source& other) : d(0)
{
     *this = other;
}

Source::Source(const QString& source, const QString& url) : d(new SourcePrivate)
{
    d->isNull = false;
    d->source = source;
    d->url = url;
}

Source::~Source()
{
    if (d->deref())
    {
        delete d;
        d = 0;
    }
}

Source& Source::operator=(const Source& other)
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

bool Source::operator==(const Source &other) const
{
    return *d == *other.d;
}


void Source::setSource(const QString& source)
{
    d = SourcePrivate::copyOnWrite(d);
    d->isNull = false;
    d->source = source;
}

QString Source::source() const
{
    return !d->isNull ? d->source : QString::null;
}

void Source::setUrl(const QString& url)
{
    d = SourcePrivate::copyOnWrite(d);
    d->isNull = false;
    d->url = url;
}

QString Source::url() const
{
    return !d->isNull ? d->url : QString::null;
}

QString Source::debugInfo() const
{
    QString info;
    info += "### Source: ###################\n";
    info += "source: #" + source() + "#\n";
    info += "url: #" + url() + "#\n";
    info += "### Source end ################\n";
    return info;
}


} // namespace RSS2
} // namespace LibSyndication

