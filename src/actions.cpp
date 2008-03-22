#include "actions.h"

#include <KIcon>
#include <KLocalizedString>
#include <KUrl>

#include <QAction>

QAction* Akregator::createOpenLinkInNewTabAction( const KUrl& url, QObject* receiver, const char* slot, QObject* parent )
{
    QAction* action = new QAction( KIcon( "tab-new" ), i18n( "Open Link in New &Tab" ), parent );
    action->setData( url );
    if ( receiver && slot )
        QObject::connect( action, SIGNAL( triggered( bool ) ), receiver, slot );
    return action;
}

QAction* Akregator::createOpenLinkInExternalBrowserAction( const KUrl& url, QObject* receiver, const char* slot, QObject* parent )
{
    QAction* action = new QAction( KIcon( "window-new" ), i18n( "Open Link in External &Browser" ), parent );
    action->setData( url );
    if ( receiver && slot )
        QObject::connect( action, SIGNAL( triggered( bool ) ), receiver, slot );
    return action;
}

