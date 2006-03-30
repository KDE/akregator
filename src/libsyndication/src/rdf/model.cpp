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
#include "nodevisitor.h"
#include "property.h"
#include "rdfvocab.h"
#include "resource.h"
#include "sequence.h"
#include "statement.h"

#include <QHash>
#include <QList>
#include <QString>

namespace LibSyndication {
namespace RDF {

class Model::ModelPrivate
{
    public:
    
    long id;
    static long idCounter;
    LiteralPtr nullLiteral;
    PropertyPtr nullProperty;
    ResourcePtr nullResource;
    StatementPtr nullStatement;
    QHash<QString, StatementPtr> statements;
    QHash<int, NodePtr> nodes;
    QHash<QString, ResourcePtr> resources;
    QHash<QString, PropertyPtr> properties;
    QHash<QString, SequencePtr> sequences;
    Model* model;
    
    class AddToHashesVisitor;
    
    ModelPrivate(Model* m)
    {
        model = m;
        id = idCounter++;
        addToHashesVisitor = new AddToHashesVisitor(this);
        initialized = false;
    }
    
    ~ModelPrivate()
    {
        delete addToHashesVisitor;
    }
    
    bool operator==(const ModelPrivate& other) const
    {
        return id == other.id;
    }
    
    class AddToHashesVisitor : public NodeVisitor
    {
        public:
            
        AddToHashesVisitor(ModelPrivate* parent) : p(parent)
        {}
        
        bool visitResource(ResourcePtr res)
        {
            visitNode(res);
            p->resources[res->uri()] = res;
            return true;
        }
    
        bool visitSequence(SequencePtr seq)
        {
            visitResource(seq);
            p->sequences[seq->uri()] = seq;
            return true;
        }

        bool visitProperty(PropertyPtr prop)
        {
            visitResource(prop);
            p->properties[prop->uri()] = prop;
            return true;
        }

        bool visitNode(NodePtr node)
        {
            p->nodes[node->id()] = node;
            return true;
        }
        
        ModelPrivate* p;
    };
    
    AddToHashesVisitor* addToHashesVisitor;
    
    void addToHashes(NodePtr node)
    {
        addToHashesVisitor->visit(node);
    }
    
    void addToHashes(StatementPtr stmt, const QString& key)
    {
        statements[key] = stmt;
    }
    
    void removeFromHashes(const QString& key)
    {
        statements.remove(key);
    }
    
    bool initialized;
    
