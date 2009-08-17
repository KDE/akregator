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

#ifndef AKREGATOR_SETUPAKONADICOMMAND_H
#define AKREGATOR_SETUPAKONADICOMMAND_H

#include "command.h"

namespace Akregator {

class SetUpAkonadiCommand : public Command {
    Q_OBJECT
public:
    explicit SetUpAkonadiCommand( QObject* parent=0 );

    enum Error {
        SetupCanceled=UserDefinedError,
        SetupFailed
    };

    QWidget* mainWidget() const;
    void setMainWidget( QWidget* widget );

private:
    void doStart();

private:
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void startSetup() )
    Q_PRIVATE_SLOT( d, void dialogAccepted() )
    Q_PRIVATE_SLOT( d, void dialogRejected() )
    Q_PRIVATE_SLOT( d, void resourceCreated(KJob*) )
};

}

#endif //AKREGATOR_SETUPAKONADICOMMAND_H

