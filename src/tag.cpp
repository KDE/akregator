/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "shared.h"
#include "tag.h"

#include <qstring.h>

namespace Akregator {

class Tag::TagPrivate : public Shared
{
    public:
    QString id;
    QString name;
    bool operator==(const TagPrivate& other) const
    {
        return id == other.id; // name is ignored!
    }
};

Tag::Tag() : d(new TagPrivate)
{}

Tag::Tag(const QString& id, const QString& name) : d(new TagPrivate)
{
    d->id = id;
    d->name = name.isNull() ? id : name;
}

Tag::Tag(const Tag& other) : d(0)
{
    *this = other;
}

Tag::~Tag()
{
    if (d->deref())
    {
        delete d;
        d = 0;
    }
}

Tag& Tag::operator=(const Tag& other)
{
    if (this != &other)
    {
        other.d->ref();
        if (d && d->deref())
            delete d;
        d = other.d;
    }
    return *this;
}


bool Tag::operator==(const Tag& other) const
{
    return *(other.d) == *d;
}

bool Tag::isNull() const
{
    return d->id.isNull();
}

QString Tag::name() const
{
    return d->name;
}

void Tag::setName(const QString& name)
{
    d->name = name;
}

QString Tag::id() const
{
    return d->id;
}

} // namespace Akregator
