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
