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

#include "feed.h"
#include "feeditem.h"

#include <kdebug.h>
#include <kiconloader.h>

using namespace Akregator;

FeedItem::FeedItem(FeedGroupItem* parent, Feed* node) : TreeNodeItem(parent, node)
{
    setExpandable(false);
    
    if (node)
    {
        setText(0, node->title());
        if (!node->favicon().isNull())
            setPixmap( 0, node->favicon() );
        else
            setPixmap( 0, defaultPixmap() );
    }
}

FeedItem::FeedItem(KListView* parent, Feed* node) : TreeNodeItem(parent, node)
{
    setExpandable(false);
    
    if (node)
    {
        setText(0, node->title());
        if (!node->favicon().isNull())
            setPixmap( 0, node->favicon() );
        else
            setPixmap( 0, defaultPixmap() );
            
    }
    else
        kdDebug() << "FeedItem::FeedItem(): node is null!" << endl;
}

FeedItem::FeedItem(FeedGroupItem* parent, TreeNodeItem* after, Feed* node) : TreeNodeItem(parent, after, node)
{
    setExpandable(false);
    if (node)
    {
        setText(0, node->title());
        if (!node->favicon().isNull())
            setPixmap( 0, node->favicon() );
        else
            setPixmap( 0, defaultPixmap() );
    }
}

FeedItem::~FeedItem()
{
}

Feed* FeedItem::node() 
{ 
    return static_cast<Feed*> (m_node); 
}

void FeedItem::nodeChanged()
{
    if ( node()->fetchErrorOccurred() )
        setPixmap(0, errorPixmap());
    else
    {
        if (!node()->favicon().isNull())
             setPixmap(0, node()->favicon());
        else
            setPixmap( 0, defaultPixmap() );
    }
    
    TreeNodeItem::nodeChanged();
}

QPixmap FeedItem::errorPixmap()
{
    return KGlobal::iconLoader()->loadIcon("error", KIcon::Small);
}

QPixmap FeedItem::defaultPixmap()
{
    return KGlobal::iconLoader()->loadIcon("txt", KIcon::Small);
}


