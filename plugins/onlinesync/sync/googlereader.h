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

#ifndef GOOGLEREADER_H
#define GOOGLEREADER_H

#include <KApplication>
#include <QMutex>

#include "aggregator.h"
#include "subscriptionlist.h"

class KConfigGroup;
class KJob;

namespace feedsync
{

class GoogleReader: public Aggregator
{
    Q_OBJECT

 public:
    explicit GoogleReader(const KConfigGroup& configgroup, QObject* parent=0);
    ~GoogleReader();
    SubscriptionList getSubscriptionList() const;
    void load();
    void add(const SubscriptionList & list);
    void update(const SubscriptionList & list);
    void remove(const SubscriptionList & list);

 protected Q_SLOTS:
    void slotAuthenticationDone(KJob*);
    void slotListDone(KJob*);
    void slotTokenDone(KJob*);
    void slotAddDone(KJob*);
    void slotRemoveDone(KJob*);

 private:
    QString _sid;
    QString _token;
    QString getSID() const;
    QString getUser() const;
    QString getPassword() const;
    QString getToken() const;
    void setUser(const QString& user);
    void setPassword(const QString& password);
    void setToken(const QString& token);
    SubscriptionList _subscriptionList;
    QString _user;
    QString _password;
    int _cursor;
    SubscriptionList _cursorList;
    void genError(const QString& msg);

 signals:
    void loadDone();
    void addDone();
    void updateDone();
    void removeDone();
    void error(const QString& msg);
};

}

#endif
