/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
#include "feedscollection.h"
#include "feedgroup.h"
#include "feed.h"
#include "treenode.h"
#include "treenodeitem.h"

#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kmessagebox.h>


using namespace Akregator;

FeedsCollection::FeedsCollection()
    : QPtrDict<TreeNode>()
    , modified(false)
{
}

FeedsCollection::~FeedsCollection()
{}

void FeedsCollection::addFeed(Feed *f)
{
    insert(f->item(), f);
    modified = true;
}

void FeedsCollection::addFeedGroup(FeedGroup* feedGroup)
{
    insert(feedGroup->item(), feedGroup);
    modified = true;
}

void FeedsCollection::removeFeed(TreeNodeItem *item)
{
    TreeNode *node = find(item);
    if (!node)
    {
        KMessageBox::error( 0, i18n("Internal error while removing note.") );
        return;
    }

    for (QListViewItemIterator it(item); it.current() && (it.current() == item || it.current()->depth() > item->depth()); ++it)
        delete find(it.current());
    delete item;
    modified = true;
}

bool FeedsCollection::changeFeed(TreeNodeItem */*item*/)
{
    return false;
}

void FeedsCollection::clearFeeds()
{
    setAutoDelete(true);
    clear();
    setAutoDelete(false);
}

void FeedsCollection::updateFeedsView()
{
}
