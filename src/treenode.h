/*
    This file is part of Akregator.

    Copyright (C) 2004 Frank Osterfeld <frank.osterfeld at kdemail.net>

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

#ifndef AKREGATORTREENODE_H
#define AKREGATORTREENODE_H

#include <qobject.h>
#include <qstring.h>

class QDomDocument;
class QDomElement;

namespace Akregator 
{

class ArticleSequence;
class FeedGroup;
class FetchTransaction;

/**
    \brief Abstract base class for all kind of elements in the feed tree, like feeds and feed groups (and search folders later).
                   
    TODO: detailed description goes here
*/
class TreeNode : public QObject
{
Q_OBJECT

public:
        
    /** Standard constructor */
    TreeNode();

    /** Standard destructor */
    virtual ~TreeNode();


    /** The unread count, returns the number of new/unread articles in the node (for groups: the accumulated count of the subtree)
    @return number of new/unread articles */

    virtual int unread() const = 0;
    

    /** returns the number of total articles in the node (for groups: the accumulated count of the subtree)
    @return number of articles */
    
    virtual int totalCount() const = 0;

    
    /** Get title of node.
    @return the title of the node */
    
    virtual const QString& title() const;
    
    
    /** Sets the title of the node.
    @c title should not contain entities.
    @param title the title string */
    
    virtual void setTitle(const QString& title);
    
    
    /** Get the next sibling.
    @return the next sibling, 0 if there is none */
    
    virtual TreeNode* nextSibling() const;
    
    
    /** Get the previous sibling.
    @return the previous sibling, 0 if there is none */
    
    virtual TreeNode* prevSibling() const;
    
    
    /** Returns the parent node.
    @return the parent feed group, 0 if there is none */
    
    virtual FeedGroup* parent() const;
    
    
    /** Sets parent node; Don't call this directly, is done automatically by 
    insertChild-methods in @ref FeedGroup. */
    
    virtual void setParent(FeedGroup* parent);
    
    
    /** Returns a sequence of the articles this node contains. For feed groups, this returns a concatenated list of all articles in the sub tree.
    @return sequence of articles */
    
    virtual ArticleSequence articles() = 0; // TODO: insert a const version as well
    
    
    /** Helps the rest of the app to decide if node should be handled as group or not. Only use where necessary, use polymorphism where possible.
    @return whether the node is a feed group or not */
    
    virtual bool isGroup() const = 0;

    /** exports node and child nodes to OPML (with akregator settings)
        @param parent the dom element the child node will be attached to
        @param document the opml document */
    
    virtual QDomElement toOPML( QDomElement parent, QDomDocument document ) const = 0;
    
    /** 
    @param doNotify notification on changes on/off flag
    @param notifyOccuredChanges notify changes occured while turn off when set to true again */
    
    virtual void setNotificationMode(bool doNotify, bool notifyOccuredChanges = true);  
    /** returns the next node in the tree.
        Calling next() unless it returns 0 iterates through the tree in pre-order
     */ 
    virtual TreeNode* next() = 0; 

    /** returns the ID of this node. IDs are managed by @ref FeedList objects and must be unique within the list. Some IDs have a special meaning:
    @c 0 is the default value and indicates that no ID was set
    @c 1 is reserved for the "All Feeds" root node */
    
    virtual uint id() const;

    /** sets the ID */
    virtual void setId(uint id);
    
public slots:
    
    /** Deletes all expired articles in the node (depending on the expiry settings).
        Works recursively for feed groups. */
    
    virtual void slotDeleteExpiredArticles() = 0;
    
    
    /** Marks all articles in this node as read.
    Works recursively for feed groups. */
    
    virtual void slotMarkAllArticlesAsRead() = 0;

    /** adds node to a fetch transaction */
    
    virtual void slotAddToFetchTransaction(FetchTransaction* transaction) = 0;
    //virtual void slotFetch(int timeout) = 0;    
    //virtual void slotAbortFetch() = 0;
      
signals:
    
    /** Notification mechanism: emitted, when the node was modified and notification 
    is enabled. */
    
    void signalChanged(TreeNode*);
    
    
    /** Emitted when this object is deleted. */
    
    void signalDestroyed(TreeNode*);   
    
    
    /** TODO: not used yet */
    
    void signalFetched();
    
    
    /** TODO: not used yet */
    
    void signalFetchAborted();
    
    
    /** TODO: not used yet */
    
    void signalFetchTimeout();

    
protected:    
    
    /** call this if you modified the object. Will do notification immediately or cache it, depending on @c m_doNotify. */
    virtual void modified();
    
    /** If set to true, signalChanged is emitted when the node was modified */
    bool m_doNotify;
   
    /** If m_doNotify is set false, this flag caches occurred changes. */
    bool m_changeOccured;
    
    /** title of the node */
    QString m_title;
    
    /** The node's parent */
    FeedGroup* m_parent;

    uint m_id;
};

}

#endif
