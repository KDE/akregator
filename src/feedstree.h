/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
#ifndef AKREGATORFEEDSTREE_H
#define AKREGATORFEEDSTREE_H

#include <qptrdict.h>

#include <klistview.h>
#include <kurl.h>


namespace Akregator
{
    class Feed;
    class FeedGroup;
    class FeedGroupItem;
    class TreeNode;
    class TreeNodeItem;
    
    class FeedsTree : public KListView
    {
        Q_OBJECT
        public:
            FeedsTree( QWidget *parent = 0, const char *name = 0 );
            ~FeedsTree();

            /** Returns root node ("All Feeds").
             * @return root node
             */
            FeedGroup* rootNode();
            
            /** Returns the currently selected node, @c null when no one is selected.
             @return selected node
             */
            TreeNode* selectedNode();
   
            /** selects @c node, if it exists
             * @param node the node to select
             */
            void setSelectedNode(TreeNode* node);
            
            /** Find item belonging to tree node @c node, @c null when node is not in tree
            @return item representing node  
            @param node a tree node
             */            
            TreeNodeItem* findNodeItem(TreeNode* node);
            
            /** Find first node with title @c title
                returns 0 if no node was found
            @param title
            @return node
             */ 
            TreeNode* findNodeByTitle(const QString& title);

            /** ensures that @c node is visible. */
            void ensureNodeVisible(TreeNode* node);

            /** reimplemented to return TreeNodeItem* */
            virtual TreeNodeItem* findItem (const QString& text, int column, ComparisonFlags compare = ExactMatch | CaseSensitive ) const;

            /** reimplemented: clears the view and creates the root node ("All Feeds") */
            virtual void clear();

        protected:
             /** Returns item belonging to currently selected node. 
            @return selected node item
              */
            TreeNodeItem* selectedNodeItem();

            /** Returns item belonging to root node.
            @return root node item
             */
            FeedGroupItem* rootNodeItem();

            /** observe @c node: connect status change signals of @c node to slots */
            virtual void connectToNode(TreeNode* node);
            /** stop observing @c node: disconnect from status change signals of @c node */
            virtual void disconnectFromNode(TreeNode* node);
            
            virtual void drawContentsOffset( QPainter * p, int ox, int oy,
                                       int cx, int cy, int cw, int ch );
            virtual void contentsDragMoveEvent(QDragMoveEvent* event);
            virtual bool acceptDrag(QDropEvent *event) const;
            virtual void movableDropEvent(QListViewItem* parent, QListViewItem* afterme);
            virtual void keyPressEvent(QKeyEvent* e);
            
            void takeNode(QListViewItem* item);
            void insertNode(QListViewItem* parent, QListViewItem* item, QListViewItem* after);
            
        signals:
            void dropped (KURL::List &, TreeNodeItem*, FeedGroupItem*);
            void signalNodeSelected(TreeNode*);
        
        public slots:
           
            /** handle dropped urls */
            void slotDropped(QDropEvent *e, QListViewItem *after);
            /** Shows only first-level feeds */
            void slotCollapseAll();
            /** Shows only root of selected tree */
            void slotCollapse();
            /** Shows all feeds */
            void slotExpandAll();
            /** Shows all feeds of selected tree */
            void slotExpand();
            /** Move feed up */
            void slotItemUp();
            /** Move feed down */
            void slotItemDown();
            /** Move feed at the beginning of current list */
            void slotItemBegin();
            /** Move feed at the end of current list */
            void slotItemEnd();
            /** Move feed level up (to enclosing group) */
            void slotItemLeft();
            /** Move feed level down */
            void slotItemRight();
      
            void slotPrevFeed();
            void slotNextFeed();
            void slotPrevUnreadFeed();
            void slotNextUnreadFeed();
            
            /** called when a node is added to the tree. If no item for the node exists, it will be created */
            virtual void slotNodeAdded(FeedGroup* parent, TreeNode* node);
            
            /** Called when a node in the tree is taken out of the tree (parent->removeChild()) 
            
            Removes a node and its children from the tree. Note that it doesn't delete the corresponding view items (get deleted only when the node itself gets deleted) */
            virtual void slotNodeRemoved(FeedGroup* parent, TreeNode* node);
            
            /** deletes the item belonging to the deleted node */
            virtual void slotNodeDestroyed(TreeNode* node);
            
            /** update the item belonging to the node */
            virtual void slotNodeChanged(TreeNode* node);

        protected slots:
            virtual void slotSelectionChanged(QListViewItem* item); 
            virtual void slotItemRenamed(QListViewItem* item);
            virtual void slotFeedFetchStarted(Feed* feed);
            virtual void slotFeedFetchAborted(Feed* feed);
            virtual void slotFeedFetchError(Feed* feed);
            virtual void slotFeedFetchCompleted(Feed* feed);

        protected:
            virtual QDragObject *dragObject();

        private:
            /** used for finding the item belonging to a node */
            QPtrDict<TreeNodeItem> m_itemDict;
    };

}

#endif
// vim: ts=4 sw=4 et
