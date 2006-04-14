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
#include <QStringList>

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
    QString ns = QString::fromUtf8("http://purl.org/rss/1.0/");
    
    d->namespaceURI = ns;
    
    d->title = new Property(ns + QString::fromUtf8("title"));
    d->link = new Property(ns + QString::fromUtf8("link"));
    d->description = new Property(ns + QString::fromUtf8("description"));
    d->name = new Property(ns + QString::fromUtf8("name"));
    d->url = new Property(ns + QString::fromUtf8("url"));
    d->image = new Property(ns + QString::fromUtf8("image"));
    d->textinput = new Property(ns + QString::fromUtf8("textinput"));
    d->items = new Property(ns + QString::fromUtf8("items"));
    d->channel = new Resource(ns + QString::fromUtf8("channel"));
}

RSSVocab::~RSSVocab()
{
    delete d;
    d = 0;
}

/** @internal */
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

class RSS09Vocab::RSS09VocabPrivate
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
        ResourcePtr item;
        PropertyPtr textinput;
        QStringList properties;
        QStringList classes;
};

RSS09Vocab::RSS09Vocab() : d(new RSS09VocabPrivate)
{
    QString ns = QString::fromUtf8("http://my.netscape.com/rdf/simple/0.9/");
    
    d->namespaceURI = ns;
    
    d->title = new Property(ns + QString::fromUtf8("title"));
    d->properties.append(d->title->uri());
    d->link = new Property(ns + QString::fromUtf8("link"));
    d->properties.append(d->link->uri());
    d->description = new Property(ns + QString::fromUtf8("description"));
    d->properties.append(d->description->uri());
    d->name = new Property(ns + QString::fromUtf8("name"));
    d->properties.append(d->name->uri());
    d->url = new Property(ns + QString::fromUtf8("url"));
    d->properties.append(d->url->uri());
    d->image = new Property(ns + QString::fromUtf8("image"));
    d->properties.append(d->image->uri());
    d->textinput = new Property(ns + QString::fromUtf8("textinput"));
    d->properties.append(d->textinput->uri());
    d->item = new Resource(ns + QString::fromUtf8("item"));
    d->classes.append(d->item->uri());
    d->channel = new Resource(ns + QString::fromUtf8("channel"));
    d->classes.append(d->channel->uri());
}

RSS09Vocab::~RSS09Vocab()
{
    delete d;
    d = 0;
}

/** @internal */
static KStaticDeleter<RSS09Vocab> rss09vocabsd;

RSS09Vocab* RSS09Vocab::m_self = 0;

RSS09Vocab* RSS09Vocab::self()
{
    if (m_self == 0)
        rss09vocabsd.setObject(m_self, new RSS09Vocab);
    return m_self;
}
        
const QString& RSS09Vocab::namespaceURI() const
{
    return d->namespaceURI;
}

PropertyPtr RSS09Vocab::title() const
{
    return d->title;
}

PropertyPtr RSS09Vocab::description() const
{
    return d->description;
}

PropertyPtr RSS09Vocab::link() const
{
    return d->link;
}

PropertyPtr RSS09Vocab::name() const
{
    return d->name;
}

PropertyPtr RSS09Vocab::url() const
{
    return d->url;
}

PropertyPtr RSS09Vocab::image() const
{
    return d->image;
}

PropertyPtr RSS09Vocab::textinput() const
{
    return d->textinput;
}

ResourcePtr RSS09Vocab::item() const
{
    return d->item;
}

ResourcePtr RSS09Vocab::channel() const
{
    return d->channel;
}

QStringList RSS09Vocab::classes() const
{
    return d->classes;
}

QStringList RSS09Vocab::properties() const
{
    return d->properties;
}

} // namespace RDF
} // namespace LibSyndication
