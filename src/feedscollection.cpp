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

void FeedsCollection::addFeed(QListViewItem *item)
{
    insert(item, new Feed(item, this));
    item->setPixmap(0, KGlobal::iconLoader()->loadIcon("txt", KIcon::Small) );
    QListViewItem *parent = item->parent();
    if(parent) find(parent)->updateView();
    modified = true;
}

void FeedsCollection::addFeedGroup(QListViewItem *item)
{
    insert(item, new FeedGroup(item, this));
    item->setPixmap(0, KGlobal::iconLoader()->loadIcon("folder", KIcon::Small) );
    QListViewItem *parent = item->parent();
    if(parent) find(parent)->updateView();
    modified = true;
}

void FeedsCollection::removeFeed(QListViewItem *item)
{
    FeedGroup *feed = find(item);
    if (!feed)
    {
        KMessageBox::error( 0, i18n("Internal error while removing note") );
        return;
    }

    QListViewItem *parent = item->parent();
    for (QListViewItemIterator it(item); it.current() && (it.current() == item || it.current()->depth() > item->depth()); ++it)
        find(it.current())->destroy();
    delete item;
    if (parent) find(parent)->updateView();
    modified = true;
}

bool FeedsCollection::changeFeed(QListViewItem *item)
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
