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

#include "browserframe.h"
#include "pageviewer.h"

#include <QGridLayout>
#include <QList>
#include <QString>
#include <QWidget>

#include <kdebug.h>
#include <klibloader.h>
#include <ktrader.h>
#include <kurl.h>
#include <kparts/browserextension.h>
#include <kparts/part.h>

namespace Akregator {

class BrowserFrame::HistoryEntry
{
    public:

    KURL url;
    QString title;
    QByteArray buffer;
    int id;
    QString mimetype;
    QString strServiceName;
    QByteArray postData;
    QString postContentType;
    bool doPost;
    QString pageReferrer;

    HistoryEntry() {}
    HistoryEntry(const KURL& u, const QString& t=QString::null) : url(u), title(t), id(idCounter++)
    {
    }
    
    bool operator==(const HistoryEntry& other)
    {
        return id == other.id;
    }

    bool operator!=(const HistoryEntry& other)
    {
        return id != other.id;
    }

    static int idCounter;
};

int BrowserFrame::HistoryEntry::idCounter = 0;

class BrowserFrame::BrowserFramePrivate
{
    public:
    BrowserFramePrivate(BrowserFrame* p) : parent(p) {}

    QList<HistoryEntry> history;
    QList<HistoryEntry>::Iterator current;
    KParts::ReadOnlyPart* part;
    QGridLayout* layout;
    bool lockHistory;
    BrowserFrame* parent;
    
    QString mimetype;
    KService::Ptr service;

