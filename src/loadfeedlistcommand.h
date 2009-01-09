/*
 * loadfeedlistcommand.h
 *
 *  Created on: Dec 6, 2008
 *      Author: frank
 */

#ifndef AKREGATOR_LOADFEEDLISTCOMMAND_H
#define AKREGATOR_LOADFEEDLISTCOMMAND_H

#include "command.h"

#include <boost/shared_ptr.hpp>

class QDomDocument;

namespace Akregator {

    namespace Backend {
        class Storage;
    }

    class FeedList;

    class LoadFeedListCommand : public Command {
        Q_OBJECT
    public:
        explicit LoadFeedListCommand( QObject* parent = 0 );
        ~LoadFeedListCommand();

        void setFileName( const QString& fileName );
        void setDefaultFeedList( const QDomDocument& doc );
        void setStorage( Backend::Storage* storage );

    Q_SIGNALS:
        void result( const boost::shared_ptr<Akregator::FeedList>& feedList );

    private:
        void doStart();
        void doAbort();

    private:
        class Private;
        Private* const d;
        Q_PRIVATE_SLOT( d, void doLoad() )
    };

}

#endif // AKREGATOR_LOADFEEDLISTCOMMAND_H
