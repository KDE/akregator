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

#include "feedgroup.h"
#include "feedgroupitem.h"
#include "treenode.h"

#include <kiconloader.h>

using namespace Akregator;

FeedGroupItem::FeedGroupItem(FeedGroupItem* parent, FeedGroup* node) : TreeNodeItem(parent, node)
{
    setExpandable(true);
    setOpen( node->isOpen() );
    setPixmap ( 0, KGlobal::iconLoader()->loadIcon("folder", KIcon::Small) );
    if (node)
        setText(0, node->title());
}

FeedGroupItem::FeedGroupItem(FeedGroupItem* parent, TreeNodeItem* after, FeedGroup* node) : TreeNodeItem(parent, after, node)
{
    setExpandable(true);
    setOpen( node->isOpen() );
    setPixmap ( 0, KGlobal::iconLoader()->loadIcon("folder", KIcon::Small) );
    if (node)
        setText(0, node->title());
}

FeedGroupItem::FeedGroupItem(KListView* parent, FeedGroup* node) : TreeNodeItem(parent, node)
{
    setExpandable(true);
    setOpen(true);
    setPixmap ( 0, KGlobal::iconLoader()->loadIcon("folder", KIcon::Small) );
    if (node)
        setText(0, node->title());
}

FeedGroup* FeedGroupItem::node() 
{ 
    return static_cast<FeedGroup*> (m_node); 
}

void FeedGroupItem::setOpen(bool open)
{
    node()->setOpen(open);
    KListViewItem::setOpen(open);
}

FeedGroupItem::~FeedGroupItem()
{}

