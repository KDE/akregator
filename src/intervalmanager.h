/***************************************************************************
 *   Copyright (C) 2004 by Sashmit Bhaduri                                 *
 *   smt@vfemail.net                                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef INTERVALMGR_H
#define INTERVALMGR_H

#include <qobject.h>
#include <qmap.h>

class QTimer;

namespace Akregator
{
    class IntervalManager : public QObject
    {
        Q_OBJECT
        
        public:
            static IntervalManager *self();
            void read();
            void feedFetched(const QString&);
            uint lastFetchTime(const QString&);
    
        public slots:
            void sync();
            
        protected:
            IntervalManager (QObject * = 0L, const char * = 0L);
            ~IntervalManager ();

      private:
            static IntervalManager *m_instance;
            QMap<QString, int> m_siteMap;
            QTimer *m_syncTimer;
    };
}

#endif

// vim: set et ts=4 sts=4 sw=4:
