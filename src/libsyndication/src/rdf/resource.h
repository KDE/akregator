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
 
#ifndef LIBSYNDICATION_RDF_RESOURCE_H
#define LIBSYNDICATION_RDF_RESOURCE_H

#include "node.h"

#include <ksharedptr.h>

class QString;

namespace LibSyndication {
namespace RDF {

class Model;
class Property;
class Statement;
    
class Resource : public Node
{
    public:
    
        Resource();
        Resource(const Resource& other);
        Resource(const QString& uri, const Model& model);
        virtual ~Resource();
        
        virtual Resource& operator=(const Resource& other);
        virtual bool operator==(const Node& other) const;
    
        virtual Resource* clone() const;
    
        virtual Model model() const;
    
        virtual bool hasProperty(const Property& property) const;
        virtual Statement property(const Property& property) const;
    
        virtual bool isNull() const;
    
        virtual bool isResource() const;
        virtual bool isLiteral() const;
        virtual bool isAnon() const;
    
        virtual QString uri() const;
    
    private:
    
        class ResourcePrivate;
        KSharedPtr<ResourcePrivate> d;
};
    
} // namespace RDF
} // namespace LibSyndication

#endif // LIBSYNDICATION_RDF_RESOURCE_H
