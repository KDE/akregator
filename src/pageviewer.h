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
