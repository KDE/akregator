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

#ifndef LIBSYNDICATION_ATOM_GENERATOR_H
#define LIBSYNDICATION_ATOM_GENERATOR_H

#include "../elementwrapper.h"

class QDomDocument;
class QDomElement;
class QString;

namespace LibSyndication {
namespace Atom {

/**
 * Description of the agent used to generate the feed.
 * 
 * @author Frank Osterfeld
 */
class Generator : public ElementWrapper
{
    public:

        /**
         * default constructor, creates a null generator
         */
        Generator();
        
        /**
         * creates a Generator wrapping an atom:generator element.
         * @param element a DOM element, should be a atom:generator element 
         * (although not enforced), otherwise this object will not parse 
         * anything useful
         */
        Generator(const QDomElement& element);

        /**
         * A URI for the generator (e.g. its homepage) (optional)
         */
        QString uri() const;

        /**
         * version of the agent (optional) 
         */
        QString version() const;
        
        /**
         * human-readable name of the generator. (optional)
         * 
         * @return generator name as plain text
         */
        QString name() const;

        /**
         * a description of this generator for debugging purposes.
         * 
         * @return debug info
         */
        QString debugInfo() const;

};

} // namespace Atom
} // namespace LibSyndication

#endif // LIBSYNDICATION_ATOM_GENERATOR_H
