/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

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

#ifndef AKREGATOR_PROGRESSMANAGER_H
#define AKREGATOR_PROGRESSMANAGER_H

namespace Akregator
{

class Feed;
class TreeNode;
class ProgressItemHandler;
/** This class manages the progress items for all feeds */

class ProgressManager : public QObject
{
    Q_OBJECT
    public:
        
        static ProgressManager* self();

        ProgressManager();
        virtual ~ProgressManager();

        /** sets the feed list to be managed */
        void setFeedList(FeedList* feedList);

    protected slots:

        void slotNodeAdded(TreeNode* node);
        void slotNodeRemoved(TreeNode* node);
        void slotNodeDestroyed(TreeNode* node);

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
        ProgressItemHandler(Feed* feed);
        virtual ~ProgressItemHandler();

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
