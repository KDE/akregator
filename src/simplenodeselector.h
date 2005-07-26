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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef AKREGATOR_SIMPLENODESELECTOR_H
#define AKREGATOR_SIMPLENODESELECTOR_H

#include <qwidget.h>

class QListViewItem;

namespace Akregator {

class FeedList;
class TreeNode;

/** \brief A simple list view displaying a feed list for selection purposes
     Use this widget if you want the user to select a node from the feed list in dialogs where FeedListView is inappropriate (e.g. in a filter dialog)  */

class SimpleNodeSelector : public QWidget
{
    Q_OBJECT

    public:
        SimpleNodeSelector(FeedList* feedList, QWidget* parent, const char* name);
        virtual ~SimpleNodeSelector();

        TreeNode* selectedNode() const;
        
    public slots:
        void slotSelectNode(TreeNode* node);

    signals:
        void signalNodeSelected(TreeNode* node);

    protected slots:

        virtual void slotItemSelected(QListViewItem* item);

        virtual void slotNodeDestroyed(TreeNode* node);
        virtual void slotFeedListDestroyed(FeedList* list);

    private:

        class SimpleNodeSelectorPrivate;
        SimpleNodeSelectorPrivate* d;

        friend class NodeVisitor;
        class NodeVisitor;
};


} // namespace Akregator

#endif
