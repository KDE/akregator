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
#include "rssvocab.h"

#include <kstaticdeleter.h>

#include <QString>

namespace LibSyndication {
namespace RDF {

class RSSVocab::RSSVocabPrivate
{
    public:
        
    QString namespaceURI;
    Property title;
    Property link;
    Property description;
    Property name;
    Property url;
    Property image;
    Property channel;
    Property items;
    Property textinput;
};

RSSVocab::RSSVocab() : d(new RSSVocabPrivate)
{
    QString ns = QString::fromLatin1("http://purl.org/rss/1.0/");
    
    d->namespaceURI = ns;
    
    d->title = Property(ns + QString::fromLatin1("title"));
    d->link = Property(ns + QString::fromLatin1("link"));
    d->description = Property(ns + QString::fromLatin1("description"));
    d->name = Property(ns + QString::fromLatin1("name"));
    d->url = Property(ns + QString::fromLatin1("url"));
    d->image = Property(ns + QString::fromLatin1("image"));
    d->textinput = Property(ns + QString::fromLatin1("textinput"));
    d->items = Property(ns + QString::fromLatin1("items"));
    d->channel = Property(ns + QString::fromLatin1("channel"));
}

RSSVocab::~RSSVocab()
{
    delete d;
    d = 0;
}

static KStaticDeleter<RSSVocab> rssvocabsd;

RSSVocab* RSSVocab::m_self = 0;

RSSVocab* RSSVocab::self()
{
    if (m_self == 0)
        rssvocabsd.setObject(m_self, new RSSVocab);
    return m_self;
}
        
const QString& RSSVocab::namespaceURI() const
{
    return d->namespaceURI;
}

const Property& RSSVocab::title() const
{
    return d->title;
}

const Property& RSSVocab::description() const
{
    return d->description;
}

const Property& RSSVocab::link() const
{
    return d->link;
}

const Property& RSSVocab::name() const
{
    return d->name;
}

const Property& RSSVocab::url() const
{
    return d->url;
}

const Property& RSSVocab::image() const
{
    return d->image;
}

const Property& RSSVocab::textinput() const
{
    return d->textinput;
}


const Property& RSSVocab::items() const
{
    return d->items;
}

const Property& RSSVocab::channel() const
{
    return d->channel;
}

} // namespace RDF
} // namespace LibSyndication
