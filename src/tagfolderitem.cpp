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

#include "actionmanager.h"
#include "tagfolder.h"
#include "tagfolderitem.h"
#include "treenode.h"

#include <q3popupmenu.h>
#include <kaction.h>
#include <kiconloader.h>

namespace Akregator {

TagFolderItem::TagFolderItem(FolderItem* parent, TagFolder* node) : FolderItem(parent, node)
{
}

TagFolderItem::TagFolderItem(FolderItem* parent, TreeNodeItem* after, TagFolder* node) : FolderItem(parent, after, node)
{
}

TagFolderItem::TagFolderItem(KListView* parent, TagFolder* node) : FolderItem(parent, node)
{
}

TagFolderItem::TagFolderItem(KListView* parent, TreeNodeItem* after, TagFolder* node) : FolderItem(parent, after, node)
{
}


TagFolder* TagFolderItem::node() 
{ 
    return static_cast<TagFolder*> (m_node); 
}

TagFolderItem::~TagFolderItem()
{}


void TagFolderItem::showContextMenu(const QPoint& p)
{
    QWidget* w = ActionManager::getInstance()->container("tagfolder_popup");
    if (w)
        static_cast<Q3PopupMenu *>(w)->exec(p);
}

}
