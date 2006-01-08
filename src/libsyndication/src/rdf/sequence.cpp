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

#include "node.h"
#include "sequence.h"

#include <QList>

namespace LibSyndication {
namespace RDF {

class Sequence::SequencePrivate : public KShared
{
    public:
    
    QList<Node*> items;

    ~SequencePrivate()
    {
        QList<Node*>::ConstIterator it = items.begin();
        QList<Node*>::ConstIterator end = items.end();
    
        for ( ; it != end; ++it)
            delete *it;

    }
        
};

Sequence::Sequence() : Resource(), d(0)
{
}

Sequence::Sequence(const QString& uri, const Model& model) 
    : Resource(uri, model), d(new SequencePrivate)
{
}

Sequence::Sequence(const Sequence& other) : Resource(other)
{
    *this = other;
}

Sequence::~Sequence()
{
}

Sequence& Sequence::operator=(const Sequence& other)
{
    d = other.d;
    return *this;
}

void Sequence::append(const Node& node)
{
    if (d)
        d->items.append(node.clone());
}

QList<Node*> Sequence::items() const
{
    return d ? d->items : QList<Node*>();
}

bool Sequence::isSequence() const
{
    return true;
}

} // namespace RDF
} // namespace LibSyndication
