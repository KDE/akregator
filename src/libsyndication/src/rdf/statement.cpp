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
#include "property.h"
#include "resource.h"
#include "statement.h"

#include <QString>

namespace LibSyndication {
namespace RDF {

class Statement::StatementPrivate : public KShared
{
    public:
    
        Resource* subject;
        Property* predicate;
        Node* object;

        ~StatementPrivate()
        {
            delete subject;
            delete predicate;
            delete object;
        }

        bool operator==(const StatementPrivate& other) const
        {
            return *subject == *(other.subject) &&
                    *predicate == *(other.predicate) &&
                    *object == *(other.object);
        }
};

Statement::Statement() : d(0)
{
}

Statement::Statement(const Statement& other)
{
    d = other.d;
}

Statement::Statement(const Resource& subject, const Property& predicate, 
                        const Node& object) : d(new StatementPrivate)
{
    d->subject = subject.clone();
    d->predicate = predicate.clone();
    d->object = object.clone();
}

Statement::~Statement()
{
}

Statement& Statement::operator=(const Statement& other)
{
    d = other.d;
    return *this;
}

bool Statement::operator==(const Statement& other) const
{
    if (!d || !other.d)
        return d == other.d;

    return *d == *(other.d);
}

bool Statement::isNull() const
{
    return d == (StatementPrivate*)0;
}

Resource* Statement::subject() const
{
    return d->subject;
}

Property* Statement::predicate() const
{
    return d->predicate;
}

Node* Statement::object() const
{
    return d->object;
}

QString Statement::asString() const
{
    if (!d->object->isLiteral())
        return QString::null;

    Literal* l = dynamic_cast<Literal*>(d->object);
    if (!l) // should never happen
        return QString::null;

    return l->text();
}

} // namespace RDF
} // namespace LibSyndication
