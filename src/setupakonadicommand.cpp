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

#include "setupakonadicommand.h"
#include "akregatorconfig.h"
#include "command_p.h"

#include <krss/resourcemanager.h>

#include <KLocalizedString>
#include <KMessageBox>

using namespace Akregator;

class SetUpAkonadiCommand::Private {
    SetUpAkonadiCommand* const q;
public:
    explicit Private( SetUpAkonadiCommand* qq ) : q( qq ) {}
    void startSetup();
};

SetUpAkonadiCommand::SetUpAkonadiCommand( QObject* parent ) : d( new Private( this ) ) {}

void SetUpAkonadiCommand::doStart() {
    QMetaObject::invokeMethod( this, "startSetup", Qt::QueuedConnection );
}

void SetUpAkonadiCommand::Private::startSetup() {
    EmitResultGuard guard( q );
    const QStringList resources = KRss::ResourceManager::self()->identifiers();

    const QString id = Settings::activeAkonadiResource();
    if ( resources.contains( id ) ) {
        guard.emitResult();
        return;
    }

    if ( resources.isEmpty() ) {
        KMessageBox::error( q->parentWidget(), i18n("FIXME Resource creation from within Akregator not yet supported. Please create a resource manually.") );
        guard.setError( AkonadiSetupFailed );
        guard.emitResult();
        return;
    }

    Settings::setActiveAkonadiResource( resources.first() );
    guard.emitResult();
}

#include "setupakonadicommand.moc"
