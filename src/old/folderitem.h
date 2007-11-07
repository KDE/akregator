/*
    This file is part of Akregator.

    Copyright (C) 2004 Frank Osterfeld <frank.osterfeld at kdemail.net>

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
#ifndef AKREGATOR_FOLDERITEM_H
#define AKREGATOR_FOLDERITEM_H

#include "folder.h"
#include "treenode.h"
#include "treenodeitem.h"

class QPoint;

namespace Akregator 
{

class Folder;

class FolderItem : public TreeNodeItem
{

public:
    FolderItem(FolderItem* parent, Folder* node);
    FolderItem(FolderItem* parent, TreeNodeItem* after, Folder* node);
    FolderItem(K3ListView* parent, Folder* node);
    FolderItem(K3ListView* parent, TreeNodeItem* after, Folder* node);
    virtual ~FolderItem();
    
    virtual Folder* node();
    
    virtual void setOpen(bool open);
    virtual void showContextMenu(const QPoint& p);

private:
    void initialize(Folder* node);
};

} // namespace Akregator

#endif // AKREGATOR_FOLDERITEM_H
