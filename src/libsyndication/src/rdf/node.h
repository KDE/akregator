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
#ifndef LIBSYNDICATION_RDF_NODE_H
#define LIBSYNDICATION_RDF_NODE_H

#include <ksharedptr.h>

namespace LibSyndication {
namespace RDF {

class Model;
class Node;
class NodeVisitor;

typedef KSharedPtr<Node> NodePtr;

class Node : public KShared
{
    public:
        
        virtual ~Node();
        
        virtual void accept(NodeVisitor* visitor, NodePtr ptr);
        
        virtual bool operator==(const Node& other) const = 0;
        
        virtual Node* clone() const = 0;
        virtual bool isNull() const = 0;
        virtual unsigned int id() const = 0;
        virtual bool isResource() const = 0;
        virtual bool isLiteral() const = 0;
        virtual bool isSequence() const = 0;
        virtual bool isAnon() const = 0;
        virtual void setModel(const Model& model) = 0;
        
    protected:
        
        static unsigned int idCounter;
};

} // namespace RDF
} // namespace LibSyndication

#endif // LIBSYNDICATION_RDF_NODE_H
