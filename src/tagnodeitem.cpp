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

#include "actionmanager.h"
#include "tagnode.h"
#include "tagnodeitem.h"
#include "treenode.h"

#include <kaction.h>
#include <kiconloader.h>

#include <qpoint.h>
#include <qpopupmenu.h>

using namespace Akregator;

TagNodeItem::TagNodeItem(FolderItem* parent, TagNode* node) : TreeNodeItem(parent, node)
{
    setRenameEnabled(0, false);
    setExpandable(false);
    setPixmap ( 0, KGlobal::iconLoader()->loadIcon("bookmark_folder", KIcon::Small) );
    if (node)
        setText(0, node->title());
}

TagNodeItem::TagNodeItem(FolderItem* parent, TreeNodeItem* after, TagNode* node) : TreeNodeItem(parent, after, node)
{
    setRenameEnabled(0, false);
    setExpandable(false);
    setPixmap ( 0, KGlobal::iconLoader()->loadIcon("bookmark_folder", KIcon::Small) );
    if (node)
        setText(0, node->title());
}

TagNodeItem::TagNodeItem(KListView* parent, TagNode* node) : TreeNodeItem(parent, node)
{
    setRenameEnabled(0, false);
    setExpandable(false);
    setPixmap ( 0, KGlobal::iconLoader()->loadIcon("bookmark_folder", KIcon::Small) );
    if (node)
        setText(0, node->title());
}

TagNode* TagNodeItem::node() 
{ 
    return static_cast<TagNode*> (m_node); 
}

void TagNodeItem::showContextMenu(const QPoint& p)
{
    QWidget* w = ActionManager::getInstance()->container("tagnode_popup");
    ActionManager::getInstance()->action("feed_mark_all_as_read")->setText("&Mark Articles as Read");
    ActionManager::getInstance()->action("feed_remove")->setText("&Delete Tag");
    ActionManager::getInstance()->action("feed_modify")->setText("&Rename Tag");

    if (w)
        static_cast<QPopupMenu *>(w)->exec(p);
}

TagNodeItem::~TagNodeItem()
{}

