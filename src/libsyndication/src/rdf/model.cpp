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

#include "model.h"
#include "property.h"
#include "resource.h"
#include "statement.h"

#include <QList>
#include <QString>

namespace LibSyndication {
namespace RDF {

class Model::ModelPrivate : public KShared
{
    public:
    
        QList<Statement> stmts;

        bool operator==(const ModelPrivate& other) const
        {
            return stmts == other.stmts;
        }
};

Model::Model() : d(new ModelPrivate)
{
}

Model::Model(const Model& other) : d(new ModelPrivate)
{
    d->stmts = other.d->stmts;
}

Model::~Model()
{
}

Model& Model::operator=(const Model& other)
{
    d->stmts = other.d->stmts;
    return *this;
}

bool Model::operator==(const Model& other) const
{
    return *d == *(other.d);
}

Resource Model::createResource(const QString& uri) const
{
    return Resource(uri, *this);
}

    
void Model::addStatement(const Statement& statement)
{
    if (!d->stmts.contains(statement))
        d->stmts.append(statement);
}

void Model::addStatement(const Resource& subject, const Property& predicate, const Node& object)
{
    Statement stmt(subject, predicate, object);
    addStatement(stmt);
}

bool Model::isEmpty() const
{
    return d->stmts.isEmpty();
}

bool Model::resourceHasProperty(const Resource& resource, const Property& property) const
{
    QList<Statement>::ConstIterator it = d->stmts.begin();
    QList<Statement>::ConstIterator end = d->stmts.end();

// TODO: use more efficient storage

    for ( ; it != end; ++it)
    {
        if (*((*it).subject()) == resource && *((*it).predicate()) == property)
            return true;
    }

    return false;
}

Statement Model::resourceProperty(const Resource& resource, const Property& property) const
{
    QList<Statement>::ConstIterator it = d->stmts.begin();
    QList<Statement>::ConstIterator end = d->stmts.end();

// TODO: use more efficient storage

    for ( ; it != end; ++it)
    {
        if (*((*it).subject()) == resource && *((*it).predicate()) == property)
            return *it;
    }

    return Statement();
}

QList<Statement> Model::statements() const
{
    return d->stmts;
}

QList<Resource> Model::listSubjects() const
{
    QList<Resource> subjects;

    QList<Statement>::ConstIterator it = d->stmts.begin();
    QList<Statement>::ConstIterator end = d->stmts.end();

    for ( ; it != end; ++it)
    {
        Resource* subj = (*it).subject();
        if (!subjects.contains(*subj))
            subjects.append(*subj);
    }

    return subjects;
}

} // namespace RDF
} // namespace LibSyndication
