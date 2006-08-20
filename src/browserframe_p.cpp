/*
    This file is part of Akregator.

    Copyright (C) 2006 Frank Osterfeld <frank.osterfeld@kdemail.net>

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

#include "browserframe_p.h"

namespace Akregator {

void BrowserFrame::BrowserFramePrivate::HistoryAction::slotTriggered(bool)
{
    emit triggered(m_entry);
}

BrowserFrame::BrowserFramePrivate::HistoryAction::HistoryAction(QList<HistoryEntry>::Iterator entry,
         QObject* parent,
         BrowserFramePrivate* priv) : QAction((*entry).title, parent), m_entry(entry)
{
    connect(this, SIGNAL(triggered(bool)), this, SLOT(slotTriggered(bool)));
    connect(this, SIGNAL(triggered(QList<BrowserFrame::BrowserFramePrivate::HistoryEntry>::Iterator)), priv, SLOT(slotHistoryEntrySelected(QList<BrowserFrame::BrowserFramePrivate::HistoryEntry>::Iterator)));
}
    
int BrowserFrame::BrowserFramePrivate::HistoryEntry::idCounter = 0;


bool BrowserFrame::BrowserFramePrivate::loadPartForMimetype(const QString& mimetype)
{
    KService::List offers = KMimeTypeTrader::self()->query( mimetype, "KParts/ReadOnlyPart" );

    kDebug() << "BrowserFrame::loadPartForMimetype("<< mimetype << "): " << offers.size() << " offers" << endl;

    if (!offers.isEmpty())
    {
        // delete old part
        // FIXME: do this only if part can't be reused for the new mimetype
        if (part)
        {
            layout->removeWidget(part->widget());
            delete part;
            part = 0;
            extension = 0;
        }

        KService::Ptr ptr = offers.first();
        KLibFactory* factory = KLibLoader::self()->factory( ptr->library().toLatin1() );
        if (factory)
        {
            part = static_cast<KParts::ReadOnlyPart *>(factory->create(parent, "KParts::ReadOnlyPart"));
            part->setObjectName(ptr->name());
            extension = KParts::BrowserExtension::childObject(part);
            
            layout->addWidget(part->widget());
            connectPart();
        }

        return true;
    }
    else
        return false;
}

QString BrowserFrame::BrowserFramePrivate::debugInfo() const
{
    QString res = "HISTORY: ";
    
    QList<HistoryEntry>::ConstIterator it = history.begin();
    while(it != history.end())
    {
        res += (*it).id + " ";
        ++it;
    }
    
    res += " current=" + (current == history.end() ? -1 : (*current).id);
    return res;
}

void BrowserFrame::BrowserFramePrivate::appendHistoryEntry()
{
    if (lockHistory)
        return;
    
    kDebug() << "BrowserFramePrivate::appendHistoryEntry()" << endl;
    bool canBack = parent->canGoBack();
    bool canForward = parent->canGoForward();

    // cut off history entries after the current one
    if (current != history.end())
        history.erase(current+1, history.end());
    history.append(HistoryEntry());

    current = history.end()-1;

    if (canBack != parent->canGoBack())
        emit parent->signalCanGoBackToggled(parent, parent->canGoBack());
    if (canForward != parent->canGoForward())
        emit parent->signalCanGoForwardToggled(parent, parent->canGoForward());
    
    kDebug() << "BrowserFramePrivate::appendHistoryEntry()" << endl;
}

void BrowserFrame::BrowserFramePrivate::restoreHistoryEntry( QList<HistoryEntry>::Iterator entry)
{
    bool canBack = parent->canGoBack();
    bool canForward = parent->canGoForward();

    updateHistoryEntry();

    lockHistory = true;
    // TODO: set all fields to values from entry
    loadPartForMimetype((*entry).mimetype);

    if (!part)
    {
        lockHistory = false;
        return; // FIXME: do something better
    }

    QDataStream stream(&((*entry).buffer), QIODevice::ReadOnly);

    stream.setVersion(QDataStream::Qt_3_1);

    if (extension)
        extension->restoreState(stream);
    else
        part->openURL((*entry).url);

    current = entry;

    lockHistory = false;

    if (canForward != parent->canGoForward())
        emit parent->signalCanGoForwardToggled(parent, parent->canGoForward());
    if (canBack != parent->canGoBack())
        emit parent->signalCanGoBackToggled(parent, parent->canGoBack());
    kDebug() << "BrowserFramePrivate::restoreHistoryEntry " << debugInfo() << endl;
}


void BrowserFrame::BrowserFramePrivate::updateHistoryEntry()
{
    if (lockHistory || !part || current == history.end())
        return;

    (*current).url = part->url();
    (*current).title = parent->title();
    //(*current).strServiceName = service->desktopEntryName();
    (*current).mimetype = mimetype;

    if (extension)
    {
        (*current).buffer.clear();
        QDataStream stream( &((*current).buffer), QIODevice::WriteOnly );

        extension->saveState( stream );
    }
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
        
        KParts::BrowserExtension* ext = extension;

        if (ext)
        {
            connect( ext, SIGNAL(speedProgress(int)), parent, SLOT(slotSpeedProgress(int)) );
            connect( ext, SIGNAL(speedProgress(int)), parent,         SLOT(slotSetProgress(int)) );
            connect( ext, SIGNAL(openURLRequestDelayed(const KUrl&, const KParts::URLArgs&) ), parent, SLOT(slotOpenURLRequestDelayed(const KUrl&, const KParts::URLArgs&)) );
            connect(ext, SIGNAL(setLocationBarURL(const QString&)), parent, SLOT(slotSetLocationBarURL(const QString&)) );
            connect(ext, SIGNAL(setIconURL(const KUrl&)), parent, SLOT(slotSetIconURL(const KUrl&)) );
            connect(ext, SIGNAL(createNewWindow(const KUrl&, const KParts::URLArgs&)), parent, SLOT(slotCreateNewWindow(const KUrl&, const KParts::URLArgs&)));
            connect(ext, SIGNAL(createNewWindow(const KUrl&, const KParts::URLArgs&, const KParts::WindowArgs&, KParts::ReadOnlyPart*&)), parent, SLOT(slotCreateNewWindow(const KUrl&, const KParts::URLArgs&, const KParts::WindowArgs&, KParts::ReadOnlyPart*&)));
            connect(ext, SIGNAL(popupMenu(KXMLGUIClient *, const QPoint&,
                    const KUrl&, const KParts::URLArgs&, 
                    KParts::BrowserExtension::PopupFlags, mode_t)), 
                    parent, SLOT(slotPopupMenu(KXMLGUIClient *, const QPoint&, 
                    const KUrl&, const KParts::URLArgs&,
                    KParts::BrowserExtension::PopupFlags, mode_t)));
        }
    }
}

} // namespace Akregator

#include "browserframe_p.moc"

