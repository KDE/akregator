/***************************************************************************
*   Copyright (C) 2004 by Frank Osterfeld                                 *
*   frank.osterfeld AT kdemail.net                                      *
*                                                                         *
*   Licensed under GPL.                                                   *
***************************************************************************/

#ifndef AKREGATORFEEDGROUPITEM_H
#define AKREGATORFEEDGROUPITEM_H

#include "treenodeitem.h"
#include "treenode.h"
#include "feedgroup.h"

#include <qpixmap.h>

namespace Akregator 
{

class FeedGroup;

/**
*/
class FeedGroupItem : public TreeNodeItem
{

public:
    FeedGroupItem(FeedGroupItem* parent, FeedGroup* node);
    FeedGroupItem(FeedGroupItem* parent, TreeNodeItem* after, FeedGroup* node);
    FeedGroupItem(KListView* parent, FeedGroup* node);
    ~FeedGroupItem();
    virtual FeedGroup* node();
    
    virtual bool isFolder() { return true; }
    virtual void setOpen(bool open);
};

};

#endif
