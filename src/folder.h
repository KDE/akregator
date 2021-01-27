/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
        SPDX-FileCopyrightText: 2004-2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef AKREGATOR_FOLDER_H
#define AKREGATOR_FOLDER_H

#include "akregator_export.h"
#include "treenode.h"

class QDomDocument;
class QDomElement;
template<class T> class QList;

namespace Akregator
{
class Article;
class FetchQueue;
class TreeNodeVisitor;

/** Represents a folder (containing feeds and/or other folders)
 */
class AKREGATOR_EXPORT Folder : public TreeNode
{
    Q_OBJECT
public:
    /** creates a feed group parsed from a XML dom element.
    Child nodes are not inserted or parsed.
    @param e the element representing the feed group
    @return a freshly created feed group */
    static Folder *fromOPML(const QDomElement &e);

    /** Creates a new folder with a given title
    @param title The title of the feed group
        */
    explicit Folder(const QString &title = QString());

    ~Folder() override;

    bool accept(TreeNodeVisitor *visitor) override;

    /** returns the number of unread articles in all children
    @return number of unread articles */
    Q_REQUIRED_RESULT int unread() const override;

    /** returns the number of articles in all children
    @return number of articles */
    Q_REQUIRED_RESULT int totalCount() const override;

    /** Helps the rest of the app to decide if node should be handled as group or not. */
    Q_REQUIRED_RESULT bool isGroup() const override
    {
        return true;
    }

    // impl
    Q_REQUIRED_RESULT bool isAggregation() const override
    {
        return true;
    }

    /** converts the feed group into OPML format for save and export and appends it to node @p parent in document @p document.
    Children are processed and appended recursively.
    @param parent The parent element
    @param document The DOM document
    @return The newly created element representing this feed group */
    QDomElement toOPML(QDomElement parent, QDomDocument document) const override;

    /** returns the (direct) children of this node.
    @return a list of pointers to the child nodes
        */

    Q_REQUIRED_RESULT QList<const TreeNode *> children() const override;

    Q_REQUIRED_RESULT QList<const TreeNode *> namedChildren(const QString &title) const;
    Q_REQUIRED_RESULT QList<TreeNode *> namedChildren(const QString &title);

    Q_REQUIRED_RESULT QList<TreeNode *> children() override;

    Q_REQUIRED_RESULT QVector<const Feed *> feeds() const override;
    Q_REQUIRED_RESULT QVector<Feed *> feeds() override;
    Q_REQUIRED_RESULT QVector<const Folder *> folders() const override;
    Q_REQUIRED_RESULT QVector<Folder *> folders() override;

    Q_REQUIRED_RESULT int indexOf(const TreeNode *node) const;

    TreeNode *childAt(int pos) override;

    const TreeNode *childAt(int pos) const override;

    /** inserts @c node as child after child node @c after.
    if @c after is not a child of this group, @c node will be inserted as first child
    @param node the tree node to insert
    @param after the node after which @c node will be inserted */
    void insertChild(TreeNode *node, TreeNode *after);

    /** inserts @c node as first child
    @param node the tree node to insert */
    void prependChild(TreeNode *node);

    /** inserts @c node as last child
    @param node the tree node to insert */
    void appendChild(TreeNode *node);

    /** remove @c node from children. Note that @c node will not be deleted
    @param node the child node to remove  */
    void removeChild(TreeNode *node);

    /** returns the first child of the group, 0 if none exist */
    TreeNode *firstChild();
    const TreeNode *firstChild() const;

    /** returns the last child of the group, 0 if none exist */
    TreeNode *lastChild();
    const TreeNode *lastChild() const;

    /** returns whether the feed group is opened or not.
    Use only in \ref Folder . */
    Q_REQUIRED_RESULT bool isOpen() const;

    /** open/close the feed group (display it as expanded/collapsed in the tree view). Use only in \ref Folder . */
    void setOpen(bool open);

    /** returns the next node in the tree.
    Calling next() unless it returns 0 iterates through the tree in pre-order
        */
    TreeNode *next() override;
    const TreeNode *next() const override;

    Q_REQUIRED_RESULT QIcon icon() const override;

    /* returns whether @p node is among the direct or indirect children of this
     * folder
     */
    Q_REQUIRED_RESULT bool subtreeContains(const Akregator::TreeNode *node) const;

    KJob *createMarkAsReadJob() override;

Q_SIGNALS:
    /** emitted when a child was added */
    void signalChildAdded(Akregator::TreeNode *);

    /** emitted when a child was removed */
    void signalChildRemoved(Akregator::Folder *, Akregator::TreeNode *);

    void signalAboutToRemoveChild(Akregator::TreeNode *);

public Q_SLOTS:

    /** Called when a child was modified.
    @param node the child that was changed
        */
    void slotChildChanged(Akregator::TreeNode *node);

    /** Called when a child was destroyed.
    @param node the child that was destroyed
    */
    void slotChildDestroyed(Akregator::TreeNode *node);

    /** enqueues children recursively for fetching
    @param queue a fetch queue
    @param intervalFetchesOnly determines whether to allow only interval fetches
    */
    void slotAddToFetchQueue(Akregator::FetchQueue *queue, bool intervalFetchesOnly = false) override;

protected:
    /** inserts @c node as child on position @c index
    @param index the position where to insert
    @param node the tree node to insert */
    void insertChild(int index, TreeNode *node);

    void doArticleNotification() override;

private:
    QVector<Article> articles() override;

    void connectToNode(TreeNode *child);
    void disconnectFromNode(TreeNode *child);

    void updateUnreadCount() const;
    /** List of children */
    QList<TreeNode *> m_children;
    /** caching unread count of children */
    mutable int m_unread = 0;
    /** whether or not the folder is expanded */
    bool m_open = false;
};
} // namespace Akregator

#endif // AKREGATOR_FOLDER_H
