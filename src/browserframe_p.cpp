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
#include "utils/temporaryvalue.h"

using namespace Akregator;

BrowserFrame::Private::Private( BrowserFrame* qq )
  : QObject( qq ),
    q( qq ),
    history(),
    current( history.end() ),
    part( 0 ),
    extension( 0 ),
    layout( new QGridLayout( q ) ),
    lockHistory( false ),
    isLoading( false )
{
    layout->setMargin( 0 );
    q->setRemovable( true );
}

BrowserFrame::Private::~Private()
{
    FeedIconManager::self()->removeListener( q );
    if ( part )
        part->disconnect( this );
    delete part;
}

void BrowserFrame::Private::HistoryAction::slotTriggered(bool)
{
    emit triggered(m_entry);
}

BrowserFrame::Private::HistoryAction::HistoryAction(QList<HistoryEntry>::Iterator entry,
         QObject* q,
         Private* priv) : QAction((*entry).title, q), m_entry(entry)
{
    connect(this, SIGNAL(triggered(bool)), this, SLOT(slotTriggered(bool)));
    connect(this, SIGNAL(triggered(QList<BrowserFrame::Private::HistoryEntry>::Iterator)), priv, SLOT(slotHistoryEntrySelected(QList<BrowserFrame::Private::HistoryEntry>::Iterator)));
}

int BrowserFrame::Private::HistoryEntry::idCounter = 0;


bool BrowserFrame::Private::loadPartForMimetype(const QString& mimetype)
{
    KService::List offers = KMimeTypeTrader::self()->query( mimetype, "KParts/ReadOnlyPart" );

    kDebug() <<"BrowserFrame::loadPartForMimetype("<< mimetype <<"):" << offers.size() <<" offers";

    if (offers.isEmpty())
        return false;
    // delete old part
    // FIXME: do this only if part can't be reused for the new mimetype
    if (part)
    {
        part->disconnect( this );
        layout->removeWidget(part->widget());
        delete part;
        delete extension;
    }

    KService::Ptr ptr = offers.first();
    KPluginFactory* factory = KPluginLoader(*ptr).factory();
    if (!factory)
        return false;
    part = factory->create<KParts::ReadOnlyPart>(q);
    if (!part)
        return false;

    // Parts can be destroyed from within the part itself, thus we must listen to destroyed()
    // partDestroyed() requests deletion of this BrowserFrame, which deletes the part, which crashes as
    // the part is already half-destructed (destroyed() is emitted from the dtor) but the QPointer is not yet reset to 0.
    // Thus queue the signal to avoid he double deletion.
    connect(part, SIGNAL(destroyed(QObject*)), this, SLOT(partDestroyed(QObject*)), Qt::QueuedConnection);

    part->setObjectName(ptr->name());
    extension = KParts::BrowserExtension::childObject(part);

    layout->addWidget(part->widget());
    connectPart();
    this->mimetype = mimetype;
    return true;
}

void BrowserFrame::Private::partDestroyed( QObject* )
{
    emit q->signalPartDestroyed( q->id() );
}

void BrowserFrame::Private::appendHistoryEntry(const KUrl& url)
{
    if (lockHistory)
        return;

    bool canBack = q->canGoBack();
    bool canForward = q->canGoForward();


    if (current != history.end())
    {
        // if the new URL is equal to the previous one,
        // we do not create a new entry and exit here
        if ((*current).url == url)
            return;

        // cut off history entries after the current one
        history.erase(current+1, history.end());
    }
    history.append(HistoryEntry());

    current = history.end()-1;

    if (canBack != q->canGoBack())
        emit q->signalCanGoBackToggled(q, !canBack);
    if (canForward != q->canGoForward())
        emit q->signalCanGoForwardToggled(q, !canForward);
}

