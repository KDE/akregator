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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef AKREGATOR_NODELIST_H
#define AKREGATOR_NODELIST_H

#include <qobject.h>
//Added by qt3to4:
#include <Q3ValueList>

class QDomDocument;
class QDomNode;
class QString;

template <class K,class T> class QMap;
template <class T> class Q3ValueList;

namespace Akregator {

class Folder;
class TreeNode;

class NodeList : public QObject
{

Q_OBJECT

public:
    NodeList(QObject *parent=0, const char *name=0);
    virtual ~NodeList();

    virtual Folder* rootNode() const;

    virtual bool readFromXML(const QDomDocument& doc) = 0;

    virtual QDomDocument toXML() const = 0;

    virtual bool isEmpty() const;

    TreeNode* findByID(int id) const;

    /** returns the title of the feed list (as used in the OPML document) */
    const QString& title() const;

    /** sets the title of the feed list */
    void setTitle(const QString& name);

    /** returns a flat list containing all nodes in the tree */
    const Q3ValueList<TreeNode*>& asFlatList() const;

signals:
    void signalDestroyed(NodeList*);
    /** emitted when a node was added to the list */
    void signalNodeAdded(TreeNode*);
    /** emitted when a node was removed from the list */
    void signalNodeRemoved(TreeNode*);

public slots:

    /**
     * Clears the list without touching the root node.
     */
    void clear();

protected:

    Q3ValueList<TreeNode*>* flatList() const;
    QMap<int, TreeNode*>* idMap() const;
    
    int generateID();
    void setRootNode(Folder* folder);
    virtual void addNode(TreeNode* node, bool preserveID);
    virtual void removeNode(TreeNode* node);
  
protected slots:

    virtual void slotNodeDestroyed(TreeNode* node);
    virtual void slotNodeAdded(TreeNode* node);
    virtual void slotNodeRemoved(Folder* parent, TreeNode* node);

private:
    NodeList(const NodeList&) : QObject() {}
    NodeList& operator=(const NodeList&) { return *this; }

    friend class AddNodeVisitor;
    class AddNodeVisitor;

    friend class RemoveNodeVisitor;
    class RemoveNodeVisitor;

    class NodeListPrivate;
    NodeListPrivate* d;
    
};

} // namespace Akregator

#endif // AKREGATOR_NODELIST_H
