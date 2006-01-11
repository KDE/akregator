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
#ifndef LIBSYNDICATION_RDF_MODEL_H
#define LIBSYNDICATION_RDF_MODEL_H

#include <literal.h>
#include <node.h>
#include <property.h>
#include <sequence.h>
#include <statement.h>

#include <ksharedptr.h>

#include <QString>

template <class T> class QList;

namespace LibSyndication {
namespace RDF {


class Model
{
    public:

        Model();
        Model(const Model& other);

        virtual ~Model();

        Model& operator=(const Model& other);
        bool operator==(const Model& other) const;

        virtual ResourcePtr createResource(ResourcePtr resource);
        
        virtual ResourcePtr createResource(const QString& uri=QString::null);
        
        virtual PropertyPtr createProperty(const QString& uri=QString::null);

        virtual SequencePtr createSequence(const QString& uri=QString::null);
        
        virtual LiteralPtr createLiteral(const QString& text);
        
        virtual StatementPtr addStatement(ResourcePtr subject, PropertyPtr predicate, NodePtr object);
        
        virtual bool isEmpty() const;
        
        virtual bool resourceHasProperty(const Resource* resource, PropertyPtr property) const;
        
        virtual StatementPtr resourceProperty(const Resource* resource, PropertyPtr property) const;
    
        virtual QList<StatementPtr> statements() const;
        
        virtual QList<ResourcePtr> resourcesWithType(ResourcePtr type) const;
        
        virtual NodePtr nodeByID(uint id) const;
        
        virtual ResourcePtr resourceByID(uint id) const;
        
        virtual PropertyPtr propertyByID(uint id) const;
        
        virtual LiteralPtr literalByID(uint id) const;

    private:
        class ModelPrivate;
        KSharedPtr<ModelPrivate> d;
};


} // namespace RDF
} // namespace LibSyndication

#endif // LIBSYNDICATION_RDF_MODEL_H
