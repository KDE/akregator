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
    class TreeNode;
    
    class FeedsCollection : public QPtrDict<TreeNode>
    {
        public:
            FeedsCollection();
            ~FeedsCollection();

            void addFeed(Feed *f);
            Feed* addFeed(QListViewItem *item);
            FeedGroup* addFeedGroup(QListViewItem *item);

            void removeFeed(QListViewItem *item);
            bool changeFeed(QListViewItem *item);

            void clearFeeds();
            void updateFeedsView();

        private:
            bool modified;
    };
}

#endif
