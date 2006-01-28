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

#ifndef LIBSYNDICATION_RDF_DUBLINCOREVOCAB_H
#define LIBSYNDICATION_RDF_DUBLINCOREVOCAB_H

#include "property.h"

class QString;

namespace LibSyndication {
namespace RDF {

class KDE_EXPORT DublinCoreVocab
{
    public:

        virtual ~DublinCoreVocab();
        
        static DublinCoreVocab* self();
        
        const QString& namespaceURI() const;

        PropertyPtr contributor() const;
        
        PropertyPtr creator() const;

        PropertyPtr coverage() const;
        
        PropertyPtr date() const;
        
        PropertyPtr description() const;

        PropertyPtr format() const;
        
        PropertyPtr identifier() const;
        
        PropertyPtr language() const;
        
        PropertyPtr publisher() const;
        
        PropertyPtr relation() const;
        
        PropertyPtr rights() const;
        
        PropertyPtr source() const;
        
        PropertyPtr subject() const;
        
        PropertyPtr title() const;
        
        PropertyPtr type() const;
        
    protected:
        
        DublinCoreVocab();
        
    private:
        
        static DublinCoreVocab* m_self;
        
        class DublinCoreVocabPrivate;
        DublinCoreVocabPrivate* d;
};

} // namespace RDF
} // namespace LibSyndication

#endif // LIBSYNDICATION_RDF_DUBLINCOREVOCAB_H
