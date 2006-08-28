/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>

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

#include "actionmanager.h"
#include "browserframe.h"
#include "browserframe_p.h"
#include "openurlrequest.h"

#include <QAction>
#include <QGridLayout>
#include <QList>
#include <QPoint>
#include <QString>
#include <QWidget>

#include <kaction.h>
#include <kdebug.h>
#include <klibloader.h>
#include <kmenu.h>
#include <kmimetypetrader.h>
#include <ktoolbarpopupaction.h>
#include <kurl.h>
#include <kxmlguiclient.h>
#include <kparts/browserextension.h>
#include <kparts/part.h>


namespace Akregator {

BrowserFrame::BrowserFrame(QWidget* parent) : Frame(parent)
{
    d = new BrowserFramePrivate(this);

    d->part = 0;
    d->extension = 0;
    d->lockHistory = false;
    d->isLoading = false;
    setRemovable(true);
    d->current = d->history.end();

    d->layout = new QGridLayout(this);
    setLayout(d->layout);
}

BrowserFrame::~BrowserFrame()
{
    if (d->part)
        d->part->deleteLater();
    delete d;
    d = 0;
}

KUrl BrowserFrame::url() const
{
    return d->part ? d->part->url() : KUrl();
}

bool BrowserFrame::canGoForward() const
{
    return !d->history.isEmpty() && d->current != d->history.end()-1 && d->current != d->history.end();
}

bool BrowserFrame::canGoBack() const
{
    return !d->history.isEmpty() && d->current != d->history.begin();
}

void BrowserFrame::slotOpenURLNotify()
{
    // TODO: inform the world that a new url was opened
}

void BrowserFrame::slotSetLocationBarURL(const QString& /*url*/)
{
    // TODO: use this to update URLs for dragging (like tab drag etc.)
}

void BrowserFrame::slotSetIconURL(const KUrl& /*url*/)
{
}

void BrowserFrame::slotSpeedProgress(int /*bytesPerSecond*/)
{
}

void BrowserFrame::slotPopupMenu(KXMLGUIClient* client, 
                   const QPoint& global, 
                   const KUrl& url,
                   const KParts::URLArgs& args,
                   KParts::BrowserExtension::PopupFlags flags,
                   mode_t mode)
{
    const bool showReload = (flags & KParts::BrowserExtension::ShowReload) != 0;
    const bool showNavigationItems = (flags & KParts::BrowserExtension::ShowNavigationItems) != 0;
    const bool isLink = (flags & KParts::BrowserExtension:: IsLink) != 0;
    const bool isSelection = (flags & KParts::BrowserExtension::ShowTextSelectionItems) != 0;
        
    bool isFirst = true;
    
    KMenu popup(d->part->widget());
    
    /*    
    if (showNavigationItems)
    {
        popup.addAction(ActionManager::getInstance()->action("browser_back"));
        popup.addAction(ActionManager::getInstance()->action("browser_forward"));
        isFirst = false;
    }
    if (showReload)
    {
        popup.addAction(ActionManager::getInstance()->action("browser_reload"));
        isFirst = false;
    }
    
    if (isLink)
    {
        if (!isFirst)
            popup.addSeparator();
        // open link in new tab
        // ext browser
        popup.addSeparator();
        popup.addAction(d->part->action("savelinkas"));
        // save link as
        // copy link address
        isFirst = false;
    }
    
    if (isSelection)
    {
        if (!isFirst)
            popup.addSeparator();
    
        popup.addAction(d->part->action("copy"));
        isFirst = false;
    }
    */
    popup.exec(global);
}
                   
void BrowserFrame::slotOpenURLRequestDelayed(const KUrl& url, const KParts::URLArgs& args)
{
    OpenURLRequest req;
    
    req.setFrameId(id());
    req.setUrl(url);
    req.setArgs(args);
    
    emit signalOpenURLRequest(req);
}

void BrowserFrame::slotCreateNewWindow(const KUrl& url, const KParts::URLArgs& args)
{
    OpenURLRequest req;
    req.setFrameId(id());
    req.setUrl(url);
    req.setArgs(args);
    req.setOptions(OpenURLRequest::NewTab);
    
    emit signalOpenURLRequest(req);
}

void BrowserFrame::slotCreateNewWindow(const KUrl& url, 
                                       const KParts::URLArgs& args,
                                       const KParts::WindowArgs& /*windowArgs*/, 
                                       KParts::ReadOnlyPart*& part)
{
    OpenURLRequest req;
    req.setFrameId(id());
    req.setUrl(url);
    req.setArgs(args);
    req.setOptions(OpenURLRequest::NewTab);
    
    emit signalOpenURLRequest(req);
    part = req.part();
}

bool BrowserFrame::openURL(const OpenURLRequest& request)
{
    QString serviceType = request.args().serviceType;
    
    if (serviceType.isEmpty())
        return false;

    d->updateHistoryEntry();

    if (d->loadPartForMimetype(serviceType))
    {
        if (d->extension)
        {
            d->extension->setUrlArgs(request.args());
        }
        bool res = false;
        
        if (request.url().isValid())
            res = d->part->openUrl(request.url());
        
        if (res)
        {
            d->appendHistoryEntry(request.url());
            d->updateHistoryEntry();
        }

        return res;
    }
    else
    {
            // TODO: show open|save|cancel dialog
    
    
    }
    
    return false; // TODO: is this correct?
}

KParts::ReadOnlyPart* BrowserFrame::part() const
{
    return d->part;
}

void BrowserFrame::slotHistoryBackAboutToShow()
{
    KAction* ba = ActionManager::getInstance()->action("browser_back");
    QMenu* popup = static_cast<KToolBarPopupAction*>(ba)->menu();
    popup->clear();

    if (!canGoBack())
        return;
    
    
    QList<BrowserFramePrivate::HistoryEntry>::Iterator it = d->current-1;
    
    int i = 0;
    while( i < 10)
    {
        if ( it == d->history.begin() )
        {
            popup->addAction(new BrowserFramePrivate::HistoryAction(it, popup, d));
            return;
        }
        
        popup->addAction(new BrowserFramePrivate::HistoryAction(it, popup, d));
        ++i;
        --it;
    }
}

void BrowserFrame::slotHistoryForwardAboutToShow()
{
    KAction* fw = ActionManager::getInstance()->action("browser_forward");
    QMenu* popup = static_cast<KToolBarPopupAction*>(fw)->menu(); 
    popup->clear();
    
    if (!canGoForward())
        return;

    QList<BrowserFramePrivate::HistoryEntry>::Iterator it = d->current+1;
        
    int i = 0;
    while( i < 10)
    {
        if ( it == d->history.end()-1 )
        {
            popup->addAction( new BrowserFramePrivate::HistoryAction(it, popup, d));
            return;
        }
        
        popup->addAction(new BrowserFramePrivate::HistoryAction(it, popup, d));
        ++i;
        ++it;
    }
}

void BrowserFrame::slotHistoryForward()
{
    if (canGoForward())
        d->restoreHistoryEntry(d->current+1);
}

void BrowserFrame::slotHistoryBack()
{
    if (canGoBack())
        d->restoreHistoryEntry(d->current-1);
}

void BrowserFrame::slotReload()
{
    // TODO
    //d->lockHistory = true;
    //openURL(d->url, d->mimetype); // this s
    //d->lockHistory = false;
}

void BrowserFrame::slotStop()
{
    if (d->part)
        d->part->closeUrl();
    Frame::slotStop();
    
}

void BrowserFrame::slotPaletteOrFontChanged()
{
}

bool BrowserFrame::isReloadable() const
{
    return false; // TODO
}

bool BrowserFrame::isLoading() const
{
    return d->isLoading;
}

} // namespace Akregator

#include "browserframe.moc"
