/*
    This file is part of Akregator.

    Copyright (C) 2005 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef STORAGE_H
#define STORAGE_H

#include <qobject.h>

class QString;
class QStringList;

namespace Akregator {
namespace Backend {

class FeedStorage;

/** \brief Storage is the main interface to the article archive. It creates and manages FeedStorage objects handling the article list for a feed.
    
    An archive implementation must implement Storage, FeedStorage and StorageFactory. See mk4storage for an example.
*/
class Storage : public QObject
{
    public:

        static Storage* getInstance();
        static void setInstance(Storage* instance);

        virtual ~Storage() {}

        /** initializes the storage object with given parameters */
        
        virtual void initialize(const QStringList& params) = 0;
        
        /**
         * Open storage and prepare it for work.
         * @return true on success.
         */
        virtual bool open(bool autoCommit = false) = 0;

        /**
         * Commit changes made in feeds and articles, making them persistent.
         * @return true on success.
         */
        virtual bool commit() = 0;

        /**
         * Rollback changes made in feeds and articles, reverting to last committed values.
         * @returns true on success.
         */
        virtual bool rollback() = 0;

        /**
         * Closes storage, freeing all allocated resources. Called from destructor, so you don't need to call it directly.
         * @return true on success.
         */
        virtual bool close() = 0;

        /**
         * @return Article archive for feed at given url.
         */
        virtual FeedStorage* archiveFor(const QString &url) = 0;
        virtual bool autoCommit() const  = 0;
        virtual int unreadFor(const QString &url) = 0;
        virtual void setUnreadFor(const QString &url, int unread) = 0;
        virtual int totalCountFor(const QString &url) = 0;
        virtual void setTotalCountFor(const QString &url, int total) = 0;
        virtual int lastFetchFor(const QString& url) = 0;
        virtual void setLastFetchFor(const QString& url, int lastFetch) = 0;

     private:
        
        static Storage *m_instance;
};

}
}

#endif
