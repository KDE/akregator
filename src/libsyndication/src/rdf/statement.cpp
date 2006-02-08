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
#include "model.h"
#include "property.h"
#include "resource.h"
#include "statement.h"

#include <QString>

namespace LibSyndication {
namespace RDF {

class Statement::StatementPrivate : public KShared
{
    public:
    
        uint subjectID;
        uint predicateID;
        uint objectID;
        Model model;

        bool operator==(const StatementPrivate& other) const
        {
            // FIXME: use better check that works also with multiple models
            return subjectID == other.subjectID &&
                   predicateID == other.predicateID &&
                   objectID == other.objectID;
        }
};

Statement::Statement() : d(new StatementPrivate)
{
    d->subjectID = 0;
    d->predicateID = 0;
    d->objectID = 0;
}

Statement::Statement(const Statement& other) : KShared(other)
{
    d = other.d;
}

Statement::Statement(ResourcePtr subject, PropertyPtr predicate, 
                        NodePtr object) : d(new StatementPrivate)
{
    d->model = subject->model();
    d->subjectID = subject->id();
    d->predicateID = predicate->id();
    d->objectID = object->id();
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
    return d->subjectID == 0;
}

ResourcePtr Statement::subject() const
{
    return d->model.resourceByID(d->subjectID);
}

PropertyPtr Statement::predicate() const
{
    return d->model.propertyByID(d->predicateID);
}

NodePtr Statement::object() const
{
    return d->model.nodeByID(d->objectID);
}

ResourcePtr Statement::asResource() const
{
    if (isNull() || !d->model.nodeByID(d->objectID)->isResource())
        return ResourcePtr(new Resource);

    return d->model.resourceByID(d->objectID);
}

QString Statement::asString() const
{
    if (isNull() || !d->model.nodeByID(d->objectID)->isLiteral())
        return QString::null;

    LiteralPtr l = d->model.literalByID(d->objectID);
    
    if (l->isNull())
        return QString::null;

    return l->text();
}

} // namespace RDF
} // namespace LibSyndication
