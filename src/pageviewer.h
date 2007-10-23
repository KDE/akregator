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

#ifndef AKREGATOR_PAGEVIEWER_H
#define AKREGATOR_PAGEVIEWER_H

#include "viewer.h"

#include <QList>

class QString;

namespace Akregator {

// the back/forward navigation was taken from KDevelop. Kudos to the KDevelop team!
class PageViewer : public Viewer
{
    Q_OBJECT
    public:
        explicit PageViewer(QWidget* parent, const char* name=0);
        virtual ~PageViewer();
        virtual bool openURL(const KUrl &url);

        /** used by the BrowserRun object */
        virtual void openPage(const KUrl& url);

    protected:

        class HistoryEntry;
        void addHistoryEntry(const KUrl& url);
        void restoreHistoryEntry(const QList<HistoryEntry>::Iterator& entry);
        void updateHistoryEntry();

    protected slots:

        void slotSetCaption(const QString& cap);
        void slotBack();
        void slotForward();
        void slotBackAboutToShow();
        void slotForwardAboutToShow();
        void slotReload();
        void slotStop();

        virtual void slotPaletteOrFontChanged();

        void slotStarted(KIO::Job *);
        void slotCompleted();
        void slotCancelled(const QString &errMsg);
        
        void slotPopupActivated( int id );
        virtual void slotPopupMenu(const QPoint&, const KUrl&, mode_t, const KParts::URLArgs&, KParts::BrowserExtension::PopupFlags);
        
        void slotGlobalBookmarkArticle();
        
        virtual void slotOpenUrlRequest(const KUrl& url, const KParts::URLArgs& args);
        virtual void urlSelected(const QString &url, int button, int state, const QString &_target, KParts::URLArgs args);
        
    signals:
        void setTabIcon(const QPixmap&);

    private:
        class PageViewerPrivate;
        PageViewerPrivate* d;
};

} // namespace Akregator

#endif // AKREGATOR_PAGEVIEWER_H
