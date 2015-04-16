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

#ifndef AKREGATOR_H
#define AKREGATOR_H

#include "aggregator.h"
#include "subscriptionlist.h"

namespace feedsync
{

class Akregator: public Aggregator
{
    Q_OBJECT

public:

    explicit Akregator(QObject *parent = Q_NULLPTR);
    ~Akregator();
    SubscriptionList getSubscriptionList() const;
    void load();
    void add(const SubscriptionList &list);
    void update(const SubscriptionList &list);
    void remove(const SubscriptionList &list);

private:
    SubscriptionList _subscriptionList;

protected Q_SLOTS:
    void sendSignalLoadDone();

Q_SIGNALS:
    void loadDone();
    void addDone();
    void updateDone();
    void removeDone();
};

}

#endif
