/***************************************************************************
 *   Copyright (C) 2004 by Sashmit Bhaduri                                 *
 *   smt@vfemail.net                                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef FEEDICONMGR_H
#define FEEDICONMGR_H

#include <qobject.h>
#include <qpixmap.h>

#include <dcopobject.h>
#include <kurl.h>

class DCOPClient;


namespace Akregator
{
    class FeedIconManager:public QObject, public DCOPObject
    {
        Q_OBJECT
        K_DCOP
        
        public:
            static FeedIconManager *self();
            void loadIcon(const QString &);
            QString iconLocation(const KURL &) const;
            
        k_dcop:
            void slotIconChanged(bool, QString, QString);

        signals:
            void iconChanged(const QString &, const QPixmap &);

        protected:
            FeedIconManager(QObject * = 0L, const char * = 0L);
            ~FeedIconManager();

      private:
            static FeedIconManager *m_instance;
    };
}

#endif
