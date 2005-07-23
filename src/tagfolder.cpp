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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "tagfolder.h"
#include "treenodevisitor.h"

#include <qdom.h>

namespace Akregator {

class TagFolder::TagFolderPrivate
{
};

TagFolder::TagFolder(const QString& title) : Folder(title), d(new TagFolderPrivate)
{
}

TagFolder::~TagFolder()
{
    emitSignalDestroyed();
    delete d;
    d = 0;
}

bool TagFolder::accept(TreeNodeVisitor* visitor)
{
    if (visitor->visitTagFolder(this))
        return true;
    else
        return visitor->visitFolder(this);
}

QDomElement TagFolder::toOPML( QDomElement /*parent*/, QDomDocument /*document*/ ) const
{
    return QDomElement();
}

}
