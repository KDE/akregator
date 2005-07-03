
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

class QDomDocument;
class QDomElement;
class QString;
class QStringList;
template <class T> class QValueList;

namespace Akregator 
{

class TreeNodeVisitor;
class Article;
class Folder;
class FetchQueue;


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

    virtual bool accept(TreeNodeVisitor* visitor) = 0;
    
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
    
    virtual Folder* parent() const;
    
    
    /** Sets parent node; Don't call this directly, is done automatically by 
    insertChild-methods in @ref Folder. */
    
    virtual void setParent(Folder* parent);
    
    
    /** Returns a sequence of the articles this node contains. For feed groups, this returns a concatenated list of all articles in the sub tree.
    If @c tag is not null, only articles tagged with @c tag are returned
    @return sequence of articles */
    
    virtual QValueList<Article> articles(const QString& tag=QString::null) = 0;

    /** returns a list of all tags occuring in this node (sub tree for folders) */

    virtual QStringList tags() const = 0;
    
    /** Helps the rest of the app to decide if node should be handled as group or not. Only use where necessary, use polymorphism where possible.
    @return whether the node is a feed group or not */
    
    virtual bool isGroup() const = 0;

    /** exports node and child nodes to OPML (with akregator settings)
        @param parent the dom element the child node will be attached to
        @param document the opml document */
    
    virtual QDomElement toOPML( QDomElement parent, QDomDocument document ) const = 0;
    
    /** 
    @param doNotify notification on changes on/off flag
    @param notifyOccurredChanges notify changes occurred while turn off when set to true again */
    
    virtual void setNotificationMode(bool doNotify, bool notifyOccurredChanges = true);  
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

    /** adds node to a fetch queue
        @param intervalFetchesOnly */

    virtual void slotAddToFetchQueue(FetchQueue* queue, bool intervalFetchesOnly=false) = 0;
      
signals:
    
     /** Emitted when this object is deleted. */
     void signalDestroyed(TreeNode*);
 
    /** Notification mechanism: emitted, when the node was modified and notification is enabled. A node change is renamed title, icon, unread count. Added, updated or removed articles are not notified via this signal */
    void signalChanged(TreeNode*);

    /** emitted when new articles were added to this node or any node in the subtree (for folders). Note that this has nothing to do with fetching, the article might have been moved from somewhere else in the tree into this subtree, e.g. by moving the feed the article is in. For listening to newly fetched articles, you have to register yourself at @ref ArticleInterceptorManager
        @param TreeNode* the node articles were added to
        @param QStringList the guids of the articles added
    */
    void signalArticlesAdded(TreeNode*, const QValueList<Article>& guids);
    
    /** emitted when articles were updated */
    void signalArticlesUpdated(TreeNode*, const QValueList<Article>& guids);
    
    /** emitted when articles were removed from this subtree. Note that this has nothing to do with actual article deletion! The article might have moved somewhere else in the tree, e.g. if the user moved the feed */
    void signalArticlesRemoved(TreeNode*, const QValueList<Article>& guids);

protected:

    /** call this if you modified the actual node (title, unread count).
     Call this only when the _actual_ _node_ has changed, i.e. title, unread count. Don't use for article changes!
     Will do notification immediately or cache it, depending on @c m_doNotify. */
    virtual void nodeModified();
    
    /** call this if the articles in the node were changed. Sends signalArticlesAdded/Updated/Removed signals
     Will do notification immediately or cache it, depending on @c m_doNotify. */
    virtual void articlesModified();

    /** reimplement this in subclasses to do the actual notification
      called by articlesModified
    */
    virtual void doArticleNotification();

    void emitSignalDestroyed();
    
private:
    class TreeNodePrivate;
    TreeNodePrivate* d;
};

}

#endif
