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

#include "loadfeedlistcommand.h"

#include "feedlist.h"
#include "storage.h"

#include <KLocale>
#include <KMessageBox>
#include <KRandom>

#include <QDateTime>
#include <QDomDocument>
#include <QFile>
#include <QPointer>
#include <QString>
#include <QTimer>

#include <cassert>

using namespace boost;
using namespace Akregator;
using namespace Akregator::Backend;

class LoadFeedListCommand::Private {
    LoadFeedListCommand* const q;
public:
    explicit Private( LoadFeedListCommand* qq ) : q( qq ), storage( 0 ) {}
    void handleDocument( const QDomDocument& doc );
    QString createBackup( const QString& path, bool* ok );
    void emitResult( const shared_ptr<FeedList>& list );
    void doLoad();

    QString fileName;
    QDomDocument defaultFeedList;
    Storage* storage;
};

void LoadFeedListCommand::Private::emitResult( const shared_ptr<FeedList>& list ) {
    emit q->result( list );
    q->done();
}

void LoadFeedListCommand::Private::handleDocument( const QDomDocument& doc ) {
    shared_ptr<FeedList> feedList( new FeedList( storage ) );
    if ( !feedList->readFromOpml( doc ) ) {
        bool backupCreated;
        const QString backupFile = createBackup( fileName, &backupCreated );
        const QString msg = backupCreated
            ? i18n("<qt>The standard feed list is corrupted (invalid OPML). A backup was created:<p><b>%1</b></p></qt>", backupFile )
            : i18n("<qt>The standard feed list is corrupted (invalid OPML). Could not create a backup.</p></qt>" );

        QPointer<QObject> that( q );
        KMessageBox::error( q->parentWidget(), msg, i18n("OPML Parsing Error") );
        if ( !that )
            return;
        feedList.reset();
    }
    emitResult( feedList );
}

QString LoadFeedListCommand::Private::createBackup( const QString& path, bool* ok ) {
    const QString backup = path
        + QLatin1String("-backup.")
        + QString::number(QDateTime::currentDateTime().toTime_t());

    const bool copied = QFile::copy( path, backup );
    if ( ok )
        *ok = copied;
    return backup;
}

LoadFeedListCommand::LoadFeedListCommand( QObject* parent ) : Command( parent ), d( new Private( this ) ) {
}

LoadFeedListCommand::~LoadFeedListCommand() {
    delete d;
}

void LoadFeedListCommand::setFileName( const QString& fileName ) {
    d->fileName = fileName;
}
void LoadFeedListCommand::setDefaultFeedList( const QDomDocument& doc ) {
    d->defaultFeedList = doc;
}

void LoadFeedListCommand::setStorage( Backend::Storage* s ) {
    d->storage = s;
}

void LoadFeedListCommand::doStart() {
    QTimer::singleShot( KRandom::random() % 400, this, SLOT(doLoad()) );
}

void LoadFeedListCommand::doAbort() {

}

void LoadFeedListCommand::Private::doLoad() {
    assert( storage );
    assert( !fileName.isNull() );
    emit q->progress( 0, i18n("Opening Feed List...") );


    QString str;

    const QString listBackup = storage->restoreFeedList();

    QDomDocument doc;

    if ( !QFile::exists( fileName ) ) {
        handleDocument( defaultFeedList );
        return;
    }

    QFile file( fileName );

    if ( !file.open( QIODevice::ReadOnly ) ) {
        QPointer<QObject> that( q );
        KMessageBox::error( q->parentWidget(), i18n( "<qt>Could not open feed list (%1) for reading.</p></qt>", file.fileName() ), i18n( "Read Error" ) );
        if ( that )
            handleDocument( defaultFeedList );
        return;
    }

    QString errMsg;
    int errLine = 0;
    int errCol = 0;
    if ( !doc.setContent( &file, true, &errMsg, &errLine, &errCol ) ) {
        bool backupCreated = false;
        const QString backupFile = createBackup( fileName, &backupCreated );
        const QString title = i18nc( "error message window caption", "XML Parsing Error" );
        const QString details = i18n( "<qt><p>XML parsing error in line %1, column %2 of %3:</p><p>%4</p></qt>",
                                      QString::number( errLine ),
                                      QString::number( errCol ),
                                      fileName,
                                      errMsg );
        const QString msg = backupCreated
            ? i18n( "<qt>The standard feed list is corrupted (invalid XML). A backup was created:<p><b>%1</b></p></qt>", backupFile )
            : i18n( "<qt>The standard feed list is corrupted (invalid XML). Could not create a backup.</p></qt>" );

        QPointer<QObject> that( q );

        KMessageBox::detailedError( q->parentWidget(), msg, details, title );

        if ( that )
            handleDocument( defaultFeedList );
        return;
    }

    handleDocument( doc );
}

#include "loadfeedlistcommand.moc"
