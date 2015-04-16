/*
    This file is part of Akregator.

    Copyright (C) 2008 Didier Hoarau <did.hoarau@gmail.com>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef FEEDSYNC_H
#define FEEDSYNC_H

#include <QObject>

#include "subscriptionlist.h"

class KConfigGroup;

namespace feedsync
{

class Aggregator;

class FeedSync : public QObject
{
    Q_OBJECT

public:
    explicit FeedSync(QObject *parent = Q_NULLPTR);
    ~FeedSync();
    enum SyncType { Get, Send };

protected Q_SLOTS:
    void error(const QString &msg);
    void sync();
    void slotLoadDone();
    void slotAddDone();
    void slotRemoveDone();

private:
    Aggregator *_aggrSend;
    Aggregator *_aggrGet;
    Aggregator *createAggregatorFactory(const KConfigGroup &configgroup);
    int _syncstep;
    int _loadedAggrCount;
    SubscriptionList tmp_removelist;
    SubscriptionList tmp_addlist;
    void log();
};

}

#endif
