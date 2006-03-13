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

#include <k3listview.h>
#include <klocale.h>

#include <QGridLayout>
#include <QHash>
#include <QList>
#include <QWidget>

namespace Akregator
{

class SelectNodeDialog::SelectNodeDialogPrivate
{
    public:
    SimpleNodeSelector* widget;
};

SelectNodeDialog::SelectNodeDialog(FeedList* feedList, QWidget* parent, char* name) : 
 KDialogBase(parent, name, true, i18n("Select Feed or Folder"),
                  KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true), d(new SelectNodeDialogPrivate)
{
    d->widget = new SimpleNodeSelector(feedList, this);

    connect(d->widget, SIGNAL(signalNodeSelected(TreeNode*)), this, SLOT(slotNodeSelected(TreeNode*)));

    setMainWidget(d->widget);
    enableButtonOK(false);
}

SelectNodeDialog::~SelectNodeDialog()
{
    delete d;
    d = 0;
}

TreeNode* SelectNodeDialog::selectedNode() const
{
    return d->widget->selectedNode();
}

void SelectNodeDialog::slotSelectNode(TreeNode* node)
{
    d->widget->slotSelectNode(node);
}

void SelectNodeDialog::slotNodeSelected(TreeNode* node)
{
    enableButtonOK(node != 0);
}


class SimpleNodeSelector::SimpleNodeSelectorPrivate
{
    public:
    K3ListView* view;
    FeedList* list;
    NodeVisitor* visitor;
    QHash<TreeNode*,Q3ListViewItem*> nodeToItem;
    QHash<Q3ListViewItem*, TreeNode*> itemToNode;
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
        QList<TreeNode*> children = node->children();
        m_view->d->nodeToItem[node]->setExpandable(true);
        for (QList<TreeNode*>::ConstIterator it = children.begin(); it != children.end(); ++it)
             createItems(*it);
        return true;
    }

    virtual bool visitTreeNode(TreeNode* node)
    {
        Q3ListViewItem* pi = node->parent() ? m_view->d->nodeToItem[node->parent()] : 0;
         
        K3ListViewItem* item = 0;
        if (pi != 0)
             item = new K3ListViewItem(pi, node->title());
        else
             item = new K3ListViewItem(m_view->d->view, node->title());
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
    connect(feedList, SIGNAL(signalDestroyed(FeedList*)), this, SLOT(slotFeedListDestroyed(FeedList*)));

    d->view = new K3ListView(this);
    d->view->setRootIsDecorated(true);
    d->view->addColumn(i18n("Feeds"));
    
    connect(d->view, SIGNAL(selectionChanged(Q3ListViewItem*)), this, SLOT(slotItemSelected(Q3ListViewItem*)));

    QGridLayout* layout = new QGridLayout(this, 1, 1);
    layout->addWidget(d->view, 0, 0);

    d->visitor = new NodeVisitor(this);

    d->visitor->createItems(d->list->rootNode());
    d->nodeToItem[d->list->rootNode()]->setOpen(true);
    d->view->ensureItemVisible(d->nodeToItem[d->list->rootNode()]);
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
    Q3ListViewItem* item = d->nodeToItem[node];
    if (item != 0)
        d->view->setSelected(item, true);
}

void SimpleNodeSelector::slotFeedListDestroyed(FeedList* /*list*/)
{
    d->nodeToItem.clear();
    d->itemToNode.clear();
    d->view->clear();
}

void SimpleNodeSelector::slotItemSelected(Q3ListViewItem* item)
{
    emit signalNodeSelected(d->itemToNode[item]);
}

void SimpleNodeSelector::slotNodeDestroyed(TreeNode* node)
{
    if (d->nodeToItem.contains(node))
    {
        Q3ListViewItem* item = d->nodeToItem[node];
        d->nodeToItem.remove(node);
        d->itemToNode.remove(item);
        delete item;
    }
}

} // namespace Akregator

#include "simplenodeselector.moc"
	
