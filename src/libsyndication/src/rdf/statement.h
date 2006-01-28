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
#ifndef LIBSYNDICATION_RDF_STATEMENT_H
#define LIBSYNDICATION_RDF_STATEMENT_H

#include "property.h"

#include <ksharedptr.h>


class QString;

namespace LibSyndication {
namespace RDF {

class Statement;

typedef KSharedPtr<Statement> StatementPtr;

class KDE_EXPORT Statement : public KShared
{
    public:
    
        Statement();
        Statement(const Statement& other);
        Statement(ResourcePtr subject, PropertyPtr predicate, NodePtr object);
        virtual ~Statement();
    
        Statement& operator=(const Statement& other);
        virtual bool operator==(const Statement& other) const;
    
        virtual bool isNull() const;
    
        virtual ResourcePtr subject() const;
        virtual PropertyPtr predicate() const;
        virtual NodePtr object() const;

        /**
         * returns the object of this statement as resource, if possible.
         * 
         * @return the object node as Resource, or @c null if the object
         * is not a resource
         */
        virtual ResourcePtr asResource() const;
        
        /**
         * returns the object of this statement as string, if possible.
         * 
         * @return the literal text as QString, or QString::null if the object
         * is not a literal
         */
        virtual QString asString() const;

    
    private:
        class StatementPrivate;
        KSharedPtr<StatementPrivate> d;
};


} // namespace RDF
} // namespace LibSyndication

#endif // LIBSYNDICATION_RDF_STATEMENT_H
