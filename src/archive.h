/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef AKREGATORARCHIVE_H
#define AKREGATORARCHIVE_H

#include "akregatorconfig.h"
namespace Akregator
{
    class Feed;
    class FeedGroup;
    class TreeNode;
    
    class Archive
    {
        public:
            static void load(TreeNode* node);
            static void save(TreeNode* node);
            
        protected:
            static void load_p(Feed* f);
            static void save_p(Feed* f);
            static void load_p(FeedGroup* fg);
            static void save_p(FeedGroup* fg);
    };
}

#endif
