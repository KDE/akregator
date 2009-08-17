/*
    This file is part of Akregator.

    Copyright (C) 2008-2009 Frank Osterfeld <osterfeld@kde.org>

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

#include "createtagcommand.h"
#include "command_p.h"

#include <krss/tag.h>
#include <krss/tagjobs.h>
#include <krss/tagprovider.h>
#include <krss/ui/feedlistview.h>
#include <krss/ui/tagpropertiesdialog.h>

#include <KDebug>
#include <KInputDialog>
#include <KLocalizedString>
#include <KMessageBox>

#include <QPointer>
#include <QTimer>

#include <cassert>

using namespace Akregator;
using namespace boost;
using namespace KRss;

class CreateTagCommand::Private
{
    CreateTagCommand* const q;
public:
    explicit Private( const shared_ptr<const TagProvider>& tagProvider, CreateTagCommand* qq );

    void doCreate();
    void tagCreateJobFinished( KJob* );

    QPointer<FeedListView> feedListView;
    shared_ptr<const TagProvider> tagProvider;
};

CreateTagCommand::Private::Private( const shared_ptr<const TagProvider>& tp, CreateTagCommand* qq )
  : q( qq ),
    feedListView(),
    tagProvider( tp )
{
    assert( tagProvider );
}

void CreateTagCommand::Private::doCreate()
{
    EmitResultGuard guard( q );
    QPointer<KRss::TagPropertiesDialog> dialog( new KRss::TagPropertiesDialog( q->parentWidget() ) );
    if ( dialog->exec() != KDialog::Accepted || !guard.exists() ) {
        guard.emitResult();
        return;
    }

    KRss::Tag newTag;
    newTag.setLabel( dialog->label() );
    newTag.setDescription( dialog->description() );
    delete dialog;

    KRss::TagCreateJob * const job = tagProvider->tagCreateJob();
    assert( job );
    job->setTag( newTag );
    connect( job, SIGNAL(finished(KJob*)), q, SLOT(tagCreateJobFinished(KJob*)) );
    job->start();
}

void CreateTagCommand::Private::tagCreateJobFinished( KJob* j )
{
    EmitResultGuard guard( q );
    if ( j->error() )
        KMessageBox::error( q->parentWidget(), i18n("Could not create the tag: %1", j->errorString() ), i18n("Tag Creation Error") );

#ifdef KRSS_PORT_DISABLED
    if ( feedListView )
        feedListView->ensureNodeVisible( newFolder );
#else
    kWarning() << "Code temporarily disabled (Akonadi port)";
#endif //KRSS_PORT_DISABLED 
    guard.emitResult();
}

CreateTagCommand::CreateTagCommand( const shared_ptr<const TagProvider>& tagProvider, QObject* parent ) : Command( parent ), d( new Private( tagProvider, this ) )
{

}

CreateTagCommand::~CreateTagCommand()
{
    delete d;
}

void CreateTagCommand::setFeedListView( FeedListView* view )
{
    d->feedListView = view;
}

void CreateTagCommand::doStart()
{
    QTimer::singleShot( 0, this, SLOT( doCreate() ) );
}

#include "createtagcommand.moc"
