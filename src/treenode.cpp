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

#include <qstring.h>

#include <kdebug.h>

namespace Akregator {

class TreeNode::TreeNodePrivate
{
    public:
    
    bool doNotify;
    bool changeOccured;
    QString title;
    Folder* parent;
    uint id;
};

TreeNode::TreeNode()
    : QObject(0, 0), d(new TreeNodePrivate)
{
    d->doNotify = true;
    d->changeOccured = false;
    d->title = "";
    d->parent = 0;
    d->id = 0;
}

TreeNode::~TreeNode()
{
    // tell the world that this node is destroyed
    //emit signalDestroyed(this);
    delete d;
    d = 0;
}

const QString& TreeNode::title() const
{
    return d->title;
}

void TreeNode::setTitle(const QString& title)
{

    if (d->title != title)
    {
        d->title = title;
        modified();
    }
}

TreeNode* TreeNode::nextSibling() const
{
    if (!d->parent)
        return 0;
    QPtrList<TreeNode> children = d->parent->children();
    children.find(this);
    return children.next();
}

TreeNode* TreeNode::prevSibling() const
{
    if (!d->parent)
        return 0;
    QPtrList<TreeNode> children = d->parent->children();
    children.find(this);
    return children.prev();
}

Folder* TreeNode::parent() const
{
    return d->parent;
}

void TreeNode::setParent(Folder* parent)
{
    d->parent = parent;
}

void TreeNode::setNotificationMode(bool doNotify, bool notifyOccuredChanges)
{
    if (doNotify && !d->doNotify) // turned on
    {
        d->doNotify = true;
        if (d->changeOccured && notifyOccuredChanges)
            emit signalChanged(this);
        d->changeOccured = false;
    }
    if (!doNotify && d->doNotify) //turned off
    {
        d->changeOccured = false;
        d->doNotify = false;
    }
}

uint TreeNode::id() const
{
    return d->id;
}

void TreeNode::setId(uint id)
{
    d->id = id;
}

void TreeNode::modified()
{
//    kdDebug() << "enter TreeNode::modified" << title() << endl;
    if (d->doNotify)
        emit signalChanged(this);
    else
        d->changeOccured = true;
//    kdDebug() << "leave TreeNode::modified" << title()<< endl;
}

} // namespace Akregator

#include "treenode.moc"
