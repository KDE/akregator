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
    class FeedsCollection;
    
    /**
     * Represents a feed group (i.e. a folder containing feeds and/or other folders).
     */
    class FeedGroup : public TreeNode
    {
        Q_OBJECT
        public:
            /** The new constructor (DON'T USE YET). The idea: Just pass a parent and let parent do all the work,
            FeedsTree will be notified automatically and create an item
            etc.
            @param parent The parent node to attach to
            @param title The title of the feed group
             */          
            FeedGroup(FeedGroup* parent, QString title); 
            
            /** Old, deprecated constructor, will die (but has to be used now).
            @param item The view item belonging to this node (will be handled by FeedsTree)
            @param collection feed collection for item->node mapping */
            FeedGroup(QListViewItem* item, FeedsCollection* collection);
            
            /** Destructor. emits signalDestroyed to inform the world of our tragic demise */
            ~FeedGroup();

            /** returns recursively concatenated articles of children  
            @return an article sequence containing articles of children */
            virtual ArticleSequence articles();
            
            /** returns the number of unread articles in all children    
            @return number of unread articles */
            virtual int unread() const;
                       
            /** Helps the rest of the app to decide if node should be handled as group or not. Use only where necessary, use polymorphism where possible. */
            virtual bool isGroup() const { return true; }
            
            /** converts the feed group into OPML format for save and export and appends it to node @c parent in document @document. 
            @param parent The parent element 
            @param document The DOM document 
            @return The newly created element representing this feed group */
            virtual QDomElement toOPML( QDomElement parent, QDomDocument document ) const;
            
            /** returns the (direct) children of this node.
            @return a list of pointers to the child nodes
             */
            virtual QPtrList<TreeNode> children() { return m_children; }
            
        public slots:
            
            /** Delete expired articles recursively. */
            virtual void slotDeleteExpiredArticles();
            
            /** Mark articles of children recursively as read. */
            virtual void slotMarkAllArticlesAsRead();
            
            /** Called when a child was modified. */
            virtual void slotChildChanged();
            
            //virtual void slotFetch(int timeout);    
            //virtual void slotAbortFetch();
    
        protected:
            /** List of children */
            QPtrList<TreeNode> m_children;
    };
};

#endif
