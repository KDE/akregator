/***************************************************************************
 *   Copyright (C) 2004 by Frank Osterfeld                                 *
 *   frank.osterfeld AT kdemail.net                                      *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef AKREGATORFEEDITEM_H
#define AKREGATORFEEDITEM_H

#include "treenodeitem.h"

namespace Akregator 
{

class Feed;
class FeedGroup;
class FeedGroupItem;
        
/**
* the item class corresponding to a Feed
*/
class FeedItem : public TreeNodeItem 
{
 public:
    
    FeedItem(FeedGroupItem* parent, Feed* node);
    FeedItem(FeedGroupItem* parent, TreeNodeItem* after, Feed* node);
    FeedItem(KListView* parent, Feed* node);
    ~FeedItem();
    virtual Feed* node();
    virtual bool isFolder() { return false; }

    virtual void nodeChanged();
    
    static QPixmap errorPixmap();
    
    static QPixmap defaultPixmap();
};

}

#endif
