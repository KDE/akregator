/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
#include "feedscollection.h"
#include "feedgroup.h"
#include "feed.h"

#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include <qlistview.h>

using namespace Akregator;

FeedsCollection::FeedsCollection()
    : QPtrDict<FeedGroup>()
    , modified(false)
{
}

FeedsCollection::~FeedsCollection()
{}

Feed *FeedsCollection::addFeed(QListViewItem *item)
{
    Feed *feed = new Feed(item, this);
    insert(item, feed);
    item->setPixmap(0, KGlobal::iconLoader()->loadIcon("txt", KIcon::Small) );
    modified = true;
    return feed;
}

FeedGroup *FeedsCollection::addFeedGroup(QListViewItem *item)
{
    FeedGroup *feedGroup = new FeedGroup(item, this);
    insert(item, feedGroup);
    item->setPixmap(0, KGlobal::iconLoader()->loadIcon("folder", KIcon::Small) );
    modified = true;
    return feedGroup;
}

void FeedsCollection::removeFeed(QListViewItem *item)
{
    FeedGroup *feed = find(item);
    if (!feed)
    {
        KMessageBox::error( 0, i18n("Internal error while removing note.") );
        return;
    }

    for (QListViewItemIterator it(item); it.current() && (it.current() == item || it.current()->depth() > item->depth()); ++it)
        find(it.current())->destroy();
    delete item;
    modified = true;
}

bool FeedsCollection::changeFeed(QListViewItem */*item*/)
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
