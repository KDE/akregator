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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "folder.h"
#include "treenode.h"

#include <QString>
#include <QList>

#include <kdebug.h>

namespace Akregator {

class TreeNode::TreeNodePrivate
{
    public:
    
    bool doNotify;
    bool nodeChangeOccurred;
    bool articleChangeOccurred;
    QString title;
    Folder* parent;
    uint id;
    bool signalDestroyedEmitted;
};

TreeNode::TreeNode()
    : QObject(0), d(new TreeNodePrivate)
{
    d->doNotify = true;
    d->nodeChangeOccurred = false;
    d->articleChangeOccurred = false;
    d->title = "";
    d->parent = 0;
    d->id = 0;
    d->signalDestroyedEmitted = false;
    
}

void TreeNode::emitSignalDestroyed()
{
    if (!d->signalDestroyedEmitted)
    {
        emit signalDestroyed(this);
        d->signalDestroyedEmitted = true;
    } 
}

TreeNode::~TreeNode()
{

    delete d;
    d = 0;
}

QString TreeNode::title() const
{
    return d->title;
}

void TreeNode::setTitle(const QString& title)
{

    if (d->title != title)
    {
        d->title = title;
        nodeModified();
    }
}

TreeNode* TreeNode::nextSibling() const
{
    if (!d->parent)
        return 0;
    QList<TreeNode*> children = d->parent->children();
    TreeNode* me = (TreeNode*)this;
        
    int idx = children.indexOf(me);
    
    return (idx+1 < children.size()) ? children.at(idx+1) : 0L;
}

TreeNode* TreeNode::prevSibling() const
{
    if (!d->parent)
        return 0;
    QList<TreeNode*> children = d->parent->children();
    TreeNode* me = (TreeNode*)this;
    
    int idx = children.indexOf(me);
    return (idx > 0) ? children.at(idx-1) : 0L;
}

Folder* TreeNode::parent() const
{
    return d->parent;
}

QList<TreeNode*> TreeNode::children() const
{
    return QList<TreeNode*>();
}


void TreeNode::setParent(Folder* parent)
{
    d->parent = parent;
}

void TreeNode::setNotificationMode( bool doNotify )
{
    if ( doNotify && !d->doNotify ) // turned on
    {
        d->doNotify = true;
        if ( d->nodeChangeOccurred )
            emit signalChanged( this );
        if ( d->articleChangeOccurred )
            doArticleNotification();
        d->nodeChangeOccurred = false;
        d->articleChangeOccurred = false;
    }
    else if ( !doNotify && d->doNotify ) //turned off
    {
        d->nodeChangeOccurred = false;
        d->articleChangeOccurred = false;
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

void TreeNode::nodeModified()
{
    if (d->doNotify)
        emit signalChanged(this);
    else
        d->nodeChangeOccurred = true;
}

void TreeNode::articlesModified()
{
    if (d->doNotify)
        doArticleNotification();
    else
        d->articleChangeOccurred = true;
}

void TreeNode::doArticleNotification()
{
}

} // namespace Akregator

#include "treenode.moc"
