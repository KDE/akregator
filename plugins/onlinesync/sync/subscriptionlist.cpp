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

#include "subscriptionlist.h"

#include <qdebug.h>

namespace feedsync
{

SubscriptionList::SubscriptionList()
{
    qDebug();
}

SubscriptionList::~SubscriptionList()
{
    qDebug();
}

void SubscriptionList::add(const QString &iRss, const QString &iName, const QString &iCat)
{
    // qDebug() << iRss.left(20) << iName.left(10) << iCat.left(20);
    _rssList.append(iRss);
    _nameList.append(iName);
    _catListFull.append(iCat);
    if (!iCat.isEmpty()) {
        _catListSimple.append(iCat.split('/', QString::SkipEmptyParts).last());
    } else {
        _catListSimple.append(iCat);
    }
}

void SubscriptionList::remove(const QString &iRss, const QString &iName, const QString &iCat)
{
    // qDebug() << iRss.left(20) << iName.left(10) << iCat.left(10);
    int index = indexOf(iRss, iName, iCat);
    _rssList.removeAt(index);
    _nameList.removeAt(index);
    _catListSimple.removeAt(index);
    _catListFull.removeAt(index);
}

SubscriptionList SubscriptionList::compare(const SubscriptionList &iOther, ComparisonType diffType, RemovePolicy removePolicy) const
{
    qDebug();

    SubscriptionList diffList;

    // Rss/Cat to be added
    if (diffType == SubscriptionList::Added) {
        for (int i = 0; i < iOther.count(); i++) {
            QString m_rss = iOther.getRss(i);
            QString m_name = iOther.getName(i);
            QString m_cat = iOther.getCat(i);
            if (this->indexOf(m_rss, m_name, m_cat) < 0) {
                diffList.add(m_rss, m_name, m_cat);
                qDebug() << "(+)"
                         << "RSS:" << m_rss.left(10)
                         << "Name:" << m_name.left(10)
                         << "Cat:" << m_cat.left(10);
            }
        }
    }

    // Rss/Cat to be removed
    if ((diffType == SubscriptionList::Removed) && (removePolicy != Nothing)) {
        for (int i = 0; i < this->count(); i++) {
            QString m_rss = this->getRss(i);
            QString m_name = this->getName(i);
            QString m_cat = this->getCat(i);

            if ((iOther.indexOf(m_rss, m_name, m_cat) < 0)) {
                // If the feed is not here at all
                if ((iOther.indexOf(m_rss) < 0) && (removePolicy == Feed)) {
                    diffList.add(m_rss, m_name, m_cat);
                    qDebug() << "(-)"
                             << "RSS:" << m_rss.left(10)
                             << "Name:" << m_name.left(10)
                             << "Cat:" << m_cat.left(10);

                    // If this is only a new category for the feed
                } else if (iOther.indexOf(m_rss) > -1) {
                    diffList.add(m_rss, m_name, m_cat);
                    qDebug() << "(-)"
                             << "RSS:" << m_rss.left(10)
                             << "Name:" << m_name.left(10)
                             << "Cat:" << m_cat.left(10);
                }
            }
        }
    }

    return diffList;
}

int SubscriptionList::indexOf(const QString &iRss, const QString &iName, const QString &iCat) const
{
    Q_UNUSED(iName)

    for (int i = 0; i < this->count(); i++) {
        QString m_rss = this->getRss(i);
        QString m_cat = this->getCat(i);
        if ((m_rss.compare(iRss, Qt::CaseInsensitive) == 0) && (m_cat.compare(iCat, Qt::CaseInsensitive) == 0)) {
            return i;
        }
    }
    return -1;
}

int SubscriptionList::indexOf(const QString &iRss) const
{
    for (int i = 0; i < this->count(); i++) {
        QString m_rss = this->getRss(i);
        if (m_rss.compare(iRss, Qt::CaseInsensitive) == 0) {
            return i;
        }
    }

    return -1;
}

int SubscriptionList::count() const
{
    return _rssList.size();
}

int SubscriptionList::countRss(const QString &iRss) const
{
    return _rssList.count(iRss);
}

// Getters / Setters

QString SubscriptionList::getName(int index) const
{
    return _nameList.at(index);
}

QString SubscriptionList::getRss(int index) const
{
    return _rssList.at(index);
}

QString SubscriptionList::getCat(int index, CategoryFormat format) const
{
    if (format == Simple) {
        return _catListSimple.at(index);
    } else {
        return _catListFull.at(index);
    }
}

}
