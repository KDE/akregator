/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
    
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