    void init()
    {
        if (!initialized)
        {
            nullLiteral = new Literal();
            nullLiteral->setModel(*model);
            nullProperty = new Property();
            nullProperty->setModel(*model);
            nullResource = new Resource();
            nullResource->setModel(*model);
            nullStatement = new Statement();
            initialized = true;
        }
    }
};

long Model::ModelPrivate::idCounter = 0;

Model::Model() : d(new ModelPrivate(this))
{
}

Model::Model(const Model& other)
{
    *this = other;
}

Model::~Model()
{
}

Model& Model::operator=(const Model& other)
{
    d = other.d;
    return *this;
}

bool Model::operator==(const Model& other) const
{
    return *d == *(other.d);
}

PropertyPtr Model::createProperty(const QString& uri)
{
    PropertyPtr prop;
    
    if (d->properties.contains(uri))
    {
        prop = d->properties[uri];
    }
    else
    {
        prop = new Property(uri);
        prop->setModel(*this);
        // if there is a resource object with the same uri, replace
        // the resource object by the new property object and reuse the id
        if (d->resources.contains(uri))
        {
            prop->setId(d->resources[uri]->id());
        }
        d->addToHashes(prop);
    }

    return prop;

}

ResourcePtr Model::createResource(const QString& uri)
{
    ResourcePtr res;
    
    if (d->resources.contains(uri))
    {
        res = d->resources[uri];
    }
    else
    {
        res = new Resource(uri);
        res->setModel(*this);
        d->addToHashes(res);
    }

    return res;
}

SequencePtr Model::createSequence(const QString& uri)
{
    SequencePtr seq;
    
    if (d->sequences.contains(uri))
    {
        seq = d->sequences[uri];
    }
    else
    {
        seq = new Sequence(uri);
        seq->setModel(*this);
        // if there is a resource object with the same uri, replace
        // the resource object by the new sequence object and reuse the id
        if (d->resources.contains(uri))
        {
            seq->setId(d->resources[uri]->id());
        }

        d->addToHashes(seq);
    }

    return seq;
}

LiteralPtr Model::createLiteral(const QString& text)
{
    LiteralPtr lit(new Literal(text));
    
    d->addToHashes(lit);
    return lit;
}


void Model::removeStatement(StatementPtr statement)
{
    removeStatement(statement->subject(), statement->predicate(), statement->object());
}
        
void Model::removeStatement(ResourcePtr subject, PropertyPtr predicate, NodePtr object)
{
    QString key = QString("%1-%2-%3")
            .arg(QString::number(subject->id()))
            .arg(QString::number(predicate->id()))
            .arg(QString::number(object->id()));
    d->removeFromHashes(key);
}

StatementPtr Model::addStatement(ResourcePtr subject, PropertyPtr predicate, NodePtr object)
{
    d->init();
    ResourcePtr subjInternal = subject;
    
    if (!d->nodes.contains(subjInternal->id()))
    {
        subjInternal = subject->clone();
        subjInternal->setModel(*this);
        d->addToHashes(subjInternal);
    }
    
    PropertyPtr predInternal = predicate;
    
    if (!d->nodes.contains(predInternal->id()))
    {
        predInternal = predicate->clone();
        predInternal->setModel(*this);
        d->addToHashes(predInternal);
    }
    
    NodePtr objInternal = object;
            
    if (!d->nodes.contains(objInternal->id()))
    {
        objInternal = object->clone();
        objInternal->setModel(*this);
        d->addToHashes(objInternal);
    }
    
    // TODO: avoid duplicated stmts with literal objects!
    
    QString key = QString("%1-%2-%3")
            .arg(QString::number(subjInternal->id()))
            .arg(QString::number(predInternal->id()))
            .arg(QString::number(objInternal->id()));
    
    StatementPtr stmt;
            
    if (!d->statements.contains(key))
    {
        stmt = new Statement(subjInternal, predInternal, objInternal);
        d->addToHashes(stmt, key);
    }
    else
    {
        stmt = d->statements[key];
    }
    
    return stmt;
}

bool Model::isEmpty() const
{
    return d->statements.isEmpty();
}

bool Model::resourceHasProperty(const Resource* resource, PropertyPtr property) const
{
    // resource unknown
    if (!d->resources.contains(resource->uri()))
        return false;
    
    QList<StatementPtr> stmts = d->statements.values();
    QList<StatementPtr>::ConstIterator it = stmts.begin();
    QList<StatementPtr>::ConstIterator end = stmts.end();

    // TODO: use more efficient storage

    for ( ; it != end; ++it)
    {
        if (*((*it)->subject()) == *resource && *((*it)->predicate()) == *property)
            return true;
    }

    return false;
}

StatementPtr Model::resourceProperty(const Resource* resource, PropertyPtr property) const
{
    QList<StatementPtr> stmts = d->statements.values();
    QList<StatementPtr>::ConstIterator it = stmts.begin();
    QList<StatementPtr>::ConstIterator end = stmts.end();

    // TODO: use more efficient storage

    for ( ; it != end; ++it)
    {
        if (*((*it)->subject()) == *resource && *((*it)->predicate()) == *property)
            return *it;
    }

    return d->nullStatement;
}

QList<StatementPtr> Model::resourceProperties(const Resource* resource, PropertyPtr property) const
{
    QList<StatementPtr> res;
    QList<StatementPtr> stmts = d->statements.values();
    QList<StatementPtr>::ConstIterator it = stmts.begin();
    QList<StatementPtr>::ConstIterator end = stmts.end();

    // TODO: use more efficient storage

    for ( ; it != end; ++it)
    {
        if (*((*it)->subject()) == *resource && *((*it)->predicate()) == *property)
            res.append(*it);
    }
    
    return res;
}


QList<StatementPtr> Model::statements() const
{
    return d->statements.values();
}

QString Model::debugInfo() const
{
    QString info;
    
    QList<StatementPtr> stmts = d->statements.values();
    QList<StatementPtr>::ConstIterator it = stmts.begin();
    QList<StatementPtr>::ConstIterator end = stmts.end();
    
    for ( ; it != end; ++it)
    {
        info += QString("<%1> <%2> ").arg((*it)->subject()->uri()).arg((*it)->predicate()->uri());
        
        if ((*it)->object()->isLiteral())
        {
            info += QString("\"%1\"\n").arg((*it)->asString());
        }
        else
        {
            info += QString("<%1>\n").arg((*it)->asResource()->uri());
        }
         
    }
    
    return info;
}

QList<ResourcePtr> Model::resourcesWithType(ResourcePtr type) const
{
    QList<ResourcePtr> list;
    
    QList<StatementPtr> stmts = d->statements.values();
    QList<StatementPtr>::ConstIterator it = stmts.begin();
    QList<StatementPtr>::ConstIterator end = stmts.end();

    for ( ; it != end; ++it)
    {
        if (*((*it)->predicate()) == *(RDFVocab::self()->type()) && *((*it)->object()) == *type )
            list.append((*it)->subject());
    }

    return list;
}

NodePtr Model::nodeByID(uint id) const
{
    if (!d->nodes.contains(id))
    {
        return d->nullLiteral;
    }
    else
    {
        return d->nodes[id];
    }
}
        
ResourcePtr Model::resourceByID(uint id) const
{
    if (!d->nodes.contains(id))
    {
        return d->nullResource;
    }
    else
    {
        NodePtr node = d->nodes[id];
        if (node->isResource())
            return node;
        else
            return d->nullResource;
    }
}

PropertyPtr Model::propertyByID(uint id) const
{
    if (!d->nodes.contains(id))
    {
        return d->nullProperty;
    }
    else
    {
        NodePtr node = d->nodes[id];
        if (node->isProperty())
            return node;
        else
            return d->nullProperty;
    }
}

LiteralPtr Model::literalByID(uint id) const
{
    if (!d->nodes.contains(id))
    {
        return d->nullLiteral;
    }
    else
    {
        NodePtr node = d->nodes[id];
        if (node->isLiteral())
            return node;
        else
            return d->nullLiteral;
    }
}

} // namespace RDF
} // namespace LibSyndication
