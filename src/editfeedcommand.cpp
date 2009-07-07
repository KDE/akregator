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

#include "editfeedcommand.h"
#include "command_p.h"

#include <krss/feedjobs.h>
#include <krss/feedlist.h>
#include <krss/feedvisitor.h>
#include <krss/netfeed.h>
#include <krss/ui/feedpropertiesdialog.h>

#include <KLocalizedString>
#include <KMessageBox>

#include <QPointer>
#include <QTimer>

#include <cassert>
#include <boost/shared_ptr.hpp>

using namespace Akregator;
using namespace boost;
using namespace KRss;

class EditFeedCommand::Private : public FeedVisitor
{
    EditFeedCommand* const q;
public:
    explicit Private( EditFeedCommand* qq );

    void visitNetFeed( const shared_ptr<NetFeed>& nf ) {
        feedHandled = true;
        EmitResultGuard guard( q );
        QPointer<FeedPropertiesDialog> dlg( new FeedPropertiesDialog( q->parentWidget() ) );
        dlg->setFeedTitle( nf->title() );
        dlg->setUrl( nf->xmlUrl() );
        dlg->setCustomFetchInterval( nf->fetchInterval() > 0 ); //PENDING(frank) correct?
        dlg->setFetchInterval( nf->fetchInterval() );

        if ( dlg->exec() != QDialog::Accepted ) {
            delete dlg;
            guard.emitResult();
            return;
        }
        nf->setTitle( dlg->feedTitle() );
        nf->setXmlUrl( dlg->url() );
        nf->setFetchInterval( dlg->hasCustomFetchInterval() ? dlg->fetchInterval() : 0 );
        delete dlg;
        FeedModifyJob* job = new FeedModifyJob( nf );
        connect( job, SIGNAL(finished(KJob*)), q, SLOT(feedModifyDone(KJob*)) );
        job->start();
    }

    void feedModifyDone( KJob* job ) {
        EmitResultGuard guard( q );
        if ( job->error() )
            KMessageBox::error( q->parentWidget(),
                                i18n("Could not save feed settings: %1", job->errorString() ) );
        guard.emitResult();
    }

    void startEdit();
    void jobFinished();

    bool feedHandled;
    shared_ptr<Feed> feed;
    shared_ptr<const FeedList> feedList;
};

EditFeedCommand::Private::Private( EditFeedCommand* qq )
    : q( qq ), feedHandled( false )
{
}

EditFeedCommand::EditFeedCommand( QObject* parent )
    : Command( parent ), d( new Private( this ) )
{
}

EditFeedCommand::~EditFeedCommand()
{
    delete d;
}

void EditFeedCommand::setFeed( const shared_ptr<Feed>& feed )
{
    d->feed = feed;
}

shared_ptr<Feed> EditFeedCommand::feed() const
{
    return d->feed;
}

void EditFeedCommand::doStart()
{
    QTimer::singleShot( 0, this, SLOT(startEdit()) );
}

void EditFeedCommand::Private::startEdit()
{
    EmitResultGuard guard( q );
    if ( !feed ) {
        guard.emitResult();
        return;
    }
    feed->accept( this );
    if ( !feedHandled )
        guard.emitResult();
}

shared_ptr<const FeedList> EditFeedCommand::feedList() const
{
    return d->feedList;
}

void EditFeedCommand::setFeedList( const shared_ptr<const FeedList>& fl )
{
    d->feedList = fl;
}

#include "editfeedcommand.moc"
