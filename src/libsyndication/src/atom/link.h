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

#ifndef LIBSYNDICATION_ATOM_LINK_H
#define LIBSYNDICATION_ATOM_LINK_H

#include "../elementwrapper.h"

class QDomElement;
class QString;

namespace LibSyndication {
namespace Atom {
/**
 * defines a reference from an entry or feed to a Web resource
 * 
 * @author Frank Osterfeld
 */
class KDE_EXPORT Link : public ElementWrapper
{
    public:

        /**
         * creates a null link object.
         */
        Link();
        
        /**
         * creates a Link object wrapping an atom:link element.
         * @param element a DOM element, should be a atom:link element
         * (although not enforced), otherwise this object will not parse 
         * anything useful
         */
        Link(const QDomElement& element);
    
        /**
         * URL of the referenced resource (required)
         */
        QString href() const;
    
        /**
         * the relation between the feed/entry and the linked resource.
         *
         * The value of rel() is usually one of the following:
         * 
         * @c "alternate": The URL points to an alternate version of the
         * feed/entry. In practice, this is the article described in an entry,
         * or the homepage of the feed.
         * 
         * @c "enclosure": The link describes an Enclosure. See 
         * LibSyndication::Enclosure for more information.
         * 
         * @c "related": links to web resources with related content. E.g., an
         * article discussing KDE might link to the KDE homepage.
         * 
         * @c "self": "identifies a resource equivalent to the containing
         * element". This is usually the URL of the feed source itself.
         * 
         * @c "via": The link points to the source of the information contained
         * in the feed/entry
         * 
         * @return the rel value specified in the feed. Default value is
         * @c "alternate"
         */
        QString rel() const;
        
        /**
         * MIME type of the linked resource. (optional)
         * 
         * @return MIME type following (TODO: link spec), or a null string if
         * not set
         */
        QString type() const;
    
        /**
         * the language of the linked resource. (optional)
         * If used together with a rel() value of "alternate", it
         * implies a translated version of the entry.
         * 
         * @return TODO: link spec, or a null string if not specified
         */
        QString hrefLanguage() const;
    
        /**
         * human-readable information about the link. (optional)
         * 
         * @return the link title as plain text ("<", "&" are text, not 
         * markup!), or a null string if not specified
         */
        QString title() const;
    
        /**
         * size of the linked resource in bytes. (optional)
         * 
         * @return file size in bytes, or 0 if not specified
         */
        uint length() const;
    
        /**
         * description of the link object for debugging purposes
         *
         * @return debug string
         */
        QString debugInfo() const;
};

} // namespace Atom
} // namespace LibSyndication

#endif // LIBSYNDICATION_ATOM_LINK_H
