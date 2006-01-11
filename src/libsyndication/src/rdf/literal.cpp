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

#include "literal.h"
#include "nodevisitor.h"

#include <QString>

namespace LibSyndication {
namespace RDF {

class Literal::LiteralPrivate : public KShared
{
    public:
    
        QString text;
        unsigned int id;

        bool operator==(const LiteralPrivate& other) const
        {
            return text == other.text;
        }
};

Literal::Literal() : d(0)
{
}

Literal::Literal(const Literal& other) : Node(other)
{
    d = other.d;
}

Literal* Literal::clone() const
{
    return new Literal(*this);
}
    
void Literal::accept(NodeVisitor* visitor, NodePtr ptr)
{
    LiteralPtr lptr = LiteralPtr::staticCast(ptr);
    if (!visitor->visitLiteral(lptr))
        Node::accept(visitor, ptr);
}

Literal::Literal(const QString& text) : d(new LiteralPrivate)
{
    d->text = text;
    d->id = idCounter++;
}

Literal::~Literal()
{
}

Literal& Literal::operator=(const Literal& other)
{
    d = other.d;
    return *this;
}

bool Literal::operator==(const Node& other) const
{
    const Literal* o2 = dynamic_cast<const Literal*>(&other);
    if (!o2)
        return false;

    if (!d || !o2->d)
        return d == o2->d;

    return *d == *(o2->d);
}

bool Literal::isNull() const
{
    return d == (LiteralPrivate*)0;
}

unsigned int Literal::id() const
{
    return d ? d->id : 0;
}

bool Literal::isResource() const
{
    return false;
}

bool Literal::isLiteral() const
{
    return true;
}

bool Literal::isAnon() const
{
    return false;
}

bool Literal::isSequence() const
{
    return false;
}

QString Literal::text() const
{
    return d ? d->text : QString::null;
}

void Literal::setModel(const Model& /*model*/)
{
}

} // namespace RDF
} // namespace LibSyndication
