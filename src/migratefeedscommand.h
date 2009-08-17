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

#ifndef AKREGATOR_MIGRATEFEEDSCOMMAND_H
#define AKREGATOR_MIGRATEFEEDSCOMMAND_H

#include "command.h"

class QWidget;

namespace Akregator {
    class MigrateFeedsCommand : public Command {
        Q_OBJECT
    public:
        explicit MigrateFeedsCommand( QObject* parent=0 );
        ~MigrateFeedsCommand();

        void setResource( const QString& id );
        void setOpmlFile( const QString& path );

        enum Error {
            ResourceNotFound=KJob::UserDefinedError
        };

    private:
        /* reimp */ void doStart();

    private:
        class Private;
        Private* const d;
        Q_PRIVATE_SLOT( d, void doDoStart() )
        Q_PRIVATE_SLOT( d, void opmlImportFinished( KJob* ) )
        Q_PRIVATE_SLOT( d, void itemImportFinished( KJob* ) )
        Q_PRIVATE_SLOT( d, void currentIdChanged( int ) )
        Q_PRIVATE_SLOT( d, void wizardClosed() )
        Q_PRIVATE_SLOT( d, void feedListRetrievalFinished( KJob* ) )
    };
}

#endif // AKREGATOR_MIGRATEFEEDSCOMMAND_H
