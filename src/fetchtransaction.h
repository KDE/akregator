/***************************************************************************
 *  Copyright (C) 2004 by Sashmit Bhaduri (smt@vfemail.net)                *
 *                                                                         *
 *  Licensed under GPL.                                                    *
 ***************************************************************************/

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

        void start();
        void stop();

        void addFeed(Feed *f);

        int fetchesDone() { return m_fetchesDone; }
        int totalFetches() { return m_totalFetches; }

        void loadIcon(Feed *f);
        void startFetchIcons();

        void loadImage(Feed *f, RSS::Image *i);
        void startFetchImages();

        bool isRunning() { return m_running; }

    signals:
        void completed();
        void fetched(Feed*);
        void fetchError(Feed*);

    protected: // methods

        void connectToFeed(Feed* feed);
        void disconnectFromFeed(Feed* feed);

    protected slots:
        
        void slotNodeDestroyed(TreeNode* node);
        
    private: // methods
        void doFetch(int c);
        void feedDone(Feed *f);

        void doFetchIcon(int c);
        void doFetchImage(int c);


    private slots:
        void slotFeedFetched(Feed *);
        void slotFetchError(Feed *);
        void slotFetchAborted(Feed *);

        void slotFaviconFetched(const QString &, const QPixmap &);
        void slotImageFetched(const QPixmap &);

    private: // fields

        QPtrList<Feed> m_fetchList;
        QPtrList<Feed> m_currentFetches;

        QPtrList<Feed> m_iconFetchList;
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

