/*
    This file is part of Akregator.

    Copyright (C) 2004 Teemu Rytilahti <tpr@d5k.net>

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

#ifndef VIEWER_H
#define VIEWER_H

#include <khtml_part.h>

class KURL;

namespace KIO
{
    class MetaData;
}

namespace Akregator
{
    class Viewer : public KHTMLPart
    {
        Q_OBJECT
        public:
            Viewer(QWidget* parent, const char* name);
            virtual ~Viewer();

            virtual bool closeURL();
            virtual bool openURL(const KURL &);
            
            /** used by the BrowserRun object to call KHTMLPart::openURL() */
            void openPage(const KURL& url) { KHTMLPart::openURL(url);}

        public slots:
            
            void slotScrollUp(); 
            void slotScrollDown();
            void slotZoomIn();
            void slotZoomOut();
            void slotSetZoomFactor(int percent);
            void slotPrint();
            void setSafeMode();

            virtual void slotPaletteOrFontChanged() = 0;

        signals:
            /** This gets emitted when url gets clicked */
            void urlClicked(const KURL& url, bool background=false);

            protected: // methods
            int pointsToPixel(int points) const;

        protected slots:
            virtual void slotOpenURLRequest(const KURL& url, const KParts::URLArgs& args);
            virtual void slotPopupMenu(KXMLGUIClient*, const QPoint&, const KURL&, const KParts::URLArgs&, KParts::BrowserExtension::PopupFlags, mode_t);

            /** Copies current link to clipboard. */
            void slotCopyLinkAddress();

            /** Copies currently selected text to clipboard */
            virtual void slotCopy();

            /** Opens @c m_url inside this viewer */
            virtual void slotOpenLinkInternal();

            /** Opens @c m_url in external viewer, eg. Konqueror */
            virtual void slotOpenLinkInBrowser();

            /** Opens @c m_url in foreground tab */
            virtual void slotOpenLinkInForegroundTab();

            /** Opens @c m_url in background tab */
            virtual void slotOpenLinkInBackgroundTab();

            virtual void slotSaveLinkAs();

            /** This changes cursor to wait cursor */
            void slotStarted(KIO::Job *);

            /** This reverts cursor back to normal one */
            void slotCompleted();

            virtual void slotSelectionChanged();

        protected: // attributes
            KURL m_url;
            
        private:
            /**
             * Display article in external browser.
             */
            void displayInExternalBrowser(const KURL &url, const QString &mime);
    };
}

#endif // VIEWER_H
// vim: ts=4 sw=4 et
