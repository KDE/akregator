/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
#ifndef AKREGATORFEEDSCOLLECTION_H
#define AKREGATORFEEDSCOLLECTION_H

#include <qptrdict.h>

class QListViewItem;

namespace Akregator
{
    class Feed;
    class FeedGroup;

    class FeedsCollection : public QPtrDict<FeedGroup>
    {
        public:
            FeedsCollection();
            ~FeedsCollection();

            void addFeed(QListViewItem *item);
            void addFeedGroup(QListViewItem *item);

            void removeFeed(QListViewItem *item);
            bool changeFeed(QListViewItem *item);

            void clearFeeds();

            void updateFeedsView();

        private:
            bool modified;
    };
}

#endif
