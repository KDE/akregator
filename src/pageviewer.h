/***************************************************************************
 *   Copyright (C) 2004 by Sashmit Bhaduri                                 *
 *   smt@vfemail.net                                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef PAGEVIEWER_H
#define PAGEVIEWER_H

#include <cstdlib>
using std::abs;

#include <qdatetime.h>
#include <qstring.h>

#include "viewer.h"


class KAction;
class KToolBarPopupAction;


namespace Akregator
{
    
    // taken from KDevelop
    struct PageViewerHistoryEntry
    {
        KURL url;
        QString title;
        int id;
    
        PageViewerHistoryEntry() {}
        PageViewerHistoryEntry(const KURL& u, const QString& t=QString::null): url(u), title(t)
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
            virtual ~PageViewer();
            virtual bool openURL(const KURL &url);

        public slots:
            void slotBack();
            void slotForward();
            void slotReload();
            void slotStop();
            void slotSetCaption(const QString& cap);
            virtual void slotPaletteOrFontChanged();

        protected:

            void addHistoryEntry(const KURL& url);
            
        protected slots:
            void slotStarted(KIO::Job *);
            void slotCompleted();
            void slotCancelled(const QString &errMsg);
            void slotBackAboutToShow();
            void slotForwardAboutToShow();
            void slotPopupActivated( int id );
            virtual void slotPopupMenu(KXMLGUIClient*, const QPoint&, const KURL&, const KParts::URLArgs&, KParts::BrowserExtension::PopupFlags, mode_t);
            
            void slotGlobalBookmarkArticle();
            void formClicked(const KURL& url, const KParts::URLArgs& args);

            virtual void slotOpenURLRequest(const KURL& url, const KParts::URLArgs& args);
            
        signals:
            void setTabIcon(const QPixmap&);

        private:
            QValueList<PageViewerHistoryEntry> m_history;
            QValueList<PageViewerHistoryEntry>::Iterator m_current;
            
            KToolBarPopupAction* m_backAction;
            KToolBarPopupAction* m_forwardAction;
            KAction* m_reloadAction;
            KAction* m_stopAction;

            bool m_restoring;
            QString m_caption;
            

    };
}

#endif // PAGEVIEWER_H
