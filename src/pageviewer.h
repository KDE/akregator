/***************************************************************************
 *   Copyright (C) 2004 by Sashmit Bhaduri                                 *
 *   smt@vfemail.net                                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef PAGEVIEWER_H
#define PAGEVIEWER_H

#include "viewer.h"

#include <qdatetime.h>
class KAction;
class KToolBarPopupAction;

namespace Akregator
{
    // taken from KDevelop
    struct PageViewerHistoryEntry
    {
        KURL url;
        int id;
    
        PageViewerHistoryEntry() {}
        PageViewerHistoryEntry( const KURL& u ): url( u )
        {
            id = abs( QTime::currentTime().msecsTo( QTime() ) );    // nasty, but should provide a reasonably unique number
        }
    };

    // the back/forward navigation was taken from KDevelop. Kudos to the KDevelop team!
    class PageViewer : public Viewer
    {
        Q_OBJECT
        public:
            PageViewer(QWidget* parent, const char* name);
            
            virtual bool openURL(const KURL &url);

        public slots:
            void slotBack();
            void slotForward();
            void slotReload();
            void slotStop();
            void slotPrint();


        protected:

            void addHistoryEntry();
            
        protected slots:
            void slotStarted(KIO::Job *);
            void slotCompleted();
            void slotCancelled(const QString &errMsg);
            void slotBackAboutToShow();
            void slotForwardAboutToShow();
            void slotPopupActivated( int id );
            void popup( const QString & url, const QPoint & p );
            void slotCopy();
            void slotSelectionChanged();
            
        private:
            QValueList<PageViewerHistoryEntry> m_history;
            QValueList<PageViewerHistoryEntry>::Iterator m_current;
            
            KToolBarPopupAction* m_backAction;
            KToolBarPopupAction* m_forwardAction;
            KAction* m_reloadAction;
            KAction* m_stopAction;
            KAction* m_printAction;
            KAction* m_copyAction;

            bool m_restoring;
            
        private slots:
            bool slotOpenURLRequest(const KURL& url, const KParts::URLArgs& args);
            
    };
}

#endif // PAGEVIEWER_H
