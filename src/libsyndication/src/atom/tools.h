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

#ifndef LIBSYNDICATION_ATOM_TOOLS_H
#define LIBSYNDICATION_ATOM_TOOLS_H

class QString;

namespace LibSyndication {

class ElementWrapper;

namespace Atom {
    
    /**
     * extracts the content of an @c atomTextConstruct. An atomTextConstruct is an element containing
     * either plain text, escaped html, or embedded XHTML.
     * 
     * TODO: examples
     * 
     * @param parent the parent element to extract from. E.g. an atom:entry element.
     * @param tagname the child element of parent to extract from the text from. 
     * Atom namespace is assumed. Example: <atom:title>
     * @return the HTMLized version of the element content. If the content is
     * escaped HTML, the escaped markup is resolved. If it is XHTML, the XHTML
     * content is converted to a string. If it is plain text, characters like "<", ">", "&"
     * are escaped so the return string is valid HTML.
     */
    QString extractAtomText(const LibSyndication::ElementWrapper& parent, const QString& tagname);

} // namespace Atom
} // namespace LibSyndication

#endif // LIBSYNDICATION_ATOM_TOOLS_H
