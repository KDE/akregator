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

#include "sequence.h"

namespace LibSyndication {
namespace RDF {

class Sequence::SequencePrivate : public KShared
{
    public:
    
        bool operator==(const SequencePrivate& other) const
        {
            return true;
        }
};

Sequence::Sequence() : d(0)
{
}

Sequence::Sequence(const Sequence& other) : Resource(other)
{
    d = other.d;
}

Sequence::~Sequence()
{
}

Sequence& Sequence::operator=(const Sequence& other)
{
    d = other.d;
    return *this;
}

bool Sequence::operator==(const Node& other) const
{
    const Sequence* o2 = dynamic_cast<const Sequence*>(&other);
    if (!o2)
        return false;

    if (Sequence::operator==(other))
        return false;

    if (!d || !o2->d)
        return d == o2->d;
    return *d == *(o2->d);
}


} // namespace RDF
} // namespace LibSyndication
