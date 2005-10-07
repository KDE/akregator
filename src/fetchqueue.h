/*
    This file is part of Akregator.

    Copyright (C) 2004 Sashmit Bhaduri <smt@vfemail.net>
                  2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

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

#ifndef AKREGATOR_FETCHQUEUE_H
#define AKREGATOR_FETCHQUEUE_H

#include <qobject.h>

namespace Akregator {

class Feed;
class TreeNode;

class FetchQueue : public QObject
{
    Q_OBJECT

    public:

        FetchQueue(QObject* parent=0, const char* name=0);
        virtual ~FetchQueue();

        /** returns true when no feeds are neither fetching nor queued */
        bool isEmpty() const;
        
        /** adds a feed to the queue */
        void addFeed(Feed *f);

    public slots:
    
        /** aborts currently fetching feeds and empties the queue */
        void slotAbort();
        
    signals:

        void signalStarted();
        void signalStopped();
        void fetched(Feed*);
        void fetchError(Feed*);

    protected: 

        /** fetches the next feed in the queue, unless the maximum of concurrent fetches is reached */
        void fetchNextFeed();
        
        void feedDone(Feed *f);
        void connectToFeed(Feed* feed);
        void disconnectFromFeed(Feed* feed);

    protected slots:
        
        void slotNodeDestroyed(TreeNode* node);
        void slotFeedFetched(Feed *);
        void slotFetchError(Feed *);
        void slotFetchAborted(Feed *);
        
    private:

        class FetchQueuePrivate;
        FetchQueuePrivate* d;
};

} // namespace Akregator

#endif

// vim: set et ts=4 sts=4 sw=4:

