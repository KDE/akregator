/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef AKREGATORARCHIVE_H
#define AKREGATORARCHIVE_H

namespace Akregator
{
    class Feed;

    class Archive
    {
        public:
            static void load(Feed *f);
            static void save(Feed *f);
    };
}

#endif
