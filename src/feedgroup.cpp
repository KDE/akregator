/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
#include "feedgroup.h"

namespace Akregator {

FeedGroup::FeedGroup(QListViewItem *i, FeedsCollection *coll)
    : QObject(0, "some_random_name")
{
}


FeedGroup::~FeedGroup()
{
}

};

#include "feedgroup.moc"
