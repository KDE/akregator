#ifndef AKREGATOR_ACTIONS_H
#define AKREGATOR_ACTIONS_H

class QAction;
class QObject;

class KUrl;

namespace Akregator {

QAction* createOpenLinkInNewTabAction( const KUrl& url, QObject* receiver=0, const char* slot=0, QObject* parent = 0 );
QAction* createOpenLinkInExternalBrowserAction( const KUrl& url, QObject* receiver=0, const char* slot=0, QObject* parent = 0 );
    
}

#endif // AKREGATOR_ACTIONS_H
