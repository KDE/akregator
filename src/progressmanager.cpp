/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <osterfeld@kde.org>

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

#include "progressmanager.h"

#include <krss/feedlist.h>

#include <QHash>
#include <QList>

#include <KLocale>
#include <KGlobal>
#include <libkdepim/progressmanager.h>

#include <boost/shared_ptr.hpp>

using namespace boost;

namespace Akregator {

class ProgressManagerPrivate
{
    public:
        shared_ptr<KRss::FeedList> feedList;
        QHash<KRss::Feed::Id, FetchProgressItemHandler*> handlers;
        ProgressManager instance;
};

K_GLOBAL_STATIC( ProgressManagerPrivate, d )

ProgressManager* ProgressManager::self()
{
    return &d->instance;
}

ProgressManager::ProgressManager()
{
}

ProgressManager::~ProgressManager()
{
}

void ProgressManager::setFeedList( const shared_ptr<KRss::FeedList>& feedList )
{
    if ( feedList == d->feedList )
        return;

    if ( d->feedList ) {
        qDeleteAll( d->handlers );
        d->handlers.clear();
        d->feedList.get()->disconnect( this );
    }

    d->feedList = feedList;

    if ( d->feedList ) {
        Q_FOREACH( const KRss::Feed::Id& id, d->feedList->feedIds() )
            slotFeedAdded( id );

        connect( feedList.get(), SIGNAL( feedAdded( const KRss::Feed::Id&) ),
                 this, SLOT( slotFeedAdded( const KRss::Feed::Id& ) ) );
        connect( feedList.get(), SIGNAL( feedRemoved( const KRss::Feed::Id&) ),
                 this, SLOT( slotFeedRemoved( const KRss::Feed::Id& ) ) );    }
}

void ProgressManager::addJob( KJob *job )
{
    // deletes itself
    new JobProgressItemHandler( job );
}

void ProgressManager::slotFeedAdded( const KRss::Feed::Id& id )
{
    if ( d->handlers.contains( id ) )
        return;

    const shared_ptr<const KRss::Feed> feed = d->feedList->constFeedById( id );
    assert( feed );

    d->handlers.insert( id, new FetchProgressItemHandler( feed ) );
}

void ProgressManager::slotFeedRemoved( const KRss::Feed::Id& id )
{
    delete d->handlers.take( id );
}

class FetchProgressItemHandler::FetchProgressItemHandlerPrivate
{
    public:
        explicit FetchProgressItemHandlerPrivate( const shared_ptr<const KRss::Feed>& feed_ )
            : feed( feed_ ), progressItem( 0 ) {}
        const shared_ptr<const KRss::Feed> feed;
        KPIM::ProgressItem* progressItem;
};

FetchProgressItemHandler::FetchProgressItemHandler( const shared_ptr<const KRss::Feed>& feed )
    : d( new FetchProgressItemHandlerPrivate( feed ) )
{
    connect( feed.get(), SIGNAL( fetchStarted( const KRss::Feed::Id& ) ),
             this, SLOT( slotFetchStarted( const KRss::Feed::Id& ) ) );
    connect( feed.get(), SIGNAL( fetchPercent( const KRss::Feed::Id&, uint ) ),
             this, SLOT( slotFetchPercent( const KRss::Feed::Id&, uint ) ) );
    connect( feed.get(), SIGNAL( fetchFinished( const KRss::Feed::Id& ) ),
             this, SLOT( slotFetchFinished( const KRss::Feed::Id& ) ) );
    connect( feed.get(), SIGNAL( fetchFailed( const KRss::Feed::Id&, const QString& ) ),
             this, SLOT( slotFetchFailed( const KRss::Feed::Id&, const QString& ) ) );
    connect( feed.get(), SIGNAL( fetchAborted( const KRss::Feed::Id& ) ),
             this, SLOT( slotFetchAborted( const KRss::Feed::Id& ) ) );
}

FetchProgressItemHandler::~FetchProgressItemHandler()
{
    if ( d->progressItem ) {
        d->progressItem->setComplete();
        d->progressItem = 0;
    }

    delete d;
    d = 0;
}

void FetchProgressItemHandler::slotFetchStarted( const KRss::Feed::Id& id )
{
    Q_UNUSED( id )
    if ( d->progressItem ) {
        d->progressItem->setStatus( i18n( "Fetching" ) );
        d->progressItem->setComplete();
        d->progressItem = 0;
    }

    d->progressItem = KPIM::ProgressManager::createProgressItem( KPIM::ProgressManager::getUniqueID(),
                                                                 d->feed->title() );

    connect( d->progressItem, SIGNAL( progressItemCanceled( KPIM::ProgressItem* ) ),
             d->feed.get(), SLOT( abortFetch() ) );
}

void FetchProgressItemHandler::slotFetchPercent( const KRss::Feed::Id& id, uint percentage )
{
    Q_UNUSED( id )
    if ( d->progressItem ) {
        d->progressItem->setStatus( i18n( "Fetching" ) );
        d->progressItem->setProgress( percentage );
    }
}

void FetchProgressItemHandler::slotFetchFinished( const KRss::Feed::Id& id )
{
    Q_UNUSED( id )
    if ( d->progressItem ) {
        d->progressItem->setStatus(i18n("Fetch completed"));
        d->progressItem->setComplete();
        d->progressItem = 0;
    }
}

void FetchProgressItemHandler::slotFetchFailed( const KRss::Feed::Id& id, const QString& errorMessage )
{
    Q_UNUSED( id )
    if ( d->progressItem ) {
        d->progressItem->setStatus( i18n( "Fetch error. %1", errorMessage ) );
        d->progressItem->setComplete();
        d->progressItem = 0;
    }
}

void FetchProgressItemHandler::slotFetchAborted( const KRss::Feed::Id& id )
{
    Q_UNUSED( id )
    if ( d->progressItem ) {
        d->progressItem->setStatus(i18n("Fetch aborted"));
        d->progressItem->setComplete();
        d->progressItem = 0;
    }
}

class JobProgressItemHandler::JobProgressItemHandlerPrivate
{
    public:
        explicit JobProgressItemHandlerPrivate( const KJob *job_ )
            : job( job_ ), progressItem( 0 ) {}
        const KJob * const job;
        KPIM::ProgressItem* progressItem;
};

JobProgressItemHandler::JobProgressItemHandler( const KJob *job )
    : d( new JobProgressItemHandlerPrivate( job ) )
{
    connect( d->job, SIGNAL( result( KJob* ) ),
             this, SLOT( slotJobResult( KJob* ) ) );
    connect( d->job, SIGNAL(destroyed(QObject*)),
             this, SLOT(slotJobDestroyed()) );
    connect( d->job, SIGNAL( percent( KJob*, unsigned long ) ),
             this, SLOT( slotJobPercent( KJob*, unsigned long ) ) );

    d->progressItem = KPIM::ProgressManager::createProgressItem( KPIM::ProgressManager::getUniqueID(),
                                                                 i18n( "Item Loading" ), QString(),
                                                                 false );
    d->progressItem->setStatus( i18n( "Loading" ) );
    d->progressItem->setProgress( 0 );
}

JobProgressItemHandler::~JobProgressItemHandler()
{
    if ( d->progressItem ) {
        d->progressItem->setComplete();
        d->progressItem = 0;
    }

    delete d;
    d = 0;
}

void JobProgressItemHandler::slotJobPercent( KJob *job, unsigned long percentage )
{
    Q_UNUSED( job )
    if ( d->progressItem ) {
        d->progressItem->setStatus( i18n( "Loading" ) );
        d->progressItem->setProgress( percentage );
    }
}

void JobProgressItemHandler::slotJobResult( KJob *job )
{
    Q_UNUSED( job )
    job->disconnect( this );
    if ( d->progressItem ) {
        if ( job->error() )
            d->progressItem->setStatus( i18n( "Load error. %1", job->errorString() ) );
        else
            d->progressItem->setStatus( i18n( "Load completed" ) );
        d->progressItem->setComplete();
        d->progressItem = 0;
    }

    deleteLater();
}

void JobProgressItemHandler::slotJobDestroyed()
{
    if ( d->progressItem ) {
        d->progressItem->setStatus( i18n( "Canceled" ) );
        d->progressItem->setComplete();
        d->progressItem = 0;
    }

    deleteLater();
}

} // namespace Akregator

#include "progressmanager.moc"
