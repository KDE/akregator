/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>
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
#ifndef LIBSYNDICATION_ABSTRACTDOCUMENT_H
#define LIBSYNDICATION_ABSTRACTDOCUMENT_H

#include <ksharedptr.h>

class QString;

namespace LibSyndication {

class DocumentVisitor;
class AbstractDocument;

typedef KSharedPtr<AbstractDocument> AbstractDocumentPtr;

/**
 * Document interface. A document is a representation parsed from a document 
 * source (see DocumentSource). It typically contains several information items
 * (i.e. articles) plus some additional metadata about the news feed.
 * The Document classes from the several syndication formats must implement
 * this interface. It's main purpose is to provide access for document visitors
 * (see DocumentVisitor).
 *
 * @author Frank Osterfeld
 */
class AbstractDocument
{
    public:

        virtual ~AbstractDocument() {}

        virtual bool accept(DocumentVisitor* visitor) = 0;

        /**
         * Returns a description of the document for debugging purposes.
         *
         * @return debug string
         */
        virtual QString debugInfo() const = 0;
};

} // namespace LibSyndication

#endif // LIBSYNDICATION_ABSTRACTDOCUMENT_H

