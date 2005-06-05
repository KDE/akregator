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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "folder.h"
#include "folderitem.h"
#include "treenode.h"

#include <kiconloader.h>

using namespace Akregator;

FolderItem::FolderItem(FolderItem* parent, Folder* node) : TreeNodeItem(parent, node)
{
    setRenameEnabled(0, true);
    setExpandable(true);
    setOpen( node->isOpen() );
    setPixmap ( 0, KGlobal::iconLoader()->loadIcon("folder", KIcon::Small) );
    if (node)
        setText(0, node->title());
}

FolderItem::FolderItem(FolderItem* parent, TreeNodeItem* after, Folder* node) : TreeNodeItem(parent, after, node)
{
    setRenameEnabled(0, true);
    setExpandable(true);
    setOpen(node->isOpen());
    setPixmap ( 0, KGlobal::iconLoader()->loadIcon("folder", KIcon::Small) );
    if (node)
        setText(0, node->title());
}

FolderItem::FolderItem(KListView* parent, Folder* node) : TreeNodeItem(parent, node)
{
    setRenameEnabled(0, true);
    setExpandable(true);
    setOpen(true);
    setPixmap ( 0, KGlobal::iconLoader()->loadIcon("folder", KIcon::Small) );
    if (node)
        setText(0, node->title());
}

Folder* FolderItem::node() 
{ 
    return static_cast<Folder*> (m_node); 
}

void FolderItem::setOpen(bool open)
{
    node()->setOpen(open);
    KListViewItem::setOpen(open);
}

FolderItem::~FolderItem()
{}

