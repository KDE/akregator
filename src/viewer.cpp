/***************************************************************************
 *   Copyright (C) 2004 by Teemu Rytilahti                                 *
 *   teemu.rytilahti@kde-fi.org                                            *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include <kapplication.h>
#include <krun.h>
#include <kpopupmenu.h>
#include <klocale.h>

#include <qclipboard.h>

#include "viewer.h"

using namespace Akregator;

Viewer::Viewer(QWidget *parent, const char *name)
    : KHTMLPart(parent, name), m_url(0)
{
    // to be on a safe side
    setJScriptEnabled(false);
    setJavaEnabled(true);
    setMetaRefreshEnabled(true);
    setPluginsEnabled(true);
    setDNDEnabled(true);
    setAutoloadImages(true);
    setStatusMessagesEnabled(true);

    connect( browserExtension(), SIGNAL(openURLRequestDelayed(const KURL&, const KParts::URLArgs&)), this, SLOT(slotOpenURLRequest(const KURL&, const KParts::URLArgs& )) );

    connect( browserExtension(), SIGNAL(popupMenu(KXMLGUIClient*, const QPoint&, const KURL&, const KParts::URLArgs&, KParts::BrowserExtension::PopupFlags, mode_t)), this, SLOT(slotPopupMenu(KXMLGUIClient*, const QPoint&, const KURL&, const KParts::URLArgs&, KParts::BrowserExtension::PopupFlags, mode_t)) );

}

void Viewer::slotOpenURLRequest(const KURL& url, const KParts::URLArgs&)
{
   kdDebug() << "Viewer: Open url request: " << url << endl;
   openURL(url);
}

void Viewer::slotPopupMenu(KXMLGUIClient*, const QPoint& p, const KURL& url, const KParts::URLArgs&, KParts::BrowserExtension::PopupFlags, mode_t)
{
   if(url.isEmpty()) return;
   m_url = url;
   KPopupMenu popup;
   popup.insertItem(i18n("Open Link"), this, SLOT(slotOpenLinkInternal()));
   popup.insertItem(i18n("Open Link in External Browser"), this, SLOT(slotOpenLinkExternal()));
   popup.insertItem(i18n("Copy Link Location"), this, SLOT(slotCopyToClipboard()));
   popup.exec(p);
}

void Viewer::slotCopyToClipboard()
{
   if(m_url.isEmpty()) return;
   QClipboard *cb = QApplication::clipboard();
   cb->setText(m_url.prettyURL(), QClipboard::Clipboard);
}

void Viewer::slotOpenLinkInternal()
{
   if(m_url.isEmpty()) return;
   emit urlClicked(m_url);
}

void Viewer::slotOpenLinkExternal()
{
   if(m_url.isEmpty()) return;
   KRun::runURL(m_url, "text/html", false, false);
}

#include "viewer.moc"
