/*
    This file is part of Akregator.

    Copyright (C) 2008 Frank Osterfeld <osterfeld@kde.org>

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

#ifndef AKREGATOR_SUBSCRIPTIONLISTJOBS_H
#define AKREGATOR_SUBSCRIPTIONLISTJOBS_H

#include <KJob>

#include <boost/weak_ptr.hpp>

#include "akregator_export.h"

namespace Akregator
{

class FeedList;

//transitional job classes

class AKREGATOR_EXPORT  MoveSubscriptionJob : public KJob
{
    Q_OBJECT
public:
    explicit MoveSubscriptionJob(QObject *parent = 0);

    void setSubscriptionId(int id);
    void setDestination(int folder, int afterChild);

    void start();

private Q_SLOTS:
    void doMove();

private:
    int m_id;
    int m_destFolderId;
    int m_afterId;
    boost::weak_ptr<FeedList> m_feedList;
};

class AKREGATOR_EXPORT RenameSubscriptionJob : public KJob
{
    Q_OBJECT
public:
    explicit RenameSubscriptionJob(QObject *parent = 0);

    void setSubscriptionId(int id);
    void setName(const QString &name);

    void start();

private Q_SLOTS:
    void doRename();

private:
    int m_id;
    QString m_name;
    boost::shared_ptr<FeedList> m_feedList;
};

class AKREGATOR_EXPORT DeleteSubscriptionJob : public KJob
{
    Q_OBJECT
public:
    explicit DeleteSubscriptionJob(QObject *parent = 0);

    void setSubscriptionId(int id);

    void start();

private Q_SLOTS:
    void doDelete();

private:
    int m_id;
    boost::weak_ptr<FeedList> m_feedList;
};

}
#endif // AKREGATOR_SUBSCRIPTIONLISTJOBS_H
