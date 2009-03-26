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

#include "akregator.h"
#include "subscriptionlist.h"

#include "feedlistmanagementinterface.h"

#include <kdebug.h>
#include <QStringList>
#include <QTimer>

namespace feedsync {

Akregator::Akregator( QObject* p ) : Aggregator( p ) 
{
    kDebug();
}

Akregator::~Akregator() 
{
    kDebug();
}

SubscriptionList Akregator::getSubscriptionList() const 
{
    kDebug();
    return _subscriptionList;
}

void Akregator::load() 
{
    kDebug();

    using namespace Akregator;

    FeedListManagementInterface * ak_feedlist = FeedListManagementInterface::instance();
    QStringList catlist = ak_feedlist->categories();
    for (int idcat=0;idcat<catlist.size();idcat++) {
        QStringList feedlist = ak_feedlist->feeds(catlist.at(idcat));
        for (int idfeed=0;idfeed<feedlist.size();idfeed++) {
            QString tmpcat;
            if (catlist.at(idcat).compare("1/")==0) {
                tmpcat = "";
            } else {
                tmpcat = ak_feedlist->getCategoryName(catlist.at(idcat));
            }
            _subscriptionList.add( feedlist.at(idfeed),
                                    feedlist.at(idfeed),
                                    tmpcat );
        }
    }

    // Send the signal
    QTimer::singleShot( 0, this, SLOT(sendSignalLoadDone()) );
}

void Akregator::sendSignalLoadDone() 
{
    emit loadDone();
}

void Akregator::add( const SubscriptionList & list) 
{
    kDebug();

    using namespace Akregator;

    for (int i=0; i<list.count(); i++) {
        kDebug() << list.getRss(i).left(20);

        FeedListManagementInterface * ak_feedlist = FeedListManagementInterface::instance();

        // Look for the category id
        QString foundCatId;
        if (list.getCat(i).isEmpty()) {
            foundCatId = "1";
        } else {
            QStringList catlist = ak_feedlist->categories();
            int idcat = 0;
            QChar slash('/');
            while (idcat<catlist.size() && foundCatId.isEmpty()) {
                QString ak_catId = catlist.at(idcat).split(slash,QString::SkipEmptyParts).last();
                QString ak_cat = ak_feedlist->getCategoryName(ak_catId).split(slash,QString::SkipEmptyParts).last();
                if (ak_cat.compare(list.getCat(i),Qt::CaseInsensitive)==0) {
                    foundCatId = ak_catId;
                }
                idcat++;
            }
        }

        // Cat not found --> Create
        if (foundCatId.isEmpty()) {
            foundCatId = ak_feedlist->addCategory( list.getCat(i), "1" );
        }

        // Add
        kDebug() << "Cat:" << foundCatId;
        ak_feedlist->addFeed(list.getRss(i),foundCatId);
    }

    // Emit signal
    emit addDone();
}

void Akregator::update(const SubscriptionList & list) 
{
    Q_UNUSED(list)
    kDebug();

    // Emit signal
    emit updateDone();
}

void Akregator::remove(const SubscriptionList & list) 
{
    kDebug();

    for (int i=0; i<list.count(); i++) {
        kDebug() << list.getRss(i).left(20);

        using namespace Akregator;

        FeedListManagementInterface * ak_feedlist = FeedListManagementInterface::instance();

        // Look for the category id
        QString foundCatId;
        QStringList catlist = ak_feedlist->categories();
        int idcat = 0;
        QChar slash('/');
        while (idcat<catlist.size() && foundCatId.isEmpty()) {
            QString ak_catId = catlist.at(idcat).split(slash,QString::SkipEmptyParts).last();
            QString ak_cat = ak_feedlist->getCategoryName(ak_catId).split(slash,QString::SkipEmptyParts).last();
            if (ak_cat.compare(list.getCat(i),Qt::CaseInsensitive)==0) {
                foundCatId = ak_catId;
            }
            idcat++;
        }

        // Remove
        ak_feedlist->removeFeed(list.getRss(i),foundCatId);
    }

    // Emit signal
    emit removeDone();
}

}
