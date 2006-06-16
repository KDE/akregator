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

#include <QWidget>

#include <kdialog.h>

class Q3ListViewItem;

namespace Akregator {

class FeedList;
class TreeNode;

class SimpleNodeSelector;

/** \brief A dialog with a simple listview displaying a feed list for selection purposes
     Use this dialog if you want the user to select a node from the feed list where FeedListView is inappropriate (e.g. in a filter dialog)  */

class SelectNodeDialog : public KDialog
{
    Q_OBJECT
    public:
        SelectNodeDialog(FeedList* feedList, QWidget* parent=0, char* name=0);
        virtual ~SelectNodeDialog();
        
        TreeNode* selectedNode() const;

    public slots:

        virtual void slotSelectNode(TreeNode* node);

    protected slots:
        
        virtual void slotNodeSelected(TreeNode* node);
 
    private:
       class SelectNodeDialogPrivate;
       SelectNodeDialogPrivate* d;
};

class SimpleNodeSelector : public QWidget
{
    Q_OBJECT

    public:
        SimpleNodeSelector(FeedList* feedList, QWidget* parent=0, const char* name=0);
        virtual ~SimpleNodeSelector();

        TreeNode* selectedNode() const;
        
    public slots:
        void slotSelectNode(TreeNode* node);

    signals:
        void signalNodeSelected(TreeNode* node);

    protected slots:

        virtual void slotItemSelected(Q3ListViewItem* item);

        virtual void slotNodeDestroyed(TreeNode* node);
        virtual void slotFeedListDestroyed(FeedList* list);

    private:

        class SimpleNodeSelectorPrivate;
        SimpleNodeSelectorPrivate* d;

        friend class NodeVisitor;
        class NodeVisitor;
};


} // namespace Akregator

#endif // AKREGATOR_SIMPLENODESELECTOR_H
