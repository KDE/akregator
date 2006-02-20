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

#ifndef LIBSYNDICATION_MAPPER_FEEDRDFIMPL_H
#define LIBSYNDICATION_MAPPER_FEEDRDFIMPL_H

#include "../rdf/document.h"
#include "../feed.h"

namespace LibSyndication {

class FeedRDFImpl;
typedef KSharedPtr<FeedRDFImpl> FeedRDFImplPtr;
class Image;
typedef KSharedPtr<Image> ImagePtr;

/**
 * @internal
 */
class FeedRDFImpl : public LibSyndication::Feed
{
    public:
        
        FeedRDFImpl(LibSyndication::RDF::DocumentPtr doc);
        
        LibSyndication::AbstractDocumentPtr document() const;
        
        QList<ItemPtr> items() const;
        
        QList<CategoryPtr> categories() const;
        
        QString title() const;
        
        QString link() const;
        
        QString description() const;
        
        QList<PersonPtr> authors() const;
        
        QString language() const;
        
        QString copyright() const;
        
        ImagePtr image() const;
        
    private:
        
        LibSyndication::RDF::DocumentPtr m_doc;
};

} // namespace LibSyndication

#endif // LIBSYNDICATION_MAPPER_FEEDRDFIMPL_H
