/***************************************************************************
 *   Copyright (C) 2004 by Teemu Rytilahti                                 *
 *   teemu.rytilahti@kde-fi.org                                            *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

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
            virtual bool closeURL();
            virtual bool openURL(const KURL &);
            void open(const KURL &);

            //    virtual void openPage(const KURL&url, const KParts::URLArgs& args, const QString &mimetype) = 0;

        public slots:
            void slotScrollUp(); 
            void slotScrollDown();
            void slotZoomIn();
            void slotZoomOut();
            void slotSetZoomFactor(int percent);
            void slotPrint();
            void setSafeMode();

        signals:
            /**
             * This gets emited when url gets clicked
             */
            void urlClicked(const KURL& url, bool background=false);

        protected:
            KURL m_url;

        protected slots:
            virtual bool slotOpenURLRequest(const KURL& url, const KParts::URLArgs& args);
            virtual void slotPopupMenu(KXMLGUIClient*, const QPoint&, const KURL&, const KParts::URLArgs&, KParts::BrowserExtension::PopupFlags, mode_t);

            /** Copies current link to clipboard. */
            void slotCopyLinkAddress();

            /** Copies currently selected text to clipboard */
            virtual void slotCopy();
            
            /** Opens link in internal viewer. */
            virtual void slotOpenLinkInternal();

            /** Opens link in external viewer, eg. Konqueror */
            void slotOpenLinkExternal();

            /** This changes cursor to wait cursor */
            void slotStarted(KIO::Job *);

            /** This reverts cursor back to normal one */
            void slotCompleted();

            virtual void slotSelectionChanged();
            
        private:
            /**
             * Display article in external browser.
             */
            void displayInExternalBrowser(const KURL &url, const QString &mime);
    };
}

#endif // VIEWER_H
// vim: ts=4 sw=4 et
