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
#include <kiconloader.h>
#include <kprocess.h>

#ifndef KDE_MAKE_VERSION
#define KDE_MAKE_VERSION( a,b,c ) (((a) << 16) | ((b) << 8) | (c))
#endif

#ifndef KDE_IS_VERSION
#define KDE_IS_VERSION(a,b,c) ( KDE_VERSION >= KDE_MAKE_VERSION(a,b,c) )
#endif

#if KDE_IS_VERSION(3,1,94)
#include <kshell.h>
#endif

#include <qclipboard.h>

#include "viewer.h"
#include "akregatorconfig.h"

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

    // change the cursor when loading stuff...
    connect( this, SIGNAL(started(KIO::Job *)),
             this, SLOT(slotStarted(KIO::Job *)));
    connect( this, SIGNAL(completed()),
             this, SLOT(slotCompleted()));

    connect( browserExtension(), SIGNAL(openURLRequestDelayed(const KURL&, const KParts::URLArgs&)), this, SLOT(slotOpenURLRequest(const KURL&, const KParts::URLArgs& )) );

    connect( browserExtension(), SIGNAL(popupMenu(KXMLGUIClient*, const QPoint&, const KURL&, const KParts::URLArgs&, KParts::BrowserExtension::PopupFlags, mode_t)), this, SLOT(slotPopupMenu(KXMLGUIClient*, const QPoint&, const KURL&, const KParts::URLArgs&, KParts::BrowserExtension::PopupFlags, mode_t)) );

}

/**
 * Display article in external browser.
 */
void Viewer::displayInExternalBrowser(const KURL &url)
{
   if (!url.isValid()) return;
   if (Settings::externalBrowserUseKdeDefault())
       KRun::runURL(url, "text/html", false, false);
   else
   {
       QString cmd = Settings::externalBrowserCustomCommand();
       QString urlStr = url.url();
       cmd.replace(QRegExp("%u"), urlStr);
       KProcess *proc = new KProcess;
#if KDE_IS_VERSION(3,1,94)
       QStringList cmdAndArgs = KShell::splitArgs(cmd);
#else
       QStringList cmdAndArgs = QStringList::split(' ',cmd);
#endif
       *proc << cmdAndArgs;
//       This code will also work, but starts an extra shell process.
//       *proc << cmd;
//       proc->setUseShell(true);
       proc->start(KProcess::DontCare);
       delete proc;
   }        
}

void Viewer::slotOpenURLRequest(const KURL& url, const KParts::URLArgs& args)
{
   kdDebug() << "Viewer: Open url request: " << url << endl;
   if(args.frameName == "_blank" && Settings::mMBBehaviour() == Settings::EnumMMBBehaviour::OpenInExternalBrowser)
      displayInExternalBrowser(url);
   else
      openURL(url);
}

void Viewer::slotPopupMenu(KXMLGUIClient*, const QPoint& p, const KURL& url, const KParts::URLArgs&, KParts::BrowserExtension::PopupFlags, mode_t)
{
   if(this->url() == url) return;
   m_url = url;
   KPopupMenu popup;
   popup.insertItem(SmallIcon("tab_new"), i18n("Open Link"), this, SLOT(slotOpenLinkInternal()));
   popup.insertItem(SmallIcon("window_new"), i18n("Open Link in External Browser"), this, SLOT(slotOpenLinkExternal()));
   popup.insertItem(i18n("Copy Link Location"), this, SLOT(slotCopyToClipboard()));
   popup.exec(p);
}

void Viewer::slotCopyToClipboard()
{
   if(m_url.isEmpty()) return;
   QClipboard *cb = QApplication::clipboard();
   cb->setText(m_url.prettyURL(), QClipboard::Clipboard);
   cb->setText(m_url.prettyURL(), QClipboard::Selection);
}

void Viewer::slotOpenLinkInternal()
{
   if(m_url.isEmpty()) return;
   openURL(m_url);
}

void Viewer::slotOpenLinkExternal()
{
   if (m_url.isEmpty()) return;
   displayInExternalBrowser(m_url);
}

void Viewer::slotStarted(KIO::Job *)
{
   widget()->setCursor( waitCursor );
}

void Viewer::slotCompleted()
{
   widget()->unsetCursor();
}

#include "viewer.moc"
