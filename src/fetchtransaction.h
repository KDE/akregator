/*
    This file is part of Akregator.

    Copyright (C) 2004 Sashmit Bhaduri <smt@vfemail.net>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef FETCHTRANSACTION_H
#define FETCHTRANSACTION_H

#include <qobject.h>
#include <qdict.h>
#include <qptrdict.h>

#include "feed.h"

namespace Akregator {

class FetchTransaction : public QObject
{
    Q_OBJECT

    public:
        FetchTransaction(QObject *parent);
        ~FetchTransaction();

        /** starts the transaction when not already running */

        void start();
        
        /** stops a running transaction and clears the fetch lists. Does nothing when not running, to reset a not-running transaction call @ref clear(). */

        void stop();

        /** clear the fetch lists and resets the fetch transaction. Does nothing when running, you have to call @ref stop() then */

        void clear();

        /** returns whether the transaction is in state @c running */
        
        bool isRunning() { return m_running; }
        
        int fetchesDone() { return m_fetchesDone; }
        int totalFetches() { return m_totalFetches; }

        void addFeed(Feed *f);
        void addIcon(Feed *f);
        void addImage(Feed *f, RSS::Image *i);

        

    signals:
        
        void completed();
        void fetched(Feed*);
        void fetchError(Feed*);

    protected: 

        void feedDone(Feed *f);
        
        void connectToFeed(Feed* feed);
        void disconnectFromFeed(Feed* feed);
        void startFetchIcons();
        void startFetchImages();
        
    protected slots:
        
        void slotNodeDestroyed(TreeNode* node);
        void slotFeedFetched(Feed *);
        void slotFetchError(Feed *);
        void slotFetchAborted(Feed *);
        void slotFetchNextFeed();
        void slotFetchNextIcon();
        void slotFetchNextImage();
        
        void slotFaviconFetched(const QString &, const QPixmap &);
        void slotImageFetched(const QPixmap &);

    private: 

        QPtrList<Feed> m_fetchList;
        QPtrList<Feed> m_currentFetches;

        QPtrList<Feed> m_iconFetchList;
        QPtrList<Feed> m_currentIconFetches;
        QDict<Feed> m_iconFetchDict;
        

        QPtrList<RSS::Image> m_imageFetchList;
        QPtrList<RSS::Image> m_currentImageFetches;
        QPtrDict<Feed> m_imageFetchDict;

        int m_totalFetches;
        int m_fetchesDone;

        int m_concurrentFetches;
        bool m_running;

};

} // namespace Akregator

#endif

// vim: set et ts=4 sts=4 sw=4:

