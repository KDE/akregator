/***************************************************************************
*   Copyright (C) 2004 by Frank Osterfeld                                 *
*   frank.osterfeld AT kdemail.net                                      *
*                                                                         *
*   Licensed under GPL.                                                   *
***************************************************************************/

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

void FeedGroupItem::showContextMenu()
{
}

void FeedGroupItem::showEditProperties()
{
}

void FeedGroupItem::showAskDelete()
{
}

void FeedGroupItem::setOpen(bool open)
{
    node()->setOpen(open);
    KListViewItem::setOpen(open);
}

FeedGroupItem::~FeedGroupItem()
{
}

