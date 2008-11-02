/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <osterfeld@kde.org>

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
#include "actions.h"
#include "browserframe.h"
#include "browserframe_p.h"
#include "openurlrequest.h"
#include "utils/temporaryvalue.h"

#include <QAction>
#include <QGridLayout>
#include <QList>
#include <QPoint>
#include <QPointer>
#include <QString>
#include <QWidget>

#include <kaction.h>
#include <kdebug.h>
#include <klibloader.h>
#include <KMenu>
#include <kmimetypetrader.h>
#include <ktoolbarpopupaction.h>
#include <kurl.h>
#include <kxmlguiclient.h>
#include <kparts/browserextension.h>
#include <kparts/part.h>

#include <cassert>

using namespace KParts;
using namespace Akregator;

BrowserFrame::BrowserFrame(QWidget* parent) : Frame(parent), d( new Private( this ) )
{
}

BrowserFrame::~BrowserFrame()
{
    delete d;
}

KUrl BrowserFrame::url() const
{
    return d->part ? d->part->url() : KUrl();
}

bool BrowserFrame::canGoForward() const
{
    return !d->history.isEmpty() && d->current != d->history.constEnd()-1 && d->current != d->history.constEnd();
}

bool BrowserFrame::canGoBack() const
{
    return !d->history.isEmpty() && d->current != d->history.constBegin();
}

void BrowserFrame::slotOpenUrlNotify()
{
    // TODO: inform the world that a new url was opened
}

void BrowserFrame::slotSetLocationBarUrl(const QString& /*url*/)
{
    // TODO: use this to update URLs for dragging (like tab drag etc.)
}

void BrowserFrame::slotSetIconUrl(const KUrl& url )
{
    FeedIconManager::self()->removeListener( this );
    FeedIconManager::self()->addListener( url, this );
}

void BrowserFrame::setFavicon( const QIcon& icon )
{
    emit signalIconChanged( this, icon );
}

void BrowserFrame::slotSpeedProgress(int /*bytesPerSecond*/)
{
}

namespace {

static OpenUrlRequest requestFromSender( QObject* sender, int id )
{
    QAction* const action = qobject_cast<QAction*>( sender );
    assert( action );
    const KUrl url = action->data().value<KUrl>();
    OpenUrlRequest req;
    req.setFrameId( id );
    req.setUrl( url );
    return req;
}

}

void BrowserFrame::slotOpenLinkInBrowser()
{
    OpenUrlRequest req = requestFromSender( sender(), id() );
    req.setOptions( OpenUrlRequest::ExternalBrowser );
    emit signalOpenUrlRequest( req );
}

void BrowserFrame::slotOpenLinkInNewTab()
{
    OpenUrlRequest req = requestFromSender( sender(), id() );
    req.setOptions( OpenUrlRequest::NewTab );
    emit signalOpenUrlRequest( req );
}

namespace {

enum SeparatorOption {
    ShowSeparatorIfNotEmpty,
    NoSeparator
};

void addActionsToMenu( QMenu* menu, const QList<QAction*> actions, SeparatorOption option )
{
    if ( !actions.isEmpty() && option != NoSeparator )
        menu->addSeparator();
    Q_FOREACH( QAction* const i, actions )
            menu->addAction( i );
}

}

void BrowserFrame::slotPopupMenu(
                   const QPoint& global,
                   const KUrl& url,
                   mode_t mode,
                   const OpenUrlArguments& args,
                   const BrowserArguments& browserArgs,
                   BrowserExtension::PopupFlags flags,
                   const KParts::BrowserExtension::ActionGroupMap& actionGroups )
{
    const bool showReload = (flags & BrowserExtension::ShowReload) != 0;
    const bool showNavigationItems = (flags & BrowserExtension::ShowNavigationItems) != 0;
    const bool isLink = (flags & BrowserExtension:: IsLink) != 0;
    const bool isSelection = (flags & BrowserExtension::ShowTextSelectionItems) != 0;

    bool isFirst = true;

    QPointer<KMenu> popup( new KMenu( d->part->widget() ) );

    if (showNavigationItems)
    {
        popup->addAction( ActionManager::getInstance()->action( "browser_back" ) );
        popup->addAction( ActionManager::getInstance()->action( "browser_forward" ) );
        isFirst = false;
    }
    if (showReload)
    {
        popup->addAction( ActionManager::getInstance()->action( "browser_reload" ) );
        isFirst = false;
    }

#define addSeparatorIfNotFirst() if ( !isFirst ) popup->addSeparator(); isFirst = false;

    if (isLink)
    {
        addSeparatorIfNotFirst();
        popup->addAction( createOpenLinkInNewTabAction( url, this, SLOT( slotOpenLinkInNewTab() ), popup ) );
        popup->addAction( createOpenLinkInExternalBrowserAction( url, this, SLOT( slotOpenLinkInBrowser() ), popup ) );
        addActionsToMenu( popup, actionGroups.value( "linkactions" ), ShowSeparatorIfNotEmpty );
    }

    if (isSelection)
    {
        addSeparatorIfNotFirst();
        addActionsToMenu( popup, actionGroups.value( "editactions" ), NoSeparator );
    }

    addSeparatorIfNotFirst();
    addActionsToMenu( popup, actionGroups.value( "part" ), NoSeparator );

    popup->exec( global );
    delete popup;
}

