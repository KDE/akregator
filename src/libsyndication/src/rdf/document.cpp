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

#include "../documentvisitor.h"

#include "content.h"
#include "document.h"
#include "dublincore.h"
#include "image.h"
#include "item.h"
#include "rssvocab.h"
#include "sequence.h"
#include "statement.h"
#include "syndication.h"
#include "textinput.h"

#include <QList>
#include <QString>

namespace LibSyndication {
namespace RDF {


Document::Document(const QString& uri, const Model& model) 
    : LibSyndication::Document(), Resource(uri, model)
{
}

Document::Document(const Resource& resource) : Resource(resource)
{
}

Document::~Document()
{
}

bool Document::accept(DocumentVisitor* visitor)
{
    return visitor->visit(this);
}

QString Document::title() const
{
    // TODO: handle HTML, encoding etc.
    return property(RSSVocab::self()->title()).asString();
}

QString Document::description() const
{
    // TODO: handle HTML, encoding etc.
    return property(RSSVocab::self()->description()).asString();
}

QString Document::link() const
{
    return property(RSSVocab::self()->link()).asString();
}

DublinCore Document::dc() const
{
    return DublinCore(*this);
}

Syndication Document::syn() const
{
    return Syndication(*this);
}

QList<Item> Document::items() const
{
    QList<Item> list;
    if (hasProperty(RSSVocab::self()->items()))
        return list;
    
    Node* n = property(RSSVocab::self()->items()).object();
    if (n->isSequence())
    {
        Sequence* seq = dynamic_cast<Sequence*>(n);
        if (seq)
        {
            QList<Node*> items = seq->items();
            QList<Node*>::ConstIterator it = items.begin();
            QList<Node*>::ConstIterator end = items.end();
            
            for ( ; it != end; ++it)
            {
                if ((*it)->isResource())
                {
                    Item item(*(static_cast<Resource*>(*it)));
                    list.append(item);
                }
            }
        }
    }
    return list;
}

Image Document::image() const
{
    // TODO
    /* 
    if (hasProperty(RSSVocab::self()->image()))
    {
        
    }
    */
    
    return Image();
}

TextInput Document::textInput() const
{
    // TODO
    /* 
    if (hasProperty(RSSVocab::self()->textInput()))
    {
        
}
    */
    
    return TextInput();
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
    if (!img.isNull())
        info += img.debugInfo();
    TextInput input = textInput();
    if (!input.isNull())
        info += input.debugInfo();
    info += "### Document end ################\n";
    return info;
}

} // namespace RDF
} // namespace LibSyndication
