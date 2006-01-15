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

#ifndef LIBSYNDICATION_ATOM_PARSER_H
#define LIBSYNDICATION_ATOM_PARSER_H

#include "../abstractparser.h"

class QDomDocument;
class QString;

namespace LibSyndication {

class AbstractDocument;
class DocumentSource;

namespace Atom {

    
class Parser : public LibSyndication::AbstractParser
{
    public:

        bool accept(const LibSyndication::DocumentSource& source) const;

        LibSyndication::AbstractDocumentPtr parse(const LibSyndication::DocumentSource& source) const;
        
        QString format() const;
        
    protected:
        
        /**
         * converts an Atom 0.3 document to an Atom 1.0-similar document
         * that can be parsed by our parser.
         * 
         * @param document a Atom 0.3 document. If it is not Atom 0.3, the 
         * result is undefined.
         * @return a new DOM document, suitable for our parser. This is not
         * completely Atom 1.0-compliant (it might contain now obsolete 
         * elements, like e.g. atom:created or atom:info), but "enough 1.0"
         * for our purposes.
         */
        static QDomDocument convertAtom0_3(const QDomDocument& document);
};

} // namespace Atom
} // namespace LibSyndication

#endif // LIBSYNDICATION_ATOM_PARSER_H
