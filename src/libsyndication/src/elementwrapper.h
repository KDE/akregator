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
#ifndef LIBSYNDICATION_RSS2_ELEMENTWRAPPER_H
#define LIBSYNDICATION_RSS2_ELEMENTWRAPPER_H

#include <ksharedptr.h>

class QDomElement;

namespace LibSyndication {
namespace RSS2 {

/**
 * A wrapper for XML elements. Base class for RSS2 wrapper classes doing
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

    private:

        class ElementWrapperPrivate;
        KSharedPtr<ElementWrapperPrivate> d;
};

} // namespace RSS2
} // namespace LibSyndication

#endif // LIBSYNDICATION_RSS2_ELEMENTWRAPPER_H
