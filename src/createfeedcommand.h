#ifndef AKREGATOR_CREATEFEEDCOMMAND_H
#define AKREGATOR_CREATEFEEDCOMMAND_H

#include "command.h"

namespace Akregator {

class Folder;
class SubscriptionListView;
class TreeNode;

class CreateFeedCommand : public Command
{
    Q_OBJECT
public:
    explicit CreateFeedCommand( QObject* parent = 0 );
    ~CreateFeedCommand();

    void setSubscriptionListView( SubscriptionListView* view );
    void setRootFolder( Folder* rootFolder );
    void setUrl( const QString& url );
    void setPosition( Folder* parent, TreeNode* after );
    void setAutoExecute( bool autoexec );
    
private:
    void doStart();
    void doAbort();
    
private:
    class Private;
    Private* const d;
    Q_PRIVATE_SLOT( d, void doCreate() )
};

}

#endif // AKREGATOR_CREATEFEEDCOMMAND_H
