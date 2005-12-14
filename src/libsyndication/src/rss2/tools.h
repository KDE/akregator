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

#ifndef LIBSYNDICATION_RSS2_TOOLS_H
#define LIBSYNDICATION_RSS2_TOOLS_H

class QDomNode;
class QDomElement;
class QString;
template <class T> class QList;

namespace LibSyndication {
namespace RSS2 {

/**
 * Methods for XML handling and content extraction.
 *
 * @internal
 *
 * @author Frank Osterfeld
 */
class Tools
{
    public:
    
        static QString extractElementText(const QDomNode& parent, const QString& tagName);
    
        /** 
         * extracts encoded content from XHTML, content:encoded and friends
         * @param parent the parent node, e.g. a channel or item element
         */
        static QString extractContent(const QDomNode& parent);
    
        /**
         * returns all sub elements with tag name @c tagname of a given parent
         * node @c parent
         * Contrary to @ref QDomElement::elementsByTagName() only direct
         * descendents are returned
         */
        static QList<QDomElement> elementsByTagName(const QDomNode& parent, const QString& tagName);
    
    private:

        /**
         * concatenates the XML representations of all children. Example: If @c
         * parent is an @c xhtml:body element like
         * @code
         * <pre><xhtml:body><p>foo</p><blockquote>bar</blockquote></xhtml:body>
         * </pre>
         * @endcode
         * this function returns
         * @code
         * <pre><p>foo</p><blockquote>bar</blockquote></pre> 
         * @endcode
         *
         */
        static QString childNodesAsXML(const QDomNode& parent);
    

        /**
         * returns all sub elements with tag name @c tagname of a given parent
         * node @c parent with namespace URI @c nsURI.
         * Contrary to @ref QDomElement::elementsByTagNameNS() only direct
         * descendents are returned
         */
        static QList<QDomElement> elementsByTagNameNS(const QDomNode& parent, const QString& nsURI, const QString& tagName);

};

} // RSS2
} // LibSyndication

#endif // LIBSYNDICATION_RSS2_TOOLS_H
