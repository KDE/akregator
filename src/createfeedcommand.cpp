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
#include "editfeedcommand.h"
#include "command_p.h"

#include <krss/netfeed.h>
#include <krss/netfeedcreatejob.h>
#include <krss/ui/feedlistview.h>
#include <krss/feedlist.h>
#include <krss/netresource.h>

#include <KDebug>
#include <KInputDialog>
#include <KLocalizedString>
#include <KMessageBox>
#include <KUrl>

#include <QPointer>
#include <QTimer>
#include <QClipboard>

#include <cassert>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

using namespace Akregator;
using namespace KRss;
using boost::weak_ptr;
using boost::shared_ptr;

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
    weak_ptr<NetResource> m_resource;
    weak_ptr<FeedList> m_feedList;
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

        if( url.isEmpty() ) {
            const QClipboard* const clipboard = QApplication::clipboard();
            assert( clipboard );
            const QString clipboardText = clipboard->text();
            // Check for the hostname, since the isValid method is not strict enough
            if( !KUrl( clipboardText ).isEmpty() )
                url = clipboardText;
        }

        afd->setUrl( KUrl::fromPercentEncoding( url.toLatin1() ) );

        EmitResultGuard guard( q );

        if ( afd->exec() != QDialog::Accepted || !guard.exists() ) {
            delete afd;
            guard.emitResult();
            return;
        }

        url = afd->url();

        delete afd;
    }

    const shared_ptr<NetResource> resource = m_resource.lock();
    if ( !resource ) {
        q->emitResult();
        return;
    }

    NetFeedCreateJob *job = resource->netFeedCreateJob( url );
    job->setFeedList( m_feedList );
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

void CreateFeedCommand::Private::creationDone( KJob* job )
{
    EmitResultGuard guard( q );
    if ( job->error() )
        KMessageBox::error( q->parentWidget(), i18n("Could not add feed: %1", job->errorString()),
                            i18n("Feed Creation Failed") );

    if ( m_feedList.expired() ) {
        guard.emitResult();
        return;
    }

    const shared_ptr<const FeedList> sharedFeedList = m_feedList.lock();
    const NetFeedCreateJob* const cjob = qobject_cast<const NetFeedCreateJob*>( job );
    Q_ASSERT( cjob );
    const Feed::Id feedId = cjob->feedId();
    const shared_ptr<Feed> feed = sharedFeedList->feedById( feedId );
    if ( !feed ) {
        guard.emitResult();
        return;
    }

    EditFeedCommand* const ecmd = new EditFeedCommand( q );
    ecmd->setParentWidget( q->parentWidget() );
    ecmd->setFeed( feed );
    ecmd->setFeedList( sharedFeedList );
    connect( ecmd, SIGNAL(finished(KJob*)), q, SLOT(modificationDone(KJob*)) );
    ecmd->start();
}

void CreateFeedCommand::Private::modificationDone( KJob* j )
{
    EmitResultGuard guard( q );
    if ( j->error() )
        KMessageBox::error( q->parentWidget(), i18n("Could not edit the feed: %1", j->errorString() ), i18n("Editing Feed Failed") );
    guard.emitResult();
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

void CreateFeedCommand::setResource( const weak_ptr<NetResource>& resource )
{
    d->m_resource = resource;
}

void CreateFeedCommand::setUrl( const QString& url )
{
    d->m_url = url;
}

void CreateFeedCommand::setFeedList( const weak_ptr<FeedList>& feedList )
{
    d->m_feedList = feedList;
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
