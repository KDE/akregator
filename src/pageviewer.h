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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef PAGEVIEWER_H
#define PAGEVIEWER_H

#include "viewer.h"
//Added by qt3to4:
#include <QPixmap>
#include <Q3ValueList>


class KAction;
class KToolBarPopupAction;
class QString;

namespace Akregator
{
    
    // the back/forward navigation was taken from KDevelop. Kudos to the KDevelop team!
    class PageViewer : public Viewer
    {
        Q_OBJECT
        public:
            PageViewer(QWidget* parent, const char* name);
            virtual ~PageViewer();
            virtual bool openURL(const KURL &url);

            /** used by the BrowserRun object */
            virtual void openPage(const KURL& url);

        protected:

            class HistoryEntry;
            void addHistoryEntry(const KURL& url);
            void restoreHistoryEntry(const Q3ValueList<HistoryEntry>::Iterator& entry);
            void updateHistoryEntry();

        protected slots:

            void slotSetCaption(const QString& cap);
            void slotBack();
            void slotForward();
            void slotReload();
            void slotStop();

            virtual void slotPaletteOrFontChanged();

            void slotStarted(KIO::Job *);
            void slotCompleted();
            void slotCancelled(const QString &errMsg);
            void slotBackAboutToShow();
            void slotForwardAboutToShow();
            void slotPopupActivated( int id );
            virtual void slotPopupMenu(KXMLGUIClient*, const QPoint&, const KURL&, const KParts::URLArgs&, KParts::BrowserExtension::PopupFlags, mode_t);
            
            void slotGlobalBookmarkArticle();
            
            virtual void slotOpenURLRequest(const KURL& url, const KParts::URLArgs& args);
            virtual void urlSelected(const QString &url, int button, int state, const QString &_target, KParts::URLArgs args);
            
        signals:
            void setTabIcon(const QPixmap&);

        private:
            class PageViewerPrivate;
            PageViewerPrivate* d;
    };
}

#endif // PAGEVIEWER_H
