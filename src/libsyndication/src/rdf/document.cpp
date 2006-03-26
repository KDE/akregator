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

#include <documentvisitor.h>
#include <tools.h>

#include "document.h"
#include "dublincore.h"
#include "image.h"
#include "item.h"
#include "model.h"
#include "resource.h"
#include "rssvocab.h"
#include "sequence.h"
#include "statement.h"
#include "syndication.h"
#include "textinput.h"

#include <QList>
#include <QString>

namespace LibSyndication {
namespace RDF {


Document::Document() : LibSyndication::SpecificDocument(), ResourceWrapper()
{
}

Document::Document(ResourcePtr resource) : LibSyndication::SpecificDocument(), ResourceWrapper(resource)
{
}

Document::~Document()
{
}

bool Document::accept(DocumentVisitor* visitor)
{
    return visitor->visitRDFDocument(this);
}

bool Document::isValid() const
{
    return !isNull();
}
        
QString Document::title() const
{
    QString str = resource()->property(RSSVocab::self()->title())->asString();
    return htmlize(str);

}

QString Document::description() const
{
    QString str = resource()->property(RSSVocab::self()->description())->asString();
    return htmlize(str);
}

QString Document::link() const
{
    return resource()->property(RSSVocab::self()->link())->asString();
}

DublinCore Document::dc() const
{
    return DublinCore(resource());
}

Syndication Document::syn() const
{
    return Syndication(resource());
}

QList<Item> Document::items() const
{
    QList<Item> list;
    if (!resource()->hasProperty(RSSVocab::self()->items()))
        return list;
    
    NodePtr n = resource()->property(RSSVocab::self()->items())->object();
    if (n->isSequence())
    {
        Sequence* seq = static_cast<Sequence*>(n.get());
        
        QList<NodePtr> items = seq->items();
        QList<NodePtr>::Iterator it = items.begin();
        QList<NodePtr>::Iterator end = items.end();
        
        for ( ; it != end; ++it)
        {
            if ((*it)->isResource())
            {
                // well, we need it as ResourcePtr
                // maybe this should go to the node
                // interface ResourcePtr asResource()?
                ResourcePtr ptr = resource()->model().createResource((static_cast<Resource*>((*it).get()))->uri());
                
                Item item(ptr);
                list.append(item);
            }
        }
    
    }
    return list;
}

Image Document::image() const
{
    ResourcePtr img = resource()->property(RSSVocab::self()->image())->asResource();
    
    return img ? Image(img) : Image();
}

TextInput Document::textInput() const
{
    ResourcePtr ti = resource()->property(RSSVocab::self()->textinput())->asResource();
    
    return ti ? TextInput(ti) : TextInput();
}

QString Document::debugInfo() const
{
    QString info;
    info += "### Document: ###################\n";
    info += "title: #" + title() + "#\n";
    info += "link: #" + link() + "#\n";
    info += "description: #" + description() + "#\n";
    info += dc().debugInfo();
    info += syn().debugInfo();
    Image img = image();
    if (!img.resource() == 0L)
        info += img.debugInfo();
    TextInput input = textInput();
    if (!input.isNull())
        info += input.debugInfo();

    QList<Item> itlist = items();
    QList<Item>::ConstIterator it = itlist.begin();
    QList<Item>::ConstIterator end = itlist.end();
    for ( ; it != end; ++it)
        info += (*it).debugInfo();
    
    
    info += "### Document end ################\n";
    return info;
}

} // namespace RDF
} // namespace LibSyndication
