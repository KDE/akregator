/***************************************************************************
 *   Copyright (C) 2004 by Teemu Rytilahti                                 *
 *   teemu.rytilahti@kde-fi.org                                            *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include <kapplication.h>
#include <khtmlview.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <klocale.h>
#include <khtmlview.h>
#include <kiconloader.h>
#include <kprocess.h>
#include <krun.h>
#include <kshell.h>
#include <kurl.h>

#include <qaccel.h>
#include <qclipboard.h>

#include "viewer.h"
#include "akregator_run.h"
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

bool Viewer::openURL(const KURL &url)
{
    new aKregatorRun(this, (QWidget*)parent(), this, url, KParts::URLArgs()/*args*/, true);
    emit started(0);
    return true;
}


void Viewer::open(const KURL &url)
{
    KHTMLPart::openURL(url);
}


bool Viewer::closeURL()
{
   emit browserExtension()->loadingProgress(-1);
   emit canceled(QString::null);
   return KHTMLPart::closeURL();
}


/**
 * Display article in external browser.
 */
void Viewer::displayInExternalBrowser(const KURL &url, const QString &mimetype)
{
   if (!url.isValid()) return;
   if (Settings::externalBrowserUseKdeDefault())
       KRun::runURL(url, mimetype, false, false);
   else
   {
       QString cmd = Settings::externalBrowserCustomCommand();
       QString urlStr = url.url();
       cmd.replace(QRegExp("%u"), urlStr);
       KProcess *proc = new KProcess;
       QStringList cmdAndArgs = KShell::splitArgs(cmd);
       *proc << cmdAndArgs;
       proc->start(KProcess::DontCare);
       delete proc;
   }
}


bool Viewer::slotOpenURLRequest(const KURL& url, const KParts::URLArgs& args)
{
    kdDebug() << "Viewer: slotOpenURLReq url=="<<url.url()<<endl;
   if(args.frameName == "_blank" && Settings::mMBBehaviour() == Settings::EnumMMBBehaviour::OpenInExternalBrowser) {
       displayInExternalBrowser(url, "text/html"); return true;
   }
   if(args.frameName == "_blank" && Settings::mMBBehaviour() == Settings::EnumMMBBehaviour::OpenInBackground) {
       emit urlClicked(url,true); return true;
   }

   return false;
}

void Viewer::slotPopupMenu(KXMLGUIClient*, const QPoint& p, const KURL& url, const KParts::URLArgs&, KParts::BrowserExtension::PopupFlags, mode_t)
{
   if(this->url() == url) return;
   m_url = url;
   KPopupMenu popup;
   popup.insertItem(SmallIcon("tab_new"), i18n("Open Link"), this, SLOT(slotOpenLinkInternal()));
   popup.insertItem(SmallIcon("window_new"), i18n("Open Link in External Browser"), this, SLOT(slotOpenLinkExternal()));
   popup.insertItem(i18n("Copy Link Address"), this, SLOT(slotCopyToClipboard()));
   popup.exec(p);
}

void Viewer::slotCopyToClipboard()
{
   if(m_url.isEmpty()) return;
   QClipboard *cb = QApplication::clipboard();
   cb->setText(m_url.prettyURL(), QClipboard::Clipboard);
   cb->setText(m_url.prettyURL(), QClipboard::Selection);
}

/*
void Viewer::openLink(const KURL&url, const KParts::URLArgs args)
{
   aKregatorRun *run= new aKregatorRun((QWidget*)parent(), this, url, args);

   connect(run, SIGNAL(canEmbed(const KURL&, const KParts::URLArgs&, const QString &)), this, SLOT(slotOpenPage(const KURL &, const KParts::URLArgs &, const QString &)));
}*/

void Viewer::slotOpenLinkInternal()
{
   if(m_url.isEmpty()) return;
   openURL(m_url);
}

void Viewer::slotOpenLinkExternal()
{
   if (m_url.isEmpty()) return;
   displayInExternalBrowser(m_url, "text/html");
}

void Viewer::slotStarted(KIO::Job *)
{
   widget()->setCursor( waitCursor );
}

void Viewer::slotCompleted()
{
   widget()->unsetCursor();
}
void Viewer::slotScrollUp()
{ 
    view()->scrollBy(0,-10); 
}

void Viewer::slotScrollDown()
{ 
    view()->scrollBy(0,10); 
}

#include "viewer.moc"

// vim: set et ts=4 sts=4 sw=4:
