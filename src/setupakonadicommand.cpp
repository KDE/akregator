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

#include <Akonadi/AgentInstance>
#include <Akonadi/AgentInstanceCreateJob>
#include <Akonadi/AgentInstanceWidget>
#include <Akonadi/AgentManager>
#include <Akonadi/AgentType>
#include <Akonadi/AgentFilterProxyModel>

#include <KDialog>
#include <KLocalizedString>
#include <KMessageBox>

#include <QLabel>
#include <QVBoxLayout>

#include <cassert>

using namespace Akregator;
using namespace Akonadi;

class SetUpAkonadiCommand::Private {
    SetUpAkonadiCommand* const q;
public:
    explicit Private( SetUpAkonadiCommand* qq ) : q( qq ), agentWidget( 0 ) {}
    void startSetup();
    void dialogAccepted();
    void dialogRejected();
    void resourceCreated( KJob* );

    AgentInstanceWidget* agentWidget;
};

SetUpAkonadiCommand::SetUpAkonadiCommand( QObject* parent ) : d( new Private( this ) ) {}

void SetUpAkonadiCommand::doStart() {
    QMetaObject::invokeMethod( this, "startSetup", Qt::QueuedConnection );
}

void SetUpAkonadiCommand::Private::dialogAccepted() {
    Settings::setActiveAkonadiResource( agentWidget->currentAgentInstance().identifier() );
    q->emitResult();
}

void SetUpAkonadiCommand::Private::dialogRejected() {
    q->setError( SetUpAkonadiCommand::SetupCanceled );
    q->emitResult();
}

void SetUpAkonadiCommand::Private::resourceCreated( KJob* j ) {
    EmitResultGuard guard( q );
    const AgentInstanceCreateJob* const job = qobject_cast<const AgentInstanceCreateJob*>( j );
        assert( job );
    if ( job->error() ) {
        KMessageBox::error( q->parentWidget(), i18n( "Could not create a news feed resource: %1. Please check your installation or contact your system administrator.", job->errorString() ) );
        q->setError( SetUpAkonadiCommand::SetupFailed );
        q->setErrorText( job->errorText() );
    } else {
        assert( job->instance().isValid() );
        Settings::setActiveAkonadiResource( job->instance().identifier() );
    }
    guard.emitResult();
}

void SetUpAkonadiCommand::Private::startSetup() {
    EmitResultGuard guard( q );

    //TODO: ensure that akonadi is actually running

    const QStringList resources = KRss::ResourceManager::self()->identifiers();

    const QString id = Settings::activeAkonadiResource();
    if ( resources.contains( id ) ) {
        guard.emitResult();
        return;
    }

    if ( resources.isEmpty() ) {
       const QString typeId = QLatin1String( "akonadi_opml_rss_resource" );
       const AgentType type = AgentManager::self()->type( typeId );
       if ( !type.isValid() ) {
           KMessageBox::error( q->parentWidget(), i18n("Could not create a resource of type %1. Please check your installation or contact your system administrator.", typeId ) );
           guard.setError( SetUpAkonadiCommand::SetupFailed );
           guard.emitResult();
           return;
       }

       AgentInstanceCreateJob *job = new AgentInstanceCreateJob( type );
       connect( job, SIGNAL(result(KJob*)),
                q, SLOT(resourceCreated(KJob*)) );
       job->configure( q->parentWidget() );
       job->start();
       return;
    }

    if ( resources.size() == 1 ) {
        Settings::setActiveAkonadiResource( resources.first() );
        guard.emitResult();
        return;
    }

    assert( !agentWidget );
    agentWidget = new AgentInstanceWidget;
    agentWidget->agentFilterProxyModel()->addMimeTypeFilter( QLatin1String( "application/rss+xml" ) );

    QWidget* main = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout( main );
    //TODO, PENDING(frank): review wording for being too technical
    QLabel* label = new QLabel( i18n("Multiple news feed resources are configured. Akregator only supports one resource at a time. Please pick the resource to use in Akregator.") );
    label->setWordWrap( true );
    layout->addWidget( label );
    layout->addWidget( agentWidget );
    KDialog* dialog = new KDialog( q->parentWidget() );
    dialog->setAttribute( Qt::WA_DeleteOnClose );
    dialog->setWindowTitle( i18n("Select Resource") );
    dialog->setMainWidget( main );
    connect( dialog, SIGNAL(accepted()), q, SLOT(dialogAccepted()) );
    connect( dialog, SIGNAL(rejected()), q, SLOT(dialogRejected()) );
    dialog->show();
}

#include "setupakonadicommand.moc"
