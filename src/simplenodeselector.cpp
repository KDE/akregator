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

#include "feedlist.h"
#include "folder.h"
#include "simplenodeselector.h"
#include "treenode.h"
#include "treenodevisitor.h"

#include <klistview.h>

#include <qlayout.h>
#include <qmap.h>
#include <qwidget.h>

namespace Akregator
{

class SimpleNodeSelector::SimpleNodeSelectorPrivate
{
    public:
    KListView* view;
    FeedList* list;
    NodeVisitor* visitor;
    QMap<TreeNode*,QListViewItem*> nodeToItem;
    QMap<QListViewItem*, TreeNode*> itemToNode;
};

class SimpleNodeSelector::NodeVisitor : public TreeNodeVisitor
{
    public:

    NodeVisitor(SimpleNodeSelector* view) : TreeNodeVisitor(), m_view(view) {}

    void createItems(TreeNode* node)
    {
        node->accept(this);
    }

    virtual bool visitFolder(Folder* node)
    {
        visitTreeNode(node);
        QValueList<TreeNode*> children = node->children();
        m_view->d->nodeToItem[node]->setExpandable(true);
        for (QValueList<TreeNode*>::ConstIterator it = children.begin(); it != children.end(); ++it)
             createItems(*it);
        return true;
    }

    virtual bool visitTreeNode(TreeNode* node)
    {
        QListViewItem* pi = node->parent() ? m_view->d->nodeToItem[node->parent()] : 0;
         
        KListViewItem* item = 0;
        if (pi != 0)
             item = new KListViewItem(pi, node->title());
        else
             item = new KListViewItem(m_view->d->view, node->title());
        item->setExpandable(false);
        m_view->d->nodeToItem.insert(node, item);
        m_view->d->itemToNode.insert(item, node);
        connect(node, SIGNAL(signalDestroyed(TreeNode*)), m_view, SLOT(slotNodeDestroyed(TreeNode*)));
        return true;
    }

    private:

    SimpleNodeSelector* m_view;
};


SimpleNodeSelector::SimpleNodeSelector(FeedList* feedList, QWidget* parent, const char* name) : QWidget(parent, name), d(new SimpleNodeSelectorPrivate)
{
    d->list = feedList;
    connect(feedList, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotFeedListDestroyed(FeedList*)));
    d->view = new KListView(this);

    QGridLayout* layout = new QGridLayout(this, 1, 1);
    layout->addWidget(d->view, 1, 1);

    d->visitor = new NodeVisitor(this);

    d->visitor->createItems(d->list->rootNode());
}

SimpleNodeSelector::~SimpleNodeSelector()
{
    delete d->visitor;
    delete d;
    d = 0;
}

TreeNode* SimpleNodeSelector::selectedNode() const
{
    return d->itemToNode[d->view->selectedItem()];
}

void SimpleNodeSelector::slotSelectNode(TreeNode* node)
{
    QListViewItem* item = d->nodeToItem[node];
    if (item != 0)
        d->view->setSelected(item, true);
}

void SimpleNodeSelector::slotFeedListDestroyed(FeedList* /*list*/)
{
    d->nodeToItem.clear();
    d->itemToNode.clear();
    d->view->clear();
}

void SimpleNodeSelector::slotItemSelected(QListViewItem* item)
{
    emit signalNodeSelected(d->itemToNode[item]);
}

void SimpleNodeSelector::slotNodeDestroyed(TreeNode* node)
{
    if (d->nodeToItem.contains(node))
    {
        QListViewItem* item = d->nodeToItem[node];
        d->nodeToItem.remove(node);
        d->itemToNode.remove(item);
        delete item;
    }
}

} // namespace Akregator

#include "simplenodeselector.moc"
	
