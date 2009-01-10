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

#ifndef AKREGATOR_IMPORTFEEDLISTCOMMAND_H
#define AKREGATOR_IMPORTFEEDLISTCOMMAND_H

#include "command.h"

#include <boost/weak_ptr.hpp>

class QDomDocument;

namespace Akregator {

class FeedList;

class ImportFeedListCommand : public Command
{
    Q_OBJECT
public:
    explicit ImportFeedListCommand( QObject* parent = 0 );
    ~ImportFeedListCommand();


    void setTargetList( const boost::weak_ptr<FeedList>& feedList );

    enum RootFolderOption {
        None,
        Auto,
        Ask
    };

    void setImportedRootFolderOption( RootFolderOption opt );
    void setImportedRootFolderName( const QString& defaultName );

    void setFeedListDocument( const QDomDocument& doc );

private:
    void doStart();
    void doAbort();

private:
    class Private;
    Private* const d;
    Q_PRIVATE_SLOT( d, void doImport() )
};

}

#endif // AKREGATOR_IMPORTFEEDLISTCOMMAND_H
