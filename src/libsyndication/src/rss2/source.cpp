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

#include "source.h"
#include "tools.h"

#include <qdom.h>
#include <qstring.h>

#include <kstaticdeleter.h>

namespace LibSyndication {
namespace RSS2 {

class Source::SourcePrivate : public KShared
{
    public:

    QString source;
    QString url;

    bool operator==(const SourcePrivate &other) const
    {
        return (source == other.source && url == other.url);
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
    QString url = e.attribute(QString::fromLatin1("url"));
    QString source = e.text();

    return Source(source, url);
}

bool Source::isNull() const
{
    return !d;
}

Source::Source() : d(0)
{
}

Source::Source(const Source& other) : d(0)
{
     *this = other;
}

Source::Source(const QString& source, const QString& url) : d(new SourcePrivate)
{
    d->source = source;
    d->url = url;
}

Source::~Source()
{
}

Source& Source::operator=(const Source& other)
{
    d = other.d;
    return *this;
}

bool Source::operator==(const Source &other) const
{
    if (!d || !other.d)
        return d == other.d;
    return *d == *other.d;
}

QString Source::source() const
{
    return d ? d->source : QString::null;
}

QString Source::url() const
{
    return d ? d->url : QString::null;
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

