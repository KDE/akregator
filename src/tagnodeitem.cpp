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
#include "tagnode.h"
#include "tagnodeitem.h"
#include "treenode.h"

#include <kaction.h>
#include <kiconloader.h>

#include <QPoint>
#include <QMenu>

namespace Akregator {

TagNodeItem::TagNodeItem(FolderItem* parent, TagNode* node) : TreeNodeItem(parent, node)
{
    initialize(node);
}

TagNodeItem::TagNodeItem(FolderItem* parent, TreeNodeItem* after, TagNode* node) : TreeNodeItem(parent, after, node)
{
    initialize(node);
}

TagNodeItem::TagNodeItem(K3ListView* parent, TagNode* node) : TreeNodeItem(parent, node)
{
    initialize(node);
}

TagNodeItem::TagNodeItem(K3ListView* parent, TreeNodeItem* after, TagNode* node) : TreeNodeItem(parent, after, node)
{
    initialize(node);
}

void TagNodeItem::initialize(TagNode* node)
{
    setExpandable(false);

    if (node)
    {
        setText(0, node->title());
        setPixmap ( 0, KGlobal::iconLoader()->loadIcon(node->icon(), K3Icon::Small) );
    }
}

void TagNodeItem::nodeChanged()
{
    setPixmap ( 0, KGlobal::iconLoader()->loadIcon(node()->icon(), K3Icon::Small));
    TreeNodeItem::nodeChanged();
}

TagNode* TagNodeItem::node() 
{ 
    return static_cast<TagNode*> (m_node); 
}

void TagNodeItem::showContextMenu(const QPoint& p)
{
    QWidget* w = ActionManager::getInstance()->container("tagnode_popup");
    if (w)
        static_cast<QMenu *>(w)->exec(p);
}

TagNodeItem::~TagNodeItem()
{}

} // namespace Akregator

