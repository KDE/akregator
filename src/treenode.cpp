/***************************************************************************
 *   Copyright (C) 2004 by Frank Osterfeld                                 *
 *   frank.osterfeld AT kdemail.net                                      *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "feedgroup.h"
#include "treenode.h"

#include "kdebug.h"

using namespace Akregator;

TreeNode::TreeNode()
    : QObject(0, 0), m_doNotify(true), m_changeOccured(false), m_title(""), m_parent(0), m_id(0)
{
}

TreeNode::~TreeNode()
{
    // tell the world that this node is destroyed
    //emit signalDestroyed(this);
}

const QString& TreeNode::title() const
{
    return m_title;
}

void TreeNode::setTitle(const QString& title)
{

    if (m_title != title)
    {
        m_title = title;
        modified();
    }
}

TreeNode* TreeNode::nextSibling() const
{
    if (!m_parent)
        return 0;
    QPtrList<TreeNode> children = m_parent->children();
    children.find(this);
    return children.next();
}

TreeNode* TreeNode::prevSibling() const
{
    if (!m_parent)
        return 0;
    QPtrList<TreeNode> children = m_parent->children();
    children.find(this);
    return children.prev();
}

FeedGroup* TreeNode::parent() const
{
    return m_parent;
}

void TreeNode::setParent(FeedGroup* parent)
{
    m_parent = parent;
}

void TreeNode::setNotificationMode(bool doNotify, bool notifyOccuredChanges)
{
    if (doNotify && !m_doNotify) // turned on
    {
        m_doNotify = true;
        if (m_changeOccured && notifyOccuredChanges)
            emit signalChanged(this);
        m_changeOccured = false;
    }
    if (!doNotify && m_doNotify) //turned off
    {
        m_changeOccured = false;
        m_doNotify = false;
    }
}

uint TreeNode::id() const
{
    return m_id;
}

void TreeNode::setId(uint id)
{
    m_id = id;
}

void TreeNode::modified()
{
//    kdDebug() << "enter TreeNode::modified" << title() << endl;
    if (m_doNotify)
        emit signalChanged(this);
    else
        m_changeOccured = true;
//    kdDebug() << "leave TreeNode::modified" << title()<< endl;
}

#include "treenode.moc"
