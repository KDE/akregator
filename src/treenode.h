/*
    This file is part of Akregator.

    Copyright (C) 2004 Frank Osterfeld <osterfeld@kde.org>

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

#ifndef AKREGATOR_TREENODE_H
#define AKREGATOR_TREENODE_H

#include "akregator_export.h"
#include <QObject>
#include <QVector>
#include <QPoint>

class KJob;

class QDomDocument;
class QDomElement;
class QIcon;
class QString;
template<class T> class QList;

namespace Akregator {
class ArticleListJob;
class TreeNodeVisitor;
class Article;
class Feed;
class Folder;
class FetchQueue;

/**
    \brief Abstract base class for all kind of elements in the feed tree, like feeds and feed groups (and search folders later).

    TODO: detailed description goes here
*/
class AKREGATOR_EXPORT TreeNode : public QObject
{
    friend class ::Akregator::ArticleListJob;
    friend class ::Akregator::Folder;

    Q_OBJECT

public:

    /** Standard constructor */
    TreeNode();

    /** Standard destructor */
    ~TreeNode() override;

    virtual bool accept(TreeNodeVisitor *visitor) = 0;

    /** The unread count, returns the number of new/unread articles in the node (for groups: the accumulated count of the subtree)
    @return number of new/unread articles */

    virtual int unread() const = 0;

    /** returns the number of total articles in the node (for groups: the accumulated count of the subtree)
    @return number of articles */

    virtual int totalCount() const = 0;

    /** Get title of node.
    @return the title of the node */

    Q_REQUIRED_RESULT QString title() const;

    /** Sets the title of the node.
    @c title should not contain entities.
    @param title the title string */

    void setTitle(const QString &title);

    /** Get the next sibling.
    @return the next sibling, 0 if there is none */

    virtual const TreeNode *nextSibling() const;
    virtual TreeNode *nextSibling();

    /** Get the previous sibling.
    @return the previous sibling, 0 if there is none */

    virtual const TreeNode *prevSibling() const;
    virtual TreeNode *prevSibling();

    /** Returns the parent node.
    @return the parent feed group, 0 if there is none */

    virtual const Folder *parent() const;
    virtual Folder *parent();

    /** returns the (direct) children of this node.
        @return a list of pointers to the child nodes
     */
    virtual QList<const TreeNode *> children() const;
    virtual QList<TreeNode *> children();

    virtual QVector<const Feed *> feeds() const = 0;
    virtual QVector<Feed *> feeds() = 0;

    virtual QVector<const Folder *> folders() const = 0;
    virtual QVector<Folder *> folders() = 0;

    virtual TreeNode *childAt(int pos);
    virtual const TreeNode *childAt(int pos) const;

    /** Sets parent node; Don't call this directly, is done automatically by
    insertChild-methods in @ref Folder. */

    virtual void setParent(Folder *parent);

    virtual QIcon icon() const = 0;

    ArticleListJob *createListJob();

    /** Helps the rest of the app to decide if node should be handled as group or not. Only use where necessary, use polymorphism where possible.
    @return whether the node is a feed group or not */

    virtual bool isGroup() const = 0;

    /** returns if the node represents an aggregation, i.e. containing
     * items from more than once source feed. Folders and virtual folders
     * are aggregations, feeds are not.
     */
    virtual bool isAggregation() const = 0;

    /** exports node and child nodes to OPML (with akregator settings)
        @param parent the dom element the child node will be attached to
        @param document the opml document */

    virtual QDomElement toOPML(QDomElement parent, QDomDocument document) const = 0;

    /**
    @param doNotify notification on changes on/off flag
    */

    virtual void setNotificationMode(bool doNotify);

    /** returns the next node in the tree.
        Calling next() unless it returns 0 iterates through the tree in pre-order
     */
    virtual const TreeNode *next() const = 0;
    virtual TreeNode *next() = 0;

    /** returns the ID of this node. IDs are managed by FeedList objects and must be unique within the list. Some IDs have a special meaning:
    @c 0 is the default value and indicates that no ID was set
    @c 1 is reserved for the "All Feeds" root node */
    virtual uint id() const;

    /** sets the ID */
    virtual void setId(uint id);

    QPoint listViewScrollBarPositions() const;
    void setListViewScrollBarPositions(const QPoint &pos);

    virtual KJob *createMarkAsReadJob() = 0;

public Q_SLOTS:

    /** adds node to a fetch queue
        @param queue pointer to the queue
        @param intervalFetchesOnly determines whether to allow only interval fetches
    */
    virtual void slotAddToFetchQueue(Akregator::FetchQueue *queue, bool intervalFetchesOnly = false) = 0;

Q_SIGNALS:

    /** Emitted when this object is deleted. */
    void signalDestroyed(Akregator::TreeNode *);

    /** Notification mechanism: emitted, when the node was modified and notification is enabled. A node change is renamed title, icon, unread count. Added, updated or removed articles are not notified via this signal */
    void signalChanged(Akregator::TreeNode *);

    /** emitted when new articles were added to this node or any node in the subtree (for folders). Note that this has nothing to do with fetching, the article might have been moved from somewhere else in the tree into this subtree, e.g. by moving the feed the article is in.
        @param TreeNode* the node articles were added to
        @param guids the guids of the articles added
    */
    void signalArticlesAdded(Akregator::TreeNode *, const QVector<Akregator::Article> &guids);

    /** emitted when articles were updated */
    void signalArticlesUpdated(Akregator::TreeNode *, const QVector<Akregator::Article> &guids);

    /** emitted when articles were removed from this subtree. Note that this has nothing to do with actual article deletion! The article might have moved somewhere else in the tree, e.g. if the user moved the feed */
    void signalArticlesRemoved(Akregator::TreeNode *, const QVector<Akregator::Article> &guids);

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
    /** Returns a sequence of the articles this node contains. For feed groups, this returns a concatenated list of all articles in the sub tree.
    @return sequence of articles */

    virtual QVector<Article> articles() = 0;

private:
    bool m_doNotify = true;
    bool m_nodeChangeOccurred = false;
    bool m_articleChangeOccurred = false;
    QString m_title;
    Folder *m_parent = nullptr;
    QPoint m_scrollBarPositions;
    uint m_id = 0;
    bool m_signalDestroyedEmitted = false;
};
} // namespace Akregator

#endif // AKREGATOR_TREENODE_H
