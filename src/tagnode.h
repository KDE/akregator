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

#ifndef AKREGATOR_TAGNODE_H
#define AKREGATOR_TAGNODE_H

#include <treenode.h>

class QDomDocument;
class QDomElement;
class QString;
class QStringList;
template <class T> class QValueList;

namespace Akregator 
{

class Article;
class Folder;
class FetchQueue;
class Tag;
class TreeNodeVisitor;

class TagNode : public TreeNode
{
Q_OBJECT

public:

    /** creates a tag node, showing articles with a specific tag, from a given node.
        @param tagId the identifier of the tag, as stored in the archive
        @param observed the tag node will list the tagged articles of @c observed (usually "All Feeds" root node)
        @param title the user visible string used to display the tag
     
    */
    TagNode(const Tag& tag, TreeNode* observed);
    
    virtual ~TagNode();

    virtual bool accept(TreeNodeVisitor* visitor);

    virtual Tag tag() const;
    
    /** The unread count, returns the number of new/unread articles in the node (for groups: the accumulated count of the subtree)
    @return number of new/unread articles */

    virtual int unread() const;
    

    /** returns the number of total articles in the node (for groups: the accumulated count of the subtree)
    @return number of articles */
    
    virtual int totalCount() const;

    
    /** Returns a sequence of the articles this node contains. For feed groups, this returns a concatenated list of all articles in the sub tree.
    If @c tag is not null, only articles tagged with @c tag are returned
    @return sequence of articles */
    
    virtual QValueList<Article> articles(const QString& tag=QString::null);

    /** returns a list of all tags occurring in this node (sub tree for folders) */

    virtual QStringList tags() const;
    
    /** Helps the rest of the app to decide if node should be handled as group or not. Only use where necessary, use polymorphism where possible.
    @return whether the node is a feed group or not */
    
    virtual bool isGroup() const {return false;}

    /** reimplemented to return an invalid element */
    
    virtual QDomElement toOPML( QDomElement parent, QDomDocument document ) const;
    
    /** returns the next node in the tree.
        Calling next() unless it returns 0 iterates through the tree in pre-order
     */ 
    virtual TreeNode* next(); 

    virtual void setTitle(const QString& title);
    
public slots:
    

    /** does nothing for TagNode */  
    virtual void slotDeleteExpiredArticles();

    /** marks all articles as read */
    virtual void slotMarkAllArticlesAsRead();

    /** does nothing for TagNode */
    virtual void slotAddToFetchQueue(FetchQueue* queue, bool intervalFetchOnly=false);
    
    virtual void slotArticlesAdded(TreeNode* node, const QValueList<Article>& list);
    virtual void slotArticlesUpdated(TreeNode* node, const QValueList<Article>& list);
    virtual void slotArticlesRemoved(TreeNode* node, const QValueList<Article>& list);
    virtual void slotObservedDestroyed(TreeNode* node);
    
protected:

    virtual void doArticleNotification();
    virtual void calcUnread();
    
private:
    class TagNodePrivate;
    TagNodePrivate* d;
};

}

#endif // AKREGATOR_TAGNODE_H
