/*
 * This file is part of Akregator
 *
 * Copyright (C) 2009 Frank Osterfeld <osterfeld@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "importitemsjob.h"

#include <krss/importitemsjob.h>
#include <krss/resource.h>

#include <KDebug>
#include <KLocalizedString>
#include <KRandom>
#include <KTemporaryFile>

#include <QFile>
#include <QPointer>
#include <QProcess>
#include <QStringList>
#include <QUrl>

#include <cassert>

using namespace Akregator;

namespace {
    static QString exporterBinaryName() {
        return QString::fromLatin1( "akregatorstorageexporter" );
    }
}
class ImportItemsJob::Private {
    ImportItemsJob* const q;
public:
    explicit Private( const KRss::Resource* res, ImportItemsJob* qq ) : q( qq ), resource( res ) {
        assert( res );
        //not using QPointer because of const
        connect( res, SIGNAL(destroyed(QObject*)), q, SLOT(resourceDestroyed(QObject*)) );
    }

    ~Private() {
        deleteTempFile();
    }

    void doStart();
    void exporterFinished( int exitCode, QProcess::ExitStatus status );
    void exporterError( QProcess::ProcessError error );
    void importFinished( KJob* job );

    void resourceDestroyed( QObject*o ) {
        assert( o == resource );
        resource = 0;
    }

    static QString generateTmpFileName() {
        KTemporaryFile file;
        file.setSuffix( KRandom::randomString( 10 ) );
        if ( !file.open() )
            return QString();
        return file.fileName();
    }

    void deleteTempFile() {
        if ( !exportFileName.isEmpty() && QFile::exists( exportFileName ) ) {
            if ( !QFile::remove( exportFileName ) )
                kWarning() << "Could not delete temporary file" << exportFileName;
        }
    }

    void cleanupAndEmitResult() {
        deleteTempFile();
        q->emitResult();
    }

    QString url;
    const KRss::Resource* resource;
    QPointer<QProcess> exporter;
    QString exportFileName;
};

ImportItemsJob::ImportItemsJob( const KRss::Resource* res, QObject* parent ) : KJob( parent ), d( new ImportItemsJob::Private( res, this ) ) {
}

ImportItemsJob::~ImportItemsJob() {
    delete d;
}

QString ImportItemsJob::xmlUrl() const {
    return d->url;
}

void ImportItemsJob::setXmlUrl( const QString& url ) {
    d->url = url;
}

void ImportItemsJob::start() {
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

void ImportItemsJob::Private::doStart() {
    assert( !exporter );
    assert( exportFileName.isEmpty() );
    exporter = new QProcess( q );
    exportFileName = generateTmpFileName();
    if ( exportFileName.isEmpty() || QFile::exists( exportFileName ) ) {
        q->setError( ImportItemsJob::ExporterError );
        q->setErrorText( i18n("Could not generate a temporary file for export." ) );
        cleanupAndEmitResult();
        return;
    }

    exporter->setStandardOutputFile( exportFileName );
    connect( exporter, SIGNAL(finished(int,QProcess::ExitStatus)),
             q, SLOT(exporterFinished(int,QProcess::ExitStatus)) );
    connect( exporter, SIGNAL(error(QProcess::ProcessError)),
             q, SLOT(exporterError(QProcess::ProcessError)) );

    exporter->start( exporterBinaryName(), QStringList() << QString::fromLatin1( "--base64" ) << QString::fromAscii( QUrl( url ).toEncoded().toBase64() ) );
    if ( !exporter->waitForStarted() ) {
        q->setError( ImportItemsJob::ExporterError );
        q->setErrorText( i18n("Could not start storage exporter %1. Make sure it is properly installed.", exporterBinaryName() ) );
        cleanupAndEmitResult();
        return;
    }
}

void ImportItemsJob::Private::exporterError( QProcess::ProcessError error ) {
    q->setError( ImportItemsJob::ExporterError );
    switch ( error ) {
        case QProcess::FailedToStart:
            q->setErrorText( i18n( "Storage exporter failed to start." ) );
            break;
        case QProcess::Crashed:
            q->setErrorText( i18n( "Storage exporter crashed." ) );
            break;
        case QProcess::Timedout:
            q->setErrorText( i18n( "Storage exporter communication timed out." ) );
            break;
        case QProcess::WriteError:
            q->setErrorText( i18n( "Storage exporter write error." ) );
            break;
        case QProcess::ReadError:
            q->setErrorText( i18n( "Storage exporter read error." ) );
            break;
        case QProcess::UnknownError:
            q->setErrorText( i18n( "Storage exporter: unknown error" ) );
            break;
    }
    cleanupAndEmitResult();
}

void ImportItemsJob::Private::exporterFinished( int exitCode, QProcess::ExitStatus status ) {
    if ( status == QProcess::CrashExit ) {
        q->setError( ImportItemsJob::ExporterError );
        q->setErrorText( i18n("Item export failed. Storage exporter %1 crashed.", exporterBinaryName() ) );
        cleanupAndEmitResult();
        return;
    }

    if ( exitCode != 0 ) {
        q->setError( ImportItemsJob::ExporterError );
        q->setErrorText( i18n("Item export failed. Storage exporter %1 returned exit code %2.", exporterBinaryName(), QString::number( exitCode ) ) );
        cleanupAndEmitResult();
        return;
    }

    if ( !resource ) {
        q->setError( ImportItemsJob::ImportFailedError );
        q->setErrorText( i18n("Item import failed, target resource was already deleted." ) );
        cleanupAndEmitResult();
        return;
    }

    KRss::ImportItemsJob* job = resource->createImportItemsJob( url, exportFileName );
    assert( job );
    connect( job, SIGNAL(finished(KJob*)), q, SLOT(importFinished(KJob*)) );
    job->start();
}

void ImportItemsJob::Private::importFinished( KJob* job ) {
    if ( job->error() ) {
        q->setError( ImportItemsJob::ImportFailedError );
        q->setErrorText( job->errorText() );
    }
    cleanupAndEmitResult();
}

#include "importitemsjob.moc"
