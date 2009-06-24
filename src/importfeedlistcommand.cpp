/*
    This file is part of Akregator.

    Copyright (C) 2009 Frank Osterfeld <osterfeld@kde.org>

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

#include "importfeedlistcommand.h"
#include "command_p.h"

#include <krss/importopmljob.h>
#include <krss/resource.h>
#include <krss/resourcemanager.h>

#include <KFileDialog>
#include <KLocalizedString>
#include <KMessageBox>
#include <KUrl>

#include <QTimer>

#include <boost/shared_ptr.hpp>

#include <cassert>

using namespace Akregator;
using namespace KRss;

class ImportFeedListCommand::Private
{
    ImportFeedListCommand* const q;
public:
    explicit Private( ImportFeedListCommand* qq );

    void doImport();
    void importFinished( KJob* );
    bool checkResource( const Resource* r );

    KUrl url;
    QString resourceIdentifier;
};

ImportFeedListCommand::Private::Private( ImportFeedListCommand* qq )
    : q( qq )
{

}

bool ImportFeedListCommand::Private::checkResource( const Resource* r ) {
    if ( r )
        return true;
    EmitResultGuard guard( q );
    KMessageBox::error( q->parentWidget(), i18n("Could not import feed list: Target resource %1 not found.", resourceIdentifier ), i18n("Import Error" ) );
    guard.emitResult();
    return false;
}

void ImportFeedListCommand::Private::doImport()
{
    //initial check for the resource
    if ( !checkResource( ResourceManager::self()->resource( resourceIdentifier ) ) )
        return;

    EmitResultGuard guard( q );
    if ( !url.isValid() ) {
        url = KFileDialog::getOpenUrl( KUrl(),
                                       QLatin1String("*.opml *.xml|")
                                       + i18n("OPML Outlines (*.opml, *.xml)")
                                       + QLatin1String("\n*|") + i18n("All Files"),
                                       q->parentWidget(), i18n("Feed List Import") );
        if ( !guard.exists() )
            return;
    }

    if ( !url.isValid() ) {
        guard.emitResult();
        return;
    }

    //the resource might be gone while the dialog was open, so re-get it
    const Resource* const resource = ResourceManager::self()->resource( resourceIdentifier );
    if ( !checkResource( resource ) )
        return;

    KRss::ImportOpmlJob* job = resource->createImportOpmlJob( url );
    connect( job, SIGNAL(finished(KJob*)), q, SLOT(importFinished(KJob*)) );
    job->start();
}

void ImportFeedListCommand::Private::importFinished( KJob* job ) {
    EmitResultGuard guard( q );
    if ( job->error() )
        KMessageBox::error( q->parentWidget(), i18n("Could not import feed list: %1", job->errorString() ), i18n("Import Error" ) );
    else
        KMessageBox::information( q->parentWidget(), i18n("The feed list was successfully imported." ), i18n("Import Finished") );
    guard.emitResult();
}

ImportFeedListCommand::ImportFeedListCommand( QObject* parent ) : Command( parent ), d( new Private( this ) )
{
}

ImportFeedListCommand::~ImportFeedListCommand()
{
    delete d;
}


void ImportFeedListCommand::setSourceUrl( const KUrl& url )
{
    d->url = url;
}

void ImportFeedListCommand::setResourceIdentifier( const QString& identifier )
{
    d->resourceIdentifier = identifier;
}

void ImportFeedListCommand::doStart()
{
    QTimer::singleShot( 0, this, SLOT(doImport()) );
}

#include "importfeedlistcommand.moc"
