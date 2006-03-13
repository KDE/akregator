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

#ifndef AKREGATOR_FEEDITEM_H
#define AKREGATOR_FEEDITEM_H

#include "treenodeitem.h"

class QPixmap;

namespace Akregator 
{

class Feed;
class FolderItem;
        
/**
* the item class corresponding to a Feed
*/
class FeedItem : public TreeNodeItem 
{
 public:
    
    FeedItem(FolderItem* parent, Feed* node);
    FeedItem(FolderItem* parent, TreeNodeItem* after, Feed* node);
    FeedItem(K3ListView* parent, Feed* node);
    FeedItem(K3ListView* parent, TreeNodeItem* after, Feed* node);

    virtual ~FeedItem();
    virtual Feed* node();
    virtual void nodeChanged();
    virtual void showContextMenu(const QPoint& p);

    static QPixmap errorPixmap();
    static QPixmap defaultPixmap();

private:
    void initialize(Feed* node);
    
};

} // namespace Akregator

#endif // AKREGATOR_FEEDITEM_H
