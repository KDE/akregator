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

#ifndef AKREGATOR_PROGRESSMANAGER_H
#define AKREGATOR_PROGRESSMANAGER_H

#include <krss/feed.h>
#include <QObject>

namespace boost {
template <typename T> class shared_ptr;
}

namespace KRss {
class FeedList;
}

namespace Akregator
{

class ProgressItemHandler;

/** This class manages the progress items for all feeds */

class ProgressManager : public QObject
{
    Q_OBJECT
    public:
        static ProgressManager* self();

        /** sets the feed list to be managed */
        void setFeedList( const boost::shared_ptr<KRss::FeedList>& feedList );
        /** adds a job to track the progress of */
        void addJob( KJob *job );

    protected slots:
        void slotFeedAdded( const KRss::Feed::Id& id );
        void slotFeedRemoved( const KRss::Feed::Id& id );

    private:
        friend class ProgressManagerPrivate;
        ProgressManager();
        ~ProgressManager();
        static ProgressManager* m_self;
};

/** this class handles the creation and deletion of progress items for one feed.
    This is an internal class intended to be used in ProgressManager only */

class FetchProgressItemHandler : public QObject
{
    Q_OBJECT
    public:
        explicit FetchProgressItemHandler( const boost::shared_ptr<const KRss::Feed>& feed );
        ~FetchProgressItemHandler();

    public slots:
        void slotFetchStarted( const KRss::Feed::Id& id );
        void slotFetchPercent( const KRss::Feed::Id& id, uint percentage );
        void slotFetchFinished( const KRss::Feed::Id& id );
        void slotFetchFailed( const KRss::Feed::Id& id, const QString& errorMessage );
        void slotFetchAborted( const KRss::Feed::Id& id );

    private:
        class FetchProgressItemHandlerPrivate;
        FetchProgressItemHandlerPrivate* d;
};

/** this class handles the creation and deletion of progress items for one item listing job.
    This is an internal class intended to be used in ProgressManager only
    note: deletes itself once the job is finished */
class JobProgressItemHandler : public QObject
{
    Q_OBJECT
    public:
        explicit JobProgressItemHandler( const KJob *job );
        ~JobProgressItemHandler();

    public slots:
        void slotJobPercent( KJob *job, unsigned long percentage );
        void slotJobResult( KJob *job );

    private:
        class JobProgressItemHandlerPrivate;
        JobProgressItemHandlerPrivate* d;
};

} // namespace Akregator

#endif // AKREGATOR_PROGRESSMANAGER_H
