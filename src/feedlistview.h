/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

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
#ifndef AKREGATORFEEDLISTVIEW_H
#define AKREGATORFEEDLISTVIEW_H

#include <klistview.h>

class KURL;

namespace Akregator
{
    class Feed;
    class Folder;
    class FeedList;
    class TreeNode;
    class TreeNodeItem;
    class TagNodeList;
    
    class FeedListView : public KListView
    {
        Q_OBJECT
        public:
            FeedListView( QWidget *parent = 0, const char *name = 0 );
            ~FeedListView();
            
            /** sets the feed list to show. Disconnects from the old feed list, if there is any. */
            void setFeedList(FeedList* feedList, TagNodeList* tagNodeList=0);

            /** returns the currently displayed feed list, or 0 if none is set */

            FeedList* feedList() const;

             /** returns the currently displayed tag node list (must belong to feedList()), or 0 if none is set */
            TagNodeList* tagNodeList() const;

            /** Returns root node ("All Feeds").
             * @return root node
             */
            Folder* rootNode();
            
            /** Returns the currently selected node, @c null when no one is selected.
             @return selected node
             */
            TreeNode* selectedNode();
   
            /** selects @c node, if it exists
             * @param node the node to select
             */
            void setSelectedNode(TreeNode* node);

            /** Find first node with title @c title
                returns 0 if no node was found
            @param title
            @return node
             */ 
            TreeNode* findNodeByTitle(const QString& title);

            /** ensures that @c node is visible. */
            void ensureNodeVisible(TreeNode* node);

            /** activates in-place renaming for the item of @c node */
            void startNodeRenaming(TreeNode* node);
            
            
            /** reimplemented: clears the view and creates the root node ("All Feeds") */
            virtual void clear();

            /** if enabled, the view shows tag folders */
            void setShowTagFolders(bool enabled);

        public slots:
           
            /** go one item up */
            void slotItemUp();
            /** go one item down */
            void slotItemDown();
            /** select the first item in the list */
            void slotItemBegin();
            /** select last item in the list */
            void slotItemEnd();
            /** go to parent item */
            void slotItemLeft();
            /** go to first child */
            void slotItemRight();
      
            void slotPrevFeed();
            void slotNextFeed();
            void slotPrevUnreadFeed();
            void slotNextUnreadFeed();
            
        signals:
            void signalDropped (KURL::List &, TreeNode*, Folder*);
            void signalNodeSelected(TreeNode*);
            void signalContextMenu(KListView*, TreeNode*, const QPoint&);

        protected:

            /** Find item belonging to tree node @c node, @c null when node is not in tree
            @return item representing node
            @param node a tree node */

            TreeNodeItem* findNodeItem(TreeNode* node);

            /** reimplemented to return TreeNodeItem* */
            virtual TreeNodeItem* findItemByTitle(const QString& text, int column, ComparisonFlags compare = ExactMatch | CaseSensitive ) const;

            /** observe @c node: connect status change signals of @c node to slots */
            virtual void connectToNode(TreeNode* node);

            /** stop observing @c node: disconnect from status change signals of @c node */
            virtual void disconnectFromNode(TreeNode* node);

            virtual void connectToFeedList(FeedList* list);
            virtual void disconnectFromFeedList(FeedList* list);
            
            virtual void drawContentsOffset( QPainter * p, int ox, int oy,
                                             int cx, int cy, int cw, int ch );
            virtual void contentsDragMoveEvent(QDragMoveEvent* event);
            virtual bool acceptDrag(QDropEvent *event) const;
            virtual void movableDropEvent(QListViewItem* parent, QListViewItem* afterme);
   
            void takeNode(QListViewItem* item);
            void insertNode(QListViewItem* parent, QListViewItem* item, QListViewItem* after);

            virtual QDragObject *dragObject();
                    
        
        protected slots:
            

            void slotDropped(QDropEvent *e, QListViewItem* after);

            virtual void slotSelectionChanged(QListViewItem* item);
            virtual void slotContextMenu(KListView* list, QListViewItem* item, const QPoint& p);
            virtual void slotItemRenamed(QListViewItem* item, int col, const QString& text);
            virtual void slotFeedFetchStarted(Feed* feed);
            virtual void slotFeedFetchAborted(Feed* feed);
            virtual void slotFeedFetchError(Feed* feed);
            virtual void slotFeedFetchCompleted(Feed* feed);
            void openFolder();

            /** called when a node is added to the tree. If no item for the node exists, it will be created */
            virtual void slotNodeAdded(TreeNode* node);
            
            /** Called when a node in the tree is taken out of the tree (parent->removeChild()) 
            
            Removes a node and its children from the tree. Note that it doesn't delete the corresponding view items (get deleted only when the node itself gets deleted) */
            virtual void slotNodeRemoved(Folder* parent, TreeNode* node);
            
            /** deletes the item belonging to the deleted node */
            virtual void slotNodeDestroyed(TreeNode* node);
            
            /** update the item belonging to the node */
            virtual void slotNodeChanged(TreeNode* node);

            virtual void slotFeedListDestroyed(FeedList*);

        private:
            friend class ConnectNodeVisitor;
            class ConnectNodeVisitor;
            
            friend class DisconnectNodeVisitor;
            class DisconnectNodeVisitor;

            friend class CreateItemVisitor;
            class CreateItemVisitor;

            friend class DragAndDropVisitor;
            class DragAndDropVisitor;

            class FeedListViewPrivate;
            FeedListViewPrivate* d;
    };

}

#endif
// vim: ts=4 sw=4 et
