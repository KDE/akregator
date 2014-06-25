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

#include "feedlist.h"
#include "folder.h"
#include "kernel.h"

#include <QDebug>
#include <QInputDialog>
#include <KLocalizedString>

#include <QDomDocument>
#include <QPointer>
#include <QTimer>

#include <boost/shared_ptr.hpp>

#include <cassert>

using namespace boost;
using namespace Akregator;

class ImportFeedListCommand::Private
{
    ImportFeedListCommand* const q;
public:
    explicit Private( ImportFeedListCommand* qq );

    void doImport();

    weak_ptr<FeedList> targetList;
    QDomDocument document;
    ImportFeedListCommand::RootFolderOption rootFolderOption;
    QString importedRootFolderName;
};

ImportFeedListCommand::Private::Private( ImportFeedListCommand* qq )
    : q( qq )
    , targetList()
    , rootFolderOption( Ask )
    , importedRootFolderName( i18n("Imported Feeds") )
{

}

void ImportFeedListCommand::Private::doImport()
{
    const shared_ptr<FeedList> target = targetList.lock();

    if ( !target )
    {
        if ( !target )
            qWarning() << "Target list was deleted, could not import feed list";
        q->done();
        return;
    }

    std::auto_ptr<FeedList> importedList( new FeedList( Kernel::self()->storage() ) );
    const bool parsed = importedList->readFromOpml( document );

    // FIXME: parsing error, print some message
    if (!parsed) {
        q->done();
        return;
    }


    QPointer<QObject> that( q );

    bool ok=false;

    if ( rootFolderOption == ImportFeedListCommand::Ask )
        importedRootFolderName = QInputDialog::getText( q->parentWidget(), i18n("Add Imported Folder"),
                i18n("Imported folder name:"), QLineEdit::Normal,
                importedRootFolderName,
                &ok);


    if ( !ok || !that ) {
        if ( that )
            q->done();
        return;
    }

    Folder* folder = target->allFeedsFolder();

    if ( rootFolderOption != None ) {
        folder = new Folder( importedRootFolderName );
        target->allFeedsFolder()->appendChild( folder );
    }

    target->append( importedList.get(), folder );

    q->done();
}

ImportFeedListCommand::ImportFeedListCommand( QObject* parent ) : Command( parent ), d( new Private( this ) )
{
}

ImportFeedListCommand::~ImportFeedListCommand()
{
    delete d;
}

void ImportFeedListCommand::setTargetList( const weak_ptr<FeedList>& feedList )
{
    d->targetList = feedList;
}

void ImportFeedListCommand::setImportedRootFolderOption( RootFolderOption opt ) {
    d->rootFolderOption = opt;
}

void ImportFeedListCommand::setImportedRootFolderName( const QString& defaultName ) {
    d->importedRootFolderName = defaultName;
}

void ImportFeedListCommand::setFeedListDocument( const QDomDocument& doc ) {
    d->document = doc;
}

void ImportFeedListCommand::doAbort()
{
    //TODO
}

void ImportFeedListCommand::doStart()
{
    QTimer::singleShot( 0, this, SLOT(doImport()) );
}

#include "moc_importfeedlistcommand.cpp"
