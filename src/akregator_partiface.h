/***************************************************************************
 *   Copyright (C) 2004 by Sashmit Bhaduri                                 *
 *   smt@vfemail.net                                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef AKREGATORPARTIFACE_H
#define AKREGATORPARTIFACE_H

#include <dcopobject.h>

namespace Akregator {

    class AkregatorPartIface : virtual public DCOPObject
    {
        K_DCOP
        k_dcop:
            virtual void openStandardFeedList() = 0;
            virtual void fetchFeedUrl(const QString&) = 0;
            virtual void fetchAllFeeds() = 0;
            virtual void saveSettings() = 0;
            virtual void addFeedToGroup(const QString&, const QString&) = 0;
    };

}

#endif