void BrowserFrame::slotOpenUrlRequestDelayed(const KUrl& url, const OpenUrlArguments& args, const BrowserArguments& browserArgs)
{
    OpenUrlRequest req;

    req.setFrameId(id());
    req.setUrl(url);
    req.setArgs(args);
    req.setBrowserArgs(browserArgs);

    emit signalOpenUrlRequest(req);
}

void BrowserFrame::slotCreateNewWindow(const KUrl& url,
                                       const OpenUrlArguments& args,
                                       const BrowserArguments& browserArgs,
                                       const WindowArgs& /*windowArgs*/,
                                       ReadOnlyPart** part)
{
    OpenUrlRequest req;
    req.setFrameId(id());
    req.setUrl(url);
    req.setArgs(args);
    req.setBrowserArgs(browserArgs);
    req.setOptions(OpenUrlRequest::NewTab);

    emit signalOpenUrlRequest(req);
    if ( part )
        *part = req.part();
}

bool BrowserFrame::openUrl(const OpenUrlRequest& request)
{
    const QString serviceType = request.args().mimeType();

    if ( serviceType.isEmpty() )
        return false;

    d->updateHistoryEntry();

    kDebug() << "serviceType: " << serviceType;
    if ( !d->loadPartForMimetype( serviceType ) )
        return false;

    assert( d->part );
    d->part->setArguments( request.args() );

    if ( !request.url().isValid() )
        return false;

    if ( !d->part->openUrl( request.url() ) )
        return false;

    d->appendHistoryEntry( request.url() );
    d->updateHistoryEntry();
    return true;
}

ReadOnlyPart* BrowserFrame::part() const
{
    return d->part;
}

void BrowserFrame::slotHistoryBackAboutToShow()
{
    QAction* ba = ActionManager::getInstance()->action("browser_back");
    QMenu* popup = static_cast<KToolBarPopupAction*>(ba)->menu();
    popup->clear();

    if (!canGoBack())
        return;


    QList<Private::HistoryEntry>::Iterator it = d->current-1;

    int i = 0;
    while( i < 10)
    {
        if ( it == d->history.constBegin() )
        {
            popup->addAction(new Private::HistoryAction(it, popup, d));
            return;
        }

        popup->addAction(new Private::HistoryAction(it, popup, d));
        ++i;
        --it;
    }
}

void BrowserFrame::slotHistoryForwardAboutToShow()
{
    QAction* fw = ActionManager::getInstance()->action("browser_forward");
    QMenu* popup = qobject_cast<KToolBarPopupAction*>(fw)->menu();
    popup->clear();

    if (!canGoForward())
        return;

    QList<Private::HistoryEntry>::Iterator it = d->current+1;

    int i = 0;
    while( i < 10)
    {
        if ( it == d->history.constEnd()-1 )
        {
            popup->addAction( new Private::HistoryAction(it, popup, d));
            return;
        }

        popup->addAction(new Private::HistoryAction(it, popup, d));
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
    TemporaryValue<bool> lock( d->lockHistory, true );

    OpenUrlRequest req(url());
    KParts::OpenUrlArguments args;
    args.setMimeType(d->mimetype);
    req.setArgs(args);
    openUrl(req);
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
    return true;
}

bool BrowserFrame::isLoading() const
{
    return d->isLoading;
}


void BrowserFrame::loadConfig( const KConfigGroup& config, const QString& prefix)
{
    QString url = config.readEntry( QString::fromLatin1( "url" ).prepend( prefix ), QString() );
    QString mimetype = config.readEntry( QString::fromLatin1( "mimetype" ).prepend( prefix ), QString() );
    OpenUrlRequest req(url);
    KParts::OpenUrlArguments args;
    args.setMimeType(mimetype);
    req.setArgs(args);
    openUrl(req);
}

void BrowserFrame::saveConfig( KConfigGroup& config, const QString& prefix)
{
    config.writeEntry( QString::fromLatin1( "url" ).prepend( prefix ), url().url() );
    config.writeEntry( QString::fromLatin1( "mimetype" ).prepend( prefix ), d->mimetype );
}

#include "browserframe.moc"
