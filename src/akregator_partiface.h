/*
    This file is part of Akregator.

    Copyright (C) 2004 Sashmit Bhaduri <smt@vfemail.net>

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

#ifndef AKREGATORPARTIFACE_H
#define AKREGATORPARTIFACE_H

#include <dcopobject.h>
#include <qstringlist.h>

namespace Akregator {

    class AkregatorPartIface : virtual public DCOPObject
    {
        K_DCOP
        k_dcop:
            virtual void openStandardFeedList() = 0;
            virtual void fetchFeedUrl(const QString&) = 0;
            virtual void fetchAllFeeds() = 0;
            virtual void saveSettings() = 0;
            virtual void addFeedsToGroup(const QStringList&, const QString&) = 0;
    };

}

#endif
