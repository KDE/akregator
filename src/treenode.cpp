//
// C++ Implementation: %{MODULE}
//
// Description:
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "treenode.h"

using namespace Akregator;

TreeNode::TreeNode(TreeNode* parent)
    : QObject(0, 0), m_title("TreeNode"), m_parent(parent)
{
}

TreeNode::~TreeNode()
{
    if (m_collection)
        m_collection->remove(m_item);

    // tell the world that this item is destroyed
    emit signalDestroyed();
}

QString TreeNode::title() const
{
    return m_title;
}

void TreeNode::setTitle(QString title)
{
    m_title = title;
}

TreeNode* TreeNode::parent() const
{
    return m_parent;
}
    
void TreeNode::setParent(TreeNode* parent)
{
    m_parent = parent;
}    

#include "treenode.moc"
