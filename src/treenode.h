/***************************************************************************
 *   Copyright (C) 2004 by Frank Osterfeld                                 *
 *   frank.osterfeld AT kdemail.net                                        *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

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
};

};

#endif
