/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <osterfeld@kde.org>

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

#ifndef AKREGATOR_PROGRESSMANAGER_H
#define AKREGATOR_PROGRESSMANAGER_H

#include <boost/shared_ptr.hpp>

namespace Akregator
{

class Feed;
class ProgressItemHandler;
class TreeNode;

/** This class manages the progress items for all feeds */

class ProgressManager : public QObject
{
    Q_OBJECT
    public:

        static ProgressManager* self();

        ProgressManager();
        ~ProgressManager();

        /** sets the feed list to be managed */
        void setFeedList( const boost::shared_ptr<FeedList>& feedList);

    protected slots:

        void slotNodeAdded(Akregator::TreeNode* node);
        void slotNodeRemoved(Akregator::TreeNode* node);
        void slotNodeDestroyed(Akregator::TreeNode* node);

    private:

        static ProgressManager* m_self;

        class ProgressManagerPrivate;
        ProgressManagerPrivate* d;
};

/** this class handles the creation and deletion of progress items for one feed.
    This is an internal class intended to be used in ProgressManager only */

class ProgressItemHandler : public QObject
{
    Q_OBJECT
    public:
        explicit ProgressItemHandler(Feed* feed);
        ~ProgressItemHandler();

    public slots:

        void slotFetchStarted();
        void slotFetchCompleted();
        void slotFetchAborted();
        void slotFetchError();

    private:
        class ProgressItemHandlerPrivate;
        ProgressItemHandlerPrivate* d;
};

} // namespace Akregator

#endif // AKREGATOR_PROGRESSMANAGER_H
