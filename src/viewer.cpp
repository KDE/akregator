/***************************************************************************
 *   Copyright (C) 2004 by Teemu Rytilahti                                 *
 *   teemu.rytilahti@kde-fi.org                                            *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include <kaction.h>
#include <kapplication.h>
#include <kfiledialog.h>
#include <khtmlview.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
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
    setJScriptEnabled(true);
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

    connect( browserExtension(),

SIGNAL(popupMenu (KXMLGUIClient*, const QPoint&, const KURL&, const
    KParts::URLArgs&, KParts::BrowserExtension::PopupFlags, mode_t)), this, SLOT(slotPopupMenu(KXMLGUIClient*, const QPoint&, const KURL&, const
    KParts::URLArgs&, KParts::BrowserExtension::PopupFlags, mode_t)));

    KStdAction::print(this, SLOT(slotPrint()), actionCollection(), "viewer_print");
    KStdAction::copy(this, SLOT(slotCopy()), actionCollection(), "viewer_copy");
    
    new KAction( i18n("&Increase Font Sizes"), "viewmag+", "Ctrl+Plus", this, SLOT(slotZoomIn()), actionCollection(), "incFontSizes" );
    new KAction( i18n("&Decrease Font Sizes"), "viewmag-", "Ctrl+Minus", this, SLOT(slotZoomOut()), actionCollection(), "decFontSizes" );

    connect(this, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()));

    new KAction(i18n("Copy Link Address"), "", 0,
                                 this, SLOT(slotCopyLinkAddress()),
                                 actionCollection(), "copylinkaddress");
}

bool Viewer::openURL(const KURL &url)
{
    new Akregator::BrowserRun(this, (QWidget*)parent(), this, url, KParts::URLArgs());
    emit started(0);
    return true;
}


bool Viewer::closeURL()
{
    emit browserExtension()->loadingProgress(-1);
    emit canceled(QString::null);
    return KHTMLPart::closeURL();
}


void Viewer::displayInExternalBrowser(const KURL &url, const QString &mimetype)
{
   if (!url.isValid()) return;
   if (Settings::externalBrowserUseKdeDefault())
   {
       if (mimetype.isEmpty()) 
           kapp->invokeBrowser(url.url(), "0");
       else
           KRun::runURL(url, mimetype, false, false);
   }
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


void Viewer::slotOpenURLRequest(const KURL& url, const KParts::URLArgs& args)
{
    m_url = url;
    if (args.frameName == "_blank") // apparently this indicates that the MMB was pressed...
    {
        switch (Settings::mMBBehaviour())
        {
            case Settings::EnumMMBBehaviour::OpenInExternalBrowser:
                slotOpenLinkInBrowser();
                break;
            case Settings::EnumMMBBehaviour::OpenInBackground:
                slotOpenLinkInBackgroundTab();
                break;
            default:
                slotOpenLinkInForegroundTab();
                break;
        }
    }
    else // LMB:
    {
        switch (Settings::lMBBehaviour())
        {
            case Settings::EnumLMBBehaviour::OpenInExternalBrowser:
                slotOpenLinkInBrowser();
                break;
            case Settings::EnumLMBBehaviour::OpenInBackground:
                slotOpenLinkInBackgroundTab();
                break;
            default:
                slotOpenLinkInForegroundTab();
                break;
        }
    }
}

void Viewer::slotPopupMenu(KXMLGUIClient*, const QPoint& p, const KURL& kurl, const KParts::URLArgs&, KParts::BrowserExtension::PopupFlags, mode_t)
{
   QString url = kurl.url();
   if(this->url() == url) return;
   m_url = url;
   KPopupMenu popup;
   
   if (!url.isEmpty())
   {
        popup.insertItem(SmallIcon("tab_new"), i18n("Open Link in New Tab"), this, SLOT(slotOpenLinkInForegroundTab()));
        popup.insertItem(SmallIcon("window_new"), i18n("Open Link in External Browser"), this, SLOT(slotOpenLinkInBrowser()));
        action("copylinkaddress")->plug(&popup);
   }
   else
   {
       action("viewer_copy")->plug(&popup);
       popup.insertSeparator();
       action("viewer_print")->plug(&popup);
       KAction *ac = action("setEncoding");
       if (ac)
            ac->plug(&popup);
   }
   popup.exec(p);
}

// taken from KDevelop
void Viewer::slotCopy()
{
    QString text = selectedText();
    text.replace( QChar( 0xa0 ), ' ' );
    QClipboard *cb = QApplication::clipboard();
    disconnect( cb, SIGNAL( selectionChanged() ), this, SLOT( slotClearSelection() ) );
    cb->setText(text);
    connect( cb, SIGNAL( selectionChanged() ), this, SLOT( slotClearSelection() ) );
}

void Viewer::slotCopyLinkAddress()
{
   if(m_url.isEmpty()) return;
   QClipboard *cb = QApplication::clipboard();
   cb->setText(m_url.prettyURL(), QClipboard::Clipboard);
   cb->setText(m_url.prettyURL(), QClipboard::Selection);
}

void Viewer::slotSelectionChanged()
{
    action("viewer_copy")->setEnabled(!selectedText().isEmpty());
}

void Viewer::slotOpenLinkInternal()
{
   openURL(m_url);
}

void Viewer::slotOpenLinkInForegroundTab()
{
    emit urlClicked(m_url, false);
}

void Viewer::slotOpenLinkInBackgroundTab()
{
    emit urlClicked(m_url, true);
}

void Viewer::slotOpenLinkInBrowser()
{
    displayInExternalBrowser(m_url, QString::null);
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

void Viewer::slotZoomIn()
{
    int zf = zoomFactor();
    if (zf < 100)
    {
        zf = zf - (zf % 20) + 20;
        setZoomFactor(zf);
    }
    else
    {
        zf = zf - (zf % 50) + 50;
        setZoomFactor(zf < 300 ? zf : 300);
    }
}

void Viewer::slotZoomOut()
{
    int zf = zoomFactor();
    if (zf <= 100)
    {
        zf = zf - (zf % 20) - 20;
        setZoomFactor(zf > 20 ? zf : 20);
    }
    else
    {
        zf = zf - (zf % 50) - 50;
        setZoomFactor(zf);
    }
}

void Viewer::slotSetZoomFactor(int percent)
{
    setZoomFactor(percent);
}

// some code taken from KDevelop (lib/widgets/kdevhtmlpart.cpp)
void Viewer::slotPrint( )
{
    view()->print();
}


void Viewer::setSafeMode()
{
    setJScriptEnabled(false);
    setJavaEnabled(false);
    setMetaRefreshEnabled(false);
    setPluginsEnabled(false);
    setDNDEnabled(true);
    setAutoloadImages(true);
    setStatusMessagesEnabled(false);
}

#include "viewer.moc"

// vim: set et ts=4 sts=4 sw=4:
