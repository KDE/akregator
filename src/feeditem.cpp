/***************************************************************************
 *   Copyright (C) 2004 by Frank Osterfeld                                 *
 *   frank.osterfeld AT kdemail.net                                      *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "feed.h"
#include "feeditem.h"

#include <kiconloader.h>

using namespace Akregator;

FeedItem::FeedItem(FeedGroupItem* parent, Feed* node) : TreeNodeItem(parent, node)
{
    setExpandable(false);
    
    if (node)
    {
        setText(0, node->title());
        if (!node->favicon().isNull())
            setPixmap( 0, defaultPixmap() );
        else
            setPixmap( 0, node->favicon() );
    }
}

FeedItem::FeedItem(KListView* parent, Feed* node) : TreeNodeItem(parent, node)
{
    setExpandable(false);
    
    if (node)
    {
        setText(0, node->title());
        if (!node->favicon().isNull())
            setPixmap( 0, defaultPixmap() );
        else
            setPixmap( 0, node->favicon() );
    }
}

FeedItem::FeedItem(FeedGroupItem* parent, TreeNodeItem* after, Feed* node) : TreeNodeItem(parent, after, node)
{
    setExpandable(false);
    if (node)
    {
        setText(0, node->title());
        if (!node->favicon().isNull())
            setPixmap( 0, defaultPixmap() );
        else
            setPixmap( 0, node->favicon() );
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
            setPixmap(0, defaultPixmap());
    }
    
    TreeNodeItem::nodeChanged();
}

void FeedItem::showContextMenu()
{
}
 
void FeedItem::showEditProperties()
{
}

void FeedItem::showAskDelete()
{
}

QPixmap FeedItem::errorPixmap()
{
    return KGlobal::iconLoader()->loadIcon("error", KIcon::Small);
}

QPixmap FeedItem::defaultPixmap()
{
    return KGlobal::iconLoader()->loadIcon("txt", KIcon::Small);
}


