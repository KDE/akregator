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

#include "treenode.h"
#include "folder.h"
#include "articlejobs.h"
#include "article.h"

#include <QPoint>
#include <QString>
#include <QList>

#include "akregator_debug.h"

#include <cassert>

using namespace Akregator;


TreeNode::TreeNode()
    : QObject(nullptr)
{
}

void TreeNode::emitSignalDestroyed()
{
    if (!m_signalDestroyedEmitted) {
        if (parent()) {
            parent()->removeChild(this);
        }
        Q_EMIT signalDestroyed(this);
        m_signalDestroyedEmitted = true;
    }
}

TreeNode::~TreeNode()
{
    Q_ASSERT(m_signalDestroyedEmitted || !"TreeNode subclasses must call emitSignalDestroyed in their destructor");
}

QString TreeNode::title() const
{
    return m_title;
}

void TreeNode::setTitle(const QString &title)
{
    if (m_title != title) {
        m_title = title;
        nodeModified();
    }
}

TreeNode *TreeNode::nextSibling()
{
    if (!m_parent) {
        return nullptr;
    }
    const QList<TreeNode *> children = parent()->children();
    const int idx = children.indexOf(this);

    return (idx + 1 < children.size()) ? children.at(idx + 1) : nullptr;
}

const TreeNode *TreeNode::nextSibling() const
{
    if (!m_parent) {
        return nullptr;
    }
    const QList<const TreeNode *> children = parent()->children();
    const int idx = children.indexOf(this);

    return (idx + 1 < children.size()) ? children.at(idx + 1) : nullptr;
}

TreeNode *TreeNode::prevSibling()
{
    if (!m_parent) {
        return nullptr;
    }
    const QList<TreeNode *> children = parent()->children();

    const int idx = children.indexOf(this);
    return (idx > 0) ? children.at(idx - 1) : nullptr;
}

const TreeNode *TreeNode::prevSibling() const
{
    if (!m_parent) {
        return nullptr;
    }
    const QList<const TreeNode *> children = parent()->children();
    const int idx = children.indexOf(this);
    return (idx > 0) ? children.at(idx - 1) : nullptr;
}

const Folder *TreeNode::parent() const
{
    return m_parent;
}

Folder *TreeNode::parent()
{
    return m_parent;
}

QList<const TreeNode *> TreeNode::children() const
{
    return QList<const TreeNode *>();
}

QList<TreeNode *> TreeNode::children()
{
    return QList<TreeNode *>();
}

const TreeNode *TreeNode::childAt(int pos) const
{
    Q_UNUSED(pos)
    return nullptr;
}

TreeNode *TreeNode::childAt(int pos)
{
    Q_UNUSED(pos)
    return nullptr;
}

void TreeNode::setParent(Folder *parent)
{
    m_parent = parent;
}

void TreeNode::setNotificationMode(bool doNotify)
{
    if (doNotify && !m_doNotify) { // turned on
        m_doNotify = true;
        if (m_nodeChangeOccurred) {
            Q_EMIT signalChanged(this);
        }
        if (m_articleChangeOccurred) {
            doArticleNotification();
        }
        m_nodeChangeOccurred = false;
        m_articleChangeOccurred = false;
    } else if (!doNotify && m_doNotify) { //turned off
        m_nodeChangeOccurred = false;
        m_articleChangeOccurred = false;
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

void TreeNode::nodeModified()
{
    if (m_doNotify) {
        Q_EMIT signalChanged(this);
    } else {
        m_nodeChangeOccurred = true;
    }
}

void TreeNode::articlesModified()
{
    if (m_doNotify) {
        doArticleNotification();
    } else {
        m_articleChangeOccurred = true;
    }
}

void TreeNode::doArticleNotification()
{
}

QPoint TreeNode::listViewScrollBarPositions() const
{
    return m_scrollBarPositions;
}

void TreeNode::setListViewScrollBarPositions(const QPoint &pos)
{
    m_scrollBarPositions = pos;
}

ArticleListJob *TreeNode::createListJob()
{
    return new ArticleListJob(this);
}
