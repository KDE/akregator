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
    PropertyPtr title;
    PropertyPtr link;
    PropertyPtr description;
    PropertyPtr name;
    PropertyPtr url;
    PropertyPtr image;
    ResourcePtr channel;
    PropertyPtr items;
    PropertyPtr textinput;
};

RSSVocab::RSSVocab() : d(new RSSVocabPrivate)
{
    QString ns = QString::fromLatin1("http://purl.org/rss/1.0/");
    
    d->namespaceURI = ns;
    
    d->title = new Property(ns + QString::fromLatin1("title"));
    d->link = new Property(ns + QString::fromLatin1("link"));
    d->description = new Property(ns + QString::fromLatin1("description"));
    d->name = new Property(ns + QString::fromLatin1("name"));
    d->url = new Property(ns + QString::fromLatin1("url"));
    d->image = new Property(ns + QString::fromLatin1("image"));
    d->textinput = new Property(ns + QString::fromLatin1("textinput"));
    d->items = new Property(ns + QString::fromLatin1("items"));
    d->channel = new Resource(ns + QString::fromLatin1("channel"));
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

PropertyPtr RSSVocab::title() const
{
    return d->title;
}

PropertyPtr RSSVocab::description() const
{
    return d->description;
}

PropertyPtr RSSVocab::link() const
{
    return d->link;
}

PropertyPtr RSSVocab::name() const
{
    return d->name;
}

PropertyPtr RSSVocab::url() const
{
    return d->url;
}

PropertyPtr RSSVocab::image() const
{
    return d->image;
}

PropertyPtr RSSVocab::textinput() const
{
    return d->textinput;
}


PropertyPtr RSSVocab::items() const
{
    return d->items;
}

ResourcePtr RSSVocab::channel() const
{
    return d->channel;
}

} // namespace RDF
} // namespace LibSyndication
