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
#include "elementwrapper.h"

#include <QDomElement>

namespace LibSyndication {
namespace RSS2 {

class ElementWrapper::ElementWrapperPrivate : public KShared
{
    public:
        
    QDomElement element;
};

ElementWrapper::ElementWrapper() : d(new ElementWrapperPrivate)
{
}

ElementWrapper::ElementWrapper(const ElementWrapper& other)
{
    *this = other;
}

ElementWrapper::ElementWrapper(const QDomElement& element) : d(new ElementWrapperPrivate)
{
    d->element = element;
}

ElementWrapper::~ElementWrapper()
{
}

ElementWrapper& ElementWrapper::operator=(const ElementWrapper& other)
{
    d = other.d;
    return *this;
}

bool ElementWrapper::operator==(const ElementWrapper& other) const
{
    return d->element == other.d->element;
}

bool ElementWrapper::isNull() const
{
    return d->element.isNull();
}

const QDomElement& ElementWrapper::element() const
{
    return d->element;
}

} // namespace RSS2
} // namespace LibSyndication
