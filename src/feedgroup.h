/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
#ifndef AKREGATORFEEDGROUP_H
#define AKREGATORFEEDGROUP_H

#include "treenode.h"

#include <qptrlist.h>

class QListViewItem;
class QDomDocument;
class QDomElement;

namespace Akregator
{
    class ArticleSequence;
    class FetchTransaction;
        
    /**
     * Represents a feed group (i.e. a folder containing feeds and/or other folders).
     */
    class FeedGroup : public TreeNode
    {
        Q_OBJECT
        public:
            /** creates a feed group parsed from a XML dom element.
            Child nodes are not inserted or parsed. 
            @param e the element representing the feed group
            @return a freshly created feed group */
            static FeedGroup* fromOPML(QDomElement e);
            
            /** The new constructor (DON'T USE YET). The idea: Just pass a parent and let parent do all the work,
            FeedsTree will be notified automatically and create an item
            etc.
            @param parent The parent node to attach to
            @param title The title of the feed group
             */          
            FeedGroup(const QString& title = QString::null); 
            
             /** Destructor. emits signalDestroyed to inform the world of our tragic demise */
            ~FeedGroup();

            /** returns recursively concatenated articles of children  
            @return an article sequence containing articles of children */
            virtual ArticleSequence articles();
            
            /** returns the number of unread articles in all children    
            @return number of unread articles */
            virtual int unread() const;
            
            /** returns the number of articles in all children
            @return number of articles */
            virtual int totalCount() const;
            
            /** Helps the rest of the app to decide if node should be handled as group or not. Use only where necessary, use polymorphism where possible. */
            virtual bool isGroup() const { return true; }
            
            /** converts the feed group into OPML format for save and export and appends it to node @c parent in document @document.
            Children are processed and appended recursively.
            @param parent The parent element 
            @param document The DOM document 
            @return The newly created element representing this feed group */
            virtual QDomElement toOPML( QDomElement parent, QDomDocument document ) const;
            
            /** returns the (direct) children of this node.
            @return a list of pointers to the child nodes
             */
            virtual QPtrList<TreeNode> children() { return m_children; }

            /** inserts @c node as child after child node @c after.
            if @c after is not a child of this group, @c node will be inserted as first child
            @param node the tree node to insert
            @param after the node after which @c node will be inserted */
            virtual void insertChild(TreeNode* node, TreeNode* after);
            
            /** inserts @c node as child on position @c index
            @param index the position where to insert
            @param node the tree node to insert */
            virtual void insertChild(uint index, TreeNode* node);

            /** inserts @c node as first child
            @param node the tree node to insert */
            virtual void prependChild(TreeNode* node);

            /** inserts @c node as last child
            @param node the tree node to insert */
            virtual void appendChild(TreeNode* node);

            /** remove @c node from children. Note that @c node will not be deleted
            @param node the child node to remove  */
            virtual void removeChild(TreeNode* node);

            /** returns the first child of the group, 0 if none exist */
            virtual TreeNode* firstChild();
            
            /** returns the last child of the group, 0 if none exist */
            virtual TreeNode* lastChild();
            
            /** returns whether the feed group is opened or not..
            Use only in \ref FeedGroupItem. */
            virtual bool isOpen() const;
            
            /** open/close the feed group (display it as expanded/collapsed in the tree view). Use only in \ref FeedGroupItem. */
            virtual void setOpen(bool open);
            
        signals:
            /** emitted when a child was added
            first param: this group, second param: the added child node*/
            void signalChildAdded(FeedGroup*, TreeNode*);

            /** emitted when a child was removed
            first param: this group, second param: the removed child node*/
            void signalChildRemoved(FeedGroup*, TreeNode*);
                       
        public slots:
            
            /** Delete expired articles recursively. */
            virtual void slotDeleteExpiredArticles();
            
            /** Mark articles of children recursively as read. */
            virtual void slotMarkAllArticlesAsRead();
 
            /** Called when a child was modified. 
            @param node the child that was changed
             */
            virtual void slotChildChanged(TreeNode* node);
            
            /** Called when a child was destroyed. 
            @param node the child that was destroyed
            */
            virtual void slotChildDestroyed(TreeNode* node);

            virtual void slotAddToFetchTransaction(FetchTransaction* transaction);
            //virtual void slotFetch(int timeout);    
            //virtual void slotAbortFetch();

            /** returns the next node in the tree.
            Calling next() unless it returns 0 iterates through the tree in pre-order
             */
            virtual TreeNode* next();
        
        protected:
            /** List of children */
            QPtrList<TreeNode> m_children;
            /** caching unread count of children */
            int m_unread;
            /** whether or not the folder is expanded */
            bool m_open;
            /** update unread count cache */
            virtual void updateUnreadCount();
    };
};

#endif
