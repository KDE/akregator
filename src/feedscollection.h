/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
#ifndef AKREGATORFEEDSCOLLECTION_H
#define AKREGATORFEEDSCOLLECTION_H

#include <qptrdict.h>



namespace Akregator
{
    class Feed;
    class FeedGroup;
    class TreeNodeItem;
    class TreeNode;
    
    class FeedsCollection : public QPtrDict<TreeNode>
    {
        public:
            FeedsCollection();
            ~FeedsCollection();

            void addFeed(Feed *f);
            void addFeedGroup(FeedGroup* feedGroup);
        
            void removeFeed(TreeNodeItem *item);
            bool changeFeed(TreeNodeItem *item);

            void clearFeeds();
            void updateFeedsView();

        private:
            bool modified;
    };
}

#endif
