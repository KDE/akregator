/*
    This file is part of Akregator.

    Copyright (C) 2004 Frank Osterfeld <osterfeld@kde.org>

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
#include "folder.h"
#include "folderitem.h"
#include "treenode.h"

#include <QMenu>
#include <kaction.h>
#include <kiconloader.h>

namespace Akregator {

FolderItem::FolderItem(FolderItem* parent, Folder* node) : TreeNodeItem(parent, node)
{
    initialize(node);
}

FolderItem::FolderItem(FolderItem* parent, TreeNodeItem* after, Folder* node) : TreeNodeItem(parent, after, node)
{
    initialize(node);
}

FolderItem::FolderItem(K3ListView* parent, Folder* node) : TreeNodeItem(parent, node)
{
    initialize(node);
}

FolderItem::FolderItem(K3ListView* parent, TreeNodeItem* after, Folder* node) : TreeNodeItem(parent, after, node)
{
    initialize(node);
}

void FolderItem::initialize(Folder* node)
{
    setOpen(node && node->isOpen());
    setPixmap ( 0, KIconLoader::global()->loadIcon("folder", KIconLoader::Small) );
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
    K3ListViewItem::setOpen(open);
}

FolderItem::~FolderItem()
{}


void FolderItem::showContextMenu(const QPoint& p)
{
    QWidget* w = ActionManager::getInstance()->container("feedgroup_popup");
    if (w)
        static_cast<QMenu *>(w)->exec(p);
}

} // namespace Akregator
