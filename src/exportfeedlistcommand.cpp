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

#include "exportfeedlistcommand.h"
#include "command_p.h"

#include <krss/exportopmljob.h>
#include <krss/netresource.h>
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

class ExportFeedListCommand::Private
{
    ExportFeedListCommand* const q;
public:
    explicit Private( ExportFeedListCommand* qq );

    void doExport();
    void exportFinished( KJob* );
    bool checkResource( const boost::shared_ptr<const NetResource>& r );

    KUrl url;
    QString resourceIdentifier;
};

ExportFeedListCommand::Private::Private( ExportFeedListCommand* qq )
    : q( qq )
{

}

bool ExportFeedListCommand::Private::checkResource( const boost::shared_ptr<const NetResource>& r ) {
    if ( r )
        return true;
    EmitResultGuard guard( q );
    KMessageBox::error( q->parentWidget(), i18n("Could not export feed list: Resource %1 not found.", resourceIdentifier ), i18n("Import Error" ) );
    guard.emitResult();
    return false;
}

void ExportFeedListCommand::Private::doExport()
{
    if ( !checkResource( ResourceManager::self()->resource( resourceIdentifier ) ) )
        return;
    EmitResultGuard guard( q );

    if ( !url.isValid() ) {
        url = KFileDialog::getSaveUrl( KUrl(),
                            QLatin1String("*.opml *.xml|") + i18n("OPML Outlines (*.opml, *.xml)")
                            + QLatin1String("\n*|") + i18n("All Files") );
        if ( !guard.exists() )
           return;
    }

    if ( !url.isValid() ) {
        guard.emitResult();
        return;
    }

    const boost::shared_ptr<const NetResource> resource = ResourceManager::self()->resource( resourceIdentifier );
    if ( !checkResource( resource ) )
        return;
    KRss::ExportOpmlJob* job = resource->createExportOpmlJob( url );
    connect( job, SIGNAL(finished(KJob*)), q, SLOT(exportFinished(KJob*)) );
    job->start();
}

void ExportFeedListCommand::Private::exportFinished( KJob* job ) {
    EmitResultGuard guard( q );
    if ( job->error() )
        KMessageBox::error( q->parentWidget(), i18n("Could not export feed list: %1", job->errorString() ), i18n("Export Error" ) );
    else
        KMessageBox::information( q->parentWidget(), i18n("The feed list was successfully exported." ), i18n("Export Finished") );
    guard.emitResult();
}

ExportFeedListCommand::ExportFeedListCommand( QObject* parent ) : Command( parent ), d( new Private( this ) )
{
}

ExportFeedListCommand::~ExportFeedListCommand()
{
    delete d;
}


void ExportFeedListCommand::setTargetUrl( const KUrl& url )
{
    d->url = url;
}

void ExportFeedListCommand::setResourceIdentifier( const QString& identifier )
{
    d->resourceIdentifier = identifier;
}

void ExportFeedListCommand::doStart()
{
    QTimer::singleShot( 0, this, SLOT(doExport()) );
}

#include "exportfeedlistcommand.moc"
