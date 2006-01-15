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
#ifndef LIBSYNDICATION_ELEMENTWRAPPER_H
#define LIBSYNDICATION_ELEMENTWRAPPER_H

#include <ksharedptr.h>

class QDomElement;
template <class T> class QList;

namespace LibSyndication {

/**
 * A wrapper for XML elements. Base class for wrapper classes doing
 * lazy parsing.
 *
 * @author Frank Osterfeld
 */
class ElementWrapper
{
    public:
        ElementWrapper();
        ElementWrapper(const ElementWrapper& other);
        ElementWrapper(const QDomElement& element);
        virtual ~ElementWrapper();

        ElementWrapper& operator=(const ElementWrapper& other);
        bool operator==(const ElementWrapper& other) const;
        
        /**
         * returns the wrapped resource.
         */
        const QDomElement& element() const;

        /**
         * returns whether the wrapped element is a null element
         * @return @c true if isNull() is true for the wrapped element,
         * @c false otherwise
         */
        bool isNull() const;
        
        /**
         * returns the xml:base value to be used for the wrapped element.
         * The xml:base attribute establishes the base URI for resolving any
         * relative references found in its scope (its own element and all 
         * descendants). (See also completeURI())
         * 
         * @return the xml:base value, or QString::null if not set
         */
        QString xmlBase() const;
        
        /**
         * returns the xml:lang value to be used for the wrapped element.
         * The xml:lang attribute indicates the natural language for its element
         * and all descendants.
         * 
         * @return the xml:lang value, or QString::null if not set
         */
        QString xmlLang() const;

        /**
         * completes relative URIs with
         * 
         * Example: 
         * @code
         * xml:base="http://www.foo.org/", uri="announcements/bar.html"
         * @endcode
         * 
         * is completed to @c http://www.foo.org/announcements/bar.html
         * 
         * @param uri a possibly relative URI
         * @return the resolved, absolute URI (using xml:base), if @c uri is
         * a relative, valid URI. If @c uri is not a valid URI or absolute, @c uri
         * is returned unmodified
         */
        QString completeURI(const QString& uri) const;
        
        QString extractElementTextNS(const QString& namespaceURI, const QString& localName) const;
        
        QString extractElementText(const QString& tagName) const;

        /**
         * returns all subelements with tag name @c tagname 
         * Contrary to @ref QDomElement::elementsByTagName() only direct descendents are returned.
         */
        QList<QDomElement> elementsByTagName(const QString& tagName) const;
    
        QString childNodesAsXML() const;
        
        /**
         * concatenates the XML representations of all children. Example: If 
         * @c parent is an @c xhtml:body element like
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
        static QString childNodesAsXML(const QDomElement& parent);
        
        /**
         * returns all sub elements with tag name @c tagname of a given parent
         * node @c parent with namespace URI @c nsURI.
         * Contrary to @ref QDomElement::elementsByTagNameNS() only direct
         * descendents are returned
         */
        QList<QDomElement> elementsByTagNameNS(const QString& nsURI, const QString& tagName) const;
        
        QDomElement firstElementByTagNameNS(const QString& nsURI, const QString& tagName) const;
        
    private:

        class ElementWrapperPrivate;
        KSharedPtr<ElementWrapperPrivate> d;
};

} // namespace LibSyndication

#endif // LIBSYNDICATION_ELEMENTWRAPPER_H