void BrowserFrame::Private::restoreHistoryEntry( const QList<HistoryEntry>::Iterator& entry)
{
    bool canBack = q->canGoBack();
    bool canForward = q->canGoForward();

    updateHistoryEntry();

    // TODO: set all fields to values from entry
    loadPartForMimetype((*entry).mimetype);

    if (!part)
        return; // FIXME: do something better

    {
        TemporaryValue<bool> lock( lockHistory, true );
        QDataStream stream(&((*entry).buffer), QIODevice::ReadOnly);
        if (extension)
            extension->restoreState(stream);
        else
        {
            kDebug() <<"BrowserFrame::restoreHistoryEntry(): no BrowserExtension found, reloading page!";
            part->openUrl((*entry).url);
        }
        mimetype = entry->mimetype;
        current = entry;
    }

    if (canForward != q->canGoForward())
        emit q->signalCanGoForwardToggled(q, !canForward);
    if (canBack != q->canGoBack())
        emit q->signalCanGoBackToggled(q, !canBack);
}


void BrowserFrame::Private::updateHistoryEntry()
{
    if (lockHistory || !part || current == history.end() || !part->url().isValid())
        return;

    kDebug() <<"BrowserFrame::updateHistoryEntry(): updating id=" << (*current).id <<" url=" << part->url().url();

    (*current).url = part->url();
    (*current).title = q->title();
    //(*current).strServiceName = service->desktopEntryName();
    (*current).mimetype = mimetype;

    if (extension)
    {
        (*current).buffer.clear();
        QDataStream stream( &((*current).buffer), QIODevice::WriteOnly );

        extension->saveState( stream );
    }
}

void BrowserFrame::Private::connectPart()
{
    if (part)
    {
        connect( part, SIGNAL(setWindowCaption(QString)),
                 q, SLOT(slotSetCaption(QString)) );
        connect( part, SIGNAL(setStatusBarText(QString)),
                 q, SLOT(slotSetStatusText(QString)) );
        connect( part, SIGNAL(started(KIO::Job*)), q, SLOT(slotSetStarted()));
        connect( part, SIGNAL(completed()), q, SLOT(slotSetCompleted()));
        connect( part, SIGNAL(canceled(QString)),
                 q, SLOT(slotSetCanceled(QString)) );
        connect( part, SIGNAL(completed(bool)),
                 q, SLOT(slotSetCompleted()) );
        connect( part, SIGNAL(setWindowCaption(QString)),
                 q, SLOT(slotSetTitle(QString)) );

        KParts::BrowserExtension* ext = extension;

        if (ext)
        {
            connect( ext, SIGNAL(speedProgress(int)),
                     q, SLOT(slotSpeedProgress(int)) );
            connect( ext, SIGNAL(speedProgress(int)),
                     q, SLOT(slotSetProgress(int)) );
            connect( ext, SIGNAL(openUrlRequestDelayed(KUrl,
                     KParts::OpenUrlArguments,
                     KParts::BrowserArguments ) ),
                     q, SLOT(slotOpenUrlRequestDelayed(KUrl,
                                  KParts::OpenUrlArguments,
                                  KParts::BrowserArguments)) );
            connect(ext, SIGNAL(setLocationBarUrl(QString)),
                    q, SLOT(slotSetLocationBarUrl(QString)) );
            connect(ext, SIGNAL(setIconUrl(KUrl)),
                    q, SLOT(slotSetIconUrl(KUrl)) );
            connect(ext, SIGNAL( createNewWindow( KUrl,
                    KParts::OpenUrlArguments,
                    KParts::BrowserArguments,
                    KParts::WindowArgs,
                    KParts::ReadOnlyPart**)),
                    q, SLOT(slotCreateNewWindow( KUrl,
                                 KParts::OpenUrlArguments,
                                 KParts::BrowserArguments,
                                 KParts::WindowArgs,
                                 KParts::ReadOnlyPart**)));
            connect(ext, SIGNAL(popupMenu(QPoint,KUrl,mode_t,
                    KParts::OpenUrlArguments, KParts::BrowserArguments,
                    KParts::BrowserExtension::PopupFlags, KParts::BrowserExtension::ActionGroupMap)),
                    q, SLOT(slotPopupMenu(QPoint,KUrl,mode_t,
                    KParts::OpenUrlArguments, KParts::BrowserArguments,
                    KParts::BrowserExtension::PopupFlags,
                    KParts::BrowserExtension::ActionGroupMap )));
        }
    }
}

#include "browserframe_p.moc"

