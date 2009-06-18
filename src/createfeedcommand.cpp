/*
    This file is part of Akregator.

    Copyright (C) 2008 Frank Osterfeld <osterfeld@kde.org>

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

#include "createfeedcommand.h"

#include "addfeeddialog.h"

#include <krss/netfeed.h>
#include <krss/netfeedcreatejob.h>
#include <krss/ui/feedlistview.h>

#include <KDebug>
#include <KInputDialog>
#include <KLocalizedString>
#include <KMessageBox>
#include <KUrl>

#include <QPointer>
#include <QTimer>
#include <QClipboard>

#include <cassert>

using namespace Akregator;
using namespace KRss;

class CreateFeedCommand::Private
{
    CreateFeedCommand* const q;
public:
    explicit Private( CreateFeedCommand* qq );

    void doCreate();
    void creationDone( KJob* );
    void modificationDone( KJob* );

    QPointer<FeedListView> m_feedListView;
    QString m_url;
    QString m_resourceIdentifier;
    bool m_autoexec;
};

CreateFeedCommand::Private::Private( CreateFeedCommand* qq )
  : q( qq ),
    m_feedListView(),
    m_autoexec( false )
{

}

void CreateFeedCommand::Private::doCreate()
{
    QString url = m_url;

    if ( !m_autoexec ) {
        QPointer<AddFeedDialog> afd = new AddFeedDialog( q->parentWidget() );

        if( url.isEmpty() )
        {
            const QClipboard* const clipboard = QApplication::clipboard();
            assert( clipboard );
            const QString clipboardText = clipboard->text();
            // Check for the hostname, since the isValid method is not strict enough
            if( !KUrl( clipboardText ).isEmpty() )
                url = clipboardText;
        }

        afd->setUrl( KUrl::fromPercentEncoding( url.toLatin1() ) );

        QPointer<QObject> thisPointer( q );

        if ( afd->exec() != QDialog::Accepted || !thisPointer ) {
            delete afd;
            if ( !thisPointer )
                q->emitResult();
            return;
        }

        url = afd->url();

        delete afd;
    }


    NetFeedCreateJob *job = new NetFeedCreateJob( url, QString(), m_resourceIdentifier, q );
    q->connect( job, SIGNAL(finished(KJob*)), q, SLOT(creationDone(KJob*)) );
    job->start();

    //PENDING(frank): the following should go to a FeedModifyCommand
#ifdef KRSS_PORT_DISABLED

    Feed* const feed = afd->feed();
    delete afd;

    if ( !feed )
    {
        q->emitResult();
        return;
    }

    QPointer<FeedPropertiesDialog> dlg = new FeedPropertiesDialog( q->parentWidget(), "edit_feed" );
    dlg->setFeed( feed );
    dlg->selectFeedName();

    if ( !m_autoexec && ( dlg->exec() != QDialog::Accepted || !thisPointer ) )
    {
        delete feed;
    }
    else
    {
        m_parentFolder = m_parentFolder ? m_parentFolder : m_rootFolder;
        m_parentFolder->insertChild( feed, m_after );
        if ( m_feedListView )
            m_feedListView->ensureNodeVisible( feed );
    }

    delete dlg;
    q->emitResult();
#endif //KRSS_PORT_DISABLED

}

void CreateFeedCommand::Private::creationDone( KJob* j )
{
    if ( j->error() ) {
        q->emitResult();
        return;
    }
    //PENDING(frank) fire off a FeedModifyCommand
    q->emitResult();
}

void CreateFeedCommand::Private::modificationDone( KJob* j )
{
    if ( j->error() )
        KMessageBox::error( q->parentWidget(), i18n("Could not edit the feed: %1", j->errorString() ), i18n("Editing Feed Failed") );
    q->emitResult();
}

CreateFeedCommand::CreateFeedCommand( QObject* parent ) : Command( parent ), d( new Private( this ) )
{

}

CreateFeedCommand::~CreateFeedCommand()
{
    delete d;
}

void CreateFeedCommand::setFeedListView( FeedListView* view )
{
    d->m_feedListView = view;
}

void CreateFeedCommand::setResourceIdentifier( const QString& identifier )
{
    d->m_resourceIdentifier = identifier;
}

void CreateFeedCommand::setUrl( const QString& url )
{
    d->m_url = url;
}

void CreateFeedCommand::setAutoExecute( bool autoexec )
{
    d->m_autoexec = autoexec;
}

void CreateFeedCommand::doStart()
{
    QTimer::singleShot( 0, this, SLOT(doCreate()) );
}

#include "createfeedcommand.moc"
