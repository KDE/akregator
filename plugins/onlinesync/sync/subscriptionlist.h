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

#ifndef SUBSCRIPTIONLIST_H
#define SUBSCRIPTIONLIST_H

#include <QStringList>
#include <KApplication>

namespace feedsync
{

class SubscriptionList
{
 public:
    explicit SubscriptionList();
    ~SubscriptionList();
    void add(const QString& iRss, const QString& iName, const QString& iCat);
    void remove(const QString& iRss, const QString& iName, const QString& iCat);

    enum ComparisonType {
        Added, Removed
    };
    enum RemovePolicy {
        Nothing, Category, Feed
    };
    enum CategoryFormat {
        Simple, Full
    };

    SubscriptionList compare(const SubscriptionList & iOther, ComparisonType diffType, RemovePolicy removePolicy=Nothing) const;
    bool rssExist(const QString& iRss,const QString& iName,const QString& iCat) const;
    QString getRss(int index) const;
    QString getName(int index) const;
    QString getCat(int index, CategoryFormat format=Simple) const;

    bool isEmpty() const { return count() == 0; }
    int count() const;
    int countRss(const QString& iRss) const;
    int indexOf(const QString& iRss, const QString& iName, const QString& iCat) const;
    int indexOf(const QString& iRss) const;



 private:
    QStringList _rssList;
    QStringList _nameList;
    QStringList _catListFull;
    QStringList _catListSimple;
};

}

#endif
