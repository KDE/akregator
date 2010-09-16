/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
                  2004-2005 Frank Osterfeld <osterfeld@kde.org>

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

#ifndef AKREGATOR_FOLDER_H
#define AKREGATOR_FOLDER_H

#include "akregator_export.h"
#include "treenode.h"

class QDomDocument;
class QDomElement;
template <class T> class QList;

namespace Akregator {

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
        static Folder* fromOPML(const QDomElement& e);

        /** Creates a new folder with a given title
        @param title The title of the feed group
            */
        explicit Folder(const QString& title = QString());

        ~Folder();

        bool accept(TreeNodeVisitor* visitor);

        /** returns the number of unread articles in all children
        @return number of unread articles */
        int unread() const;

        /** returns the number of articles in all children
        @return number of articles */
        int totalCount() const;

        /** Helps the rest of the app to decide if node should be handled as group or not. */
        bool isGroup() const { return true; }

        //impl
        bool isAggregation() const { return true; }

        /** converts the feed group into OPML format for save and export and appends it to node @c parent in document @document.
        Children are processed and appended recursively.
        @param parent The parent element
        @param document The DOM document
        @return The newly created element representing this feed group */
        QDomElement toOPML( QDomElement parent, QDomDocument document ) const;

        /** returns the (direct) children of this node.
        @return a list of pointers to the child nodes
            */

        //reimpl
        QList<const TreeNode*> children() const;

        QList<const TreeNode*> namedChildren( const QString& title ) const;
        QList<TreeNode*> namedChildren( const QString& title );

        //reimpl
        QList<TreeNode*> children();

        QVector<const Feed*> feeds() const;
        QVector<Feed*> feeds();
        QVector<const Folder*> folders() const;
        QVector<Folder*> folders();

        int indexOf( const TreeNode* node ) const;

        //reimpl
        TreeNode* childAt( int pos );

        //reimpl
        const TreeNode* childAt( int pos ) const;



        /** inserts @c node as child after child node @c after.
        if @c after is not a child of this group, @c node will be inserted as first child
        @param node the tree node to insert
        @param after the node after which @c node will be inserted */
        void insertChild(TreeNode* node, TreeNode* after);

        /** inserts @c node as first child
        @param node the tree node to insert */
        void prependChild(TreeNode* node);

        /** inserts @c node as last child
        @param node the tree node to insert */
        void appendChild(TreeNode* node);

        /** remove @c node from children. Note that @c node will not be deleted
        @param node the child node to remove  */
        void removeChild(TreeNode* node);

        /** returns the first child of the group, 0 if none exist */
        TreeNode* firstChild();
        const TreeNode* firstChild() const;


        /** returns the last child of the group, 0 if none exist */
        TreeNode* lastChild();
        const TreeNode* lastChild() const;


        /** returns whether the feed group is opened or not..
        Use only in \ref FolderItem. */
        bool isOpen() const;

        /** open/close the feed group (display it as expanded/collapsed in the tree view). Use only in \ref FolderItem. */
        void setOpen(bool open);

        /** returns the next node in the tree.
        Calling next() unless it returns 0 iterates through the tree in pre-order
            */
        TreeNode* next();
        const TreeNode* next() const;

        QIcon icon() const;

        /* returns whether @p node is among the direct or indirect children of this
         * folder
         */
        bool subtreeContains( const Akregator::TreeNode* node ) const;

        /* reimp */ KJob* createMarkAsReadJob();

    signals:
        /** emitted when a child was added */
        void signalChildAdded(Akregator::TreeNode*);

        /** emitted when a child was removed */
        void signalChildRemoved(Akregator::Folder*, Akregator::TreeNode*);

        void signalAboutToRemoveChild( Akregator::TreeNode* );

    public slots:

        /** Called when a child was modified.
        @param node the child that was changed
            */
        void slotChildChanged(Akregator::TreeNode* node);

        /** Called when a child was destroyed.
        @param node the child that was destroyed
        */
        void slotChildDestroyed(Akregator::TreeNode* node);

        /** enqueues children recursively for fetching
        @param queue a fetch queue */
        void slotAddToFetchQueue(Akregator::FetchQueue* queue, bool intervalFetchesOnly=false);

    protected:

        /** inserts @c node as child on position @c index
        @param index the position where to insert
        @param node the tree node to insert */
        void insertChild(int index, TreeNode* node);

        void doArticleNotification();

    private:
        QList<Article> articles();

        void connectToNode(TreeNode* child);
        void disconnectFromNode(TreeNode* child);

        void updateUnreadCount() const;

        class FolderPrivate;
        FolderPrivate* d;
};

} // namespace Akregator

#endif // AKREGATOR_FOLDER_H
