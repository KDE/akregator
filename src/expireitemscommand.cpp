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

#include "expireitemscommand.h"

#include "articlejobs.h"
#include "feed.h"
#include "feedlist.h"

#include <KDebug>

#include <QSet>
#include <QTimer>

#include <boost/shared_ptr.hpp>

#include <cassert>

using namespace boost;
using namespace Akregator;

class ExpireItemsCommand::Private
{
    ExpireItemsCommand* const q;
public:
    explicit Private( ExpireItemsCommand* qq );

    void createDeleteJobs();
    void addDeleteJobForFeed( Feed* feed );
    void jobFinished( KJob* );

    weak_ptr<FeedList> m_feedList;
    QVector<int> m_feeds;
    QSet<KJob*> m_jobs;
};

ExpireItemsCommand::Private::Private( ExpireItemsCommand* qq ) : q( qq ), m_feedList()
{

}

void ExpireItemsCommand::Private::addDeleteJobForFeed( Feed* feed )
{
    assert( feed );
    ArticleDeleteJob* job = new ArticleDeleteJob( q );
    connect( job, SIGNAL(finished(KJob*)), q, SLOT(jobFinished(KJob*)) );
    m_jobs.insert( job );
    feed->deleteExpiredArticles( job );
    job->start();
}

void ExpireItemsCommand::Private::jobFinished( KJob* job )
{
    assert( !m_jobs.isEmpty() );
    m_jobs.remove( job );
    emit q->progress( ( ( m_feeds.count() - m_jobs.count() ) * 100 ) / m_feeds.count(), QString() );
    if ( m_jobs.isEmpty() )
        q->done();
}

void ExpireItemsCommand::Private::createDeleteJobs()
{
    assert( m_jobs.isEmpty() );
    const shared_ptr<FeedList> feedList = m_feedList.lock();

    if ( m_feeds.isEmpty() || !feedList )
    {
        if ( !feedList )
            kWarning() << "Associated feed list was deleted, could not expire items";
        q->done();
        return;
    }

    Q_FOREACH ( const int i, m_feeds )
    {
        Feed* const feed = qobject_cast<Feed*>( feedList->findByID( i ) );
        if ( feed )
            addDeleteJobForFeed( feed );
    }
}

ExpireItemsCommand::ExpireItemsCommand( QObject* parent ) : Command( parent ), d( new Private( this ) )
{
}

ExpireItemsCommand::~ExpireItemsCommand()
{
    delete d;
}

void ExpireItemsCommand::setFeedList( const weak_ptr<FeedList>& feedList )
{
    d->m_feedList = feedList;
}

weak_ptr<FeedList> ExpireItemsCommand::feedList() const
{
    return d->m_feedList;
}

void ExpireItemsCommand::setFeeds( const QVector<int>& feeds )
{
    d->m_feeds = feeds;
}

QVector<int> ExpireItemsCommand::feeds() const
{
    return d->m_feeds;
}

void ExpireItemsCommand::doAbort()
{
    Q_FOREACH( KJob* const i, d->m_jobs )
        i->kill();
}

void ExpireItemsCommand::doStart()
{
    QTimer::singleShot( 0, this, SLOT(createDeleteJobs()) );
}

#include "moc_expireitemscommand.cpp"