    void connectPart();
    void updateHistoryEntry();
    void addHistoryEntry();
    void restoreHistoryEntry(HistoryEntry& entry);
    bool loadPartForMimetype(const QString& mimetype);
};

bool BrowserFrame::BrowserFramePrivate::loadPartForMimetype(const QString& mimetype)
{
    KTrader::OfferList offers = KTrader::self()->query( mimetype, "KParts/ReadOnlyPart", QString::null, QString::null);

    kdDebug() << "BrowserFrame::loadPartForMimetype("<< mimetype << "): " << offers.size() << " offers" << endl;

    if (!offers.isEmpty())
    {
        // delete old part
        // FIXME: do this only if part can't be reused for the new mimetype
        if (part)
        {
            layout->removeWidget(part->widget());
            delete part;
            part = 0;
        }

        KService::Ptr ptr = offers.first();
        KLibFactory* factory = KLibLoader::self()->factory( ptr->library().latin1() );
        if (factory)
        {
            part = static_cast<KParts::ReadOnlyPart *>(factory->create(parent, ptr->name().latin1(), "KParts::ReadOnlyPart"));

            layout->addWidget(part->widget());
            connectPart();
        }

        return true;
    }
    else
        return false;
}

void BrowserFrame::BrowserFramePrivate::addHistoryEntry()
{
    if (lockHistory)
        return;

    // cut off history entries after the current one
    history.erase(current, history.end());
    history.append(HistoryEntry());

    current = --(history.end());
}

void BrowserFrame::BrowserFramePrivate::restoreHistoryEntry( HistoryEntry& entry)
{
    updateHistoryEntry();

    lockHistory = true;
    // TODO: set all fields to values from entry
    loadPartForMimetype(entry.mimetype);

    if (!part)
    {
        lockHistory = false;
        return; // FIXME: do something better
    }

    QDataStream stream(&(entry.buffer), QIODevice::ReadOnly);

    stream.setVersion(QDataStream::Qt_3_1);

    if (part->browserExtension())
        part->browserExtension()->restoreState(stream);
    else
        parent->openURL(entry.url, entry.mimetype);

    current = history.find(entry);
    
    lockHistory = false;
}

void BrowserFrame::BrowserFramePrivate::connectPart()
{
    if (part)
    {
        connect(part, SIGNAL(setWindowCaption (const QString &)), parent, SLOT(slotSetCaption (const QString &)));
        connect(part, SIGNAL(setStatusBarText (const QString &)), parent, SLOT(slotSetStatusText (const QString &)));
        connect(part, SIGNAL(started(KIO::Job*)), parent, SLOT(slotSetStarted()));
        connect(part, SIGNAL(completed()), parent, SLOT(slotSetCompleted()));
        connect(part, SIGNAL(canceled(const QString &)), parent, SLOT(slotSetCanceled(const QString&)));
        connect(part, SIGNAL(completed(bool)), parent, SLOT(slotSetCompleted()));
        connect(part, SIGNAL(setWindowCaption(const QString &)), parent, SLOT(slotSetTitle(const QString &)));

        KParts::BrowserExtension *ext = KParts::BrowserExtension::childObject( part );
        if (ext)
        {
            connect( ext, SIGNAL(speedProgress(int)), parent, SLOT(slotSpeedProgress(int)) );
            connect( ext, SIGNAL(speedProgress(int)), parent,         SLOT(slotSetProgress(int)) );
            connect( ext, SIGNAL(openURLRequestDelayed(const KURL&, const KParts::URLArgs&) ), parent, SLOT(slotOpenURLRequestDelayed(const KURL&, const KParts::URLArgs&)) );
            connect(ext, SIGNAL(setLocationBarURL(const QString&)), parent, SLOT(slotSetLocationBarURL(const QString&)) ); 
            connect(ext, SIGNAL(setIconURL(const KURL&)), parent, SLOT(slotSetLocationBarURL(const KURL&)) ); 
        }
    }
    
}

void BrowserFrame::BrowserFramePrivate::updateHistoryEntry()
{
    if (lockHistory || !part || current == history.end())
        return;

    (*current).url = part->url();
    KParts::BrowserExtension* ext = part->browserExtension();
    (*current).title = parent->title();
    //(*current).strServiceName = service->desktopEntryName();
    (*current).mimetype = mimetype;

    if (ext)
    {
        (*current).buffer.clear();
        QDataStream stream( &((*current).buffer), IO_WriteOnly );

        ext->saveState( stream );
    }
}

BrowserFrame::BrowserFrame(QWidget* parent) : Frame(parent)
{
    d = new BrowserFramePrivate(this);
    
    d->part = 0;

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

KURL BrowserFrame::url() const
{
    return d->part ? d->part->url() : KURL();
}

bool BrowserFrame::canGoForward() const
{
    return !d->history.isEmpty() && d->current != --(d->history.end()) && d->current != d->history.end();
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

void BrowserFrame::slotSetIconURL(const KURL& /*url*/)
{
}

void BrowserFrame::slotSpeedProgress(int /*bytesPerSecond*/)
{
}

void BrowserFrame::slotOpenURLRequestDelayed(const KURL& url, const KParts::URLArgs& args)
{
    emit signalOpenURLRequest(this, url, args);
}

void BrowserFrame::slotCreateNewWindow(const KURL& url, const KParts::URLArgs& args)
{
    KParts::URLArgs args2(args);
    args2.setNewTab(true);

    emit signalOpenURLRequest(this, url, args2, NewTab);
}

bool BrowserFrame::openURL(const KURL& url, const QString& mimetype)
{
    if (mimetype.isEmpty())
        return false;

    d->updateHistoryEntry();
    
    if (!d->loadPartForMimetype(mimetype))
    {
        // TODO: show open|save|cancel dialog
    }
    
    bool res = d->part ? d->part->openURL(url) : false;

    if (res)
    {
        d->addHistoryEntry();
        d->updateHistoryEntry();
    }
    return res;
}

KParts::ReadOnlyPart* BrowserFrame::part() const
{
    return d->part;
}

void BrowserFrame::slotHistoryForward()
{
    if (canGoForward())
        d->restoreHistoryEntry(*(++(d->current)));
}

void BrowserFrame::slotHistoryBack()
{
    if (canGoBack())
        d->restoreHistoryEntry(*(--(d->current)));
}

void BrowserFrame::slotReload()
{
    d->lockHistory = true;
    openURL(url(), d->mimetype);
    d->lockHistory = false;
}

void BrowserFrame::slotStop()
{
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
    return true;
}

} // namespace Akregator

#include "browserframe.moc"
