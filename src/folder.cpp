/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

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
#include "articlelist.h"
#include "folder.h"
#include "fetchqueue.h"

#include <qlistview.h>
#include <qdom.h>
#include <qvaluelist.h>

#include <kdebug.h>

namespace Akregator {

class Folder::FolderPrivate
{
    public:
        /** List of children */
        QValueList<TreeNode*> children;
        /** caching unread count of children */
        int unread;
        /** whether or not the folder is expanded */
        bool open;
        /** update unread count cache */
};
           
Folder* Folder::fromOPML(QDomElement e)
{
    Folder* fg = new Folder(e.hasAttribute("text") ? e.attribute("text") : e.attribute("title"));
    fg->setOpen( e.attribute("isOpen") == "true" ? true : false );
    fg->setId( e.attribute("id").toUInt() );
    return fg;
}

Folder::Folder(const QString& title) : TreeNode(), d(new FolderPrivate)
{
    d->unread = 0;
    setTitle(title);
} 

Folder::~Folder()
{
    TreeNode* tmp = 0;
    for (QValueList<TreeNode*>::ConstIterator it = d->children.begin(); it != d->children.end(); ++it)
    {
        delete tmp;
        tmp = *it;
    }
    delete tmp;
    // tell the world that this node is destroyed
    emit signalDestroyed(this);
    delete d;
    d = 0;
}

ArticleList Folder::articles()
{
    ArticleList seq;
    QValueList<TreeNode*>::ConstIterator en = d->children.end();
    for (QValueList<TreeNode*>::ConstIterator it = d->children.begin(); it != en; ++it)
        seq += (*it)->articles();
     
    return seq;
}

QDomElement Folder::toOPML( QDomElement parent, QDomDocument document ) const
{
    QDomElement el = document.createElement( "outline" );
    el.setAttribute( "text", title() );
    parent.appendChild( el );
    el.setAttribute("isOpen", d->open ? "true" : "false");
    el.setAttribute( "id", QString::number(id()) );

    QValueList<TreeNode*>::ConstIterator en = d->children.end();
    for (QValueList<TreeNode*>::ConstIterator it = d->children.begin(); it != en; ++it)
        el.appendChild( (*it)->toOPML(el, document) );
        
    return el;
}

QValueList<TreeNode*> Folder::children()
{
    return d->children;
}

QValueList<TreeNode*> Folder::children() const
{
    return d->children;
}

void Folder::insertChild(TreeNode* node, TreeNode* after)
{
    int pos = d->children.findIndex(after);
    
    if (pos < 0)
        prependChild(node);
    else 
        insertChild(pos+1, node);
}

void Folder::insertChild(uint index, TreeNode* node)
{
//    kdDebug() << "enter Folder::insertChild(int, node) " << node->title() << endl;
    if (node)
    {
        if (index >= d->children.size())
            d->children.append(node);
        else
            d->children.insert(d->children.at(index), node);
        node->setParent(this);
        connect(node, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotChildChanged(TreeNode*)));
        connect(node, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotChildDestroyed(TreeNode*)));
        updateUnreadCount();    
        emit signalChildAdded(node);
        modified();
    }   
//    kdDebug() << "leave Folder::insertChild(int, node) " << node->title() << endl; 
}

void Folder::appendChild(TreeNode* node)
{
//    kdDebug() << "enter Folder::appendChild() " << node->title() << endl;
    if (node)
    {
        d->children.append(node);
        node->setParent(this);
        connect(node, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotChildChanged(TreeNode*)));
        connect(node, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotChildDestroyed(TreeNode*)));
        updateUnreadCount();    
        emit signalChildAdded(node);
        modified();
    }    
//    kdDebug() << "leave Folder::appendChild() " << node->title() << endl;
}

void Folder::prependChild(TreeNode* node)
{
//    kdDebug() << "enter Folder::prependChild() " << node->title() << endl;
    if (node)
    {
        d->children.prepend(node);
        node->setParent(this);
        connect(node, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotChildChanged(TreeNode*)));
        connect(node, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotChildDestroyed(TreeNode*)));
        updateUnreadCount();    
        emit signalChildAdded(node);
        modified();
    }    
//    kdDebug() << "leave Folder::prependChild() " << node->title() << endl;
}

void Folder::removeChild(TreeNode* node)
{
//    kdDebug() << "enter Folder::removeChild() node:" << (node ? node->title() : "null") << endl;
    if (node && d->children.contains(node))
    {    
        node->setParent(0);
        d->children.remove(node);
        updateUnreadCount();    
        emit signalChildRemoved(this, node);
        modified();
    }
//    kdDebug() << "leave Folder::removeChild() node: " << (node ? node->title() : "null") << endl;
}


TreeNode* Folder::firstChild()
{
    return d->children.isEmpty() ? 0 : d->children.first();
}            

TreeNode* Folder::lastChild()
{
    return d->children.isEmpty() ? 0 : d->children.last();
}
            
bool Folder::isOpen() const
{
    return d->open;
}

void Folder::setOpen(bool open)
{
    d->open = open;
}
            
int Folder::unread() const
{
    return d->unread;
}

int Folder::totalCount() const
{
    int totalCount = 0;

    QValueList<TreeNode*>::ConstIterator en = d->children.end();
    for (QValueList<TreeNode*>::ConstIterator it = d->children.begin(); it != en; ++it)
        totalCount += (*it)->totalCount();
    
    return totalCount;
}

void Folder::updateUnreadCount()
{
    int unread = 0;

    QValueList<TreeNode*>::ConstIterator en = d->children.end();
    for (QValueList<TreeNode*>::ConstIterator it = d->children.begin(); it != en; ++it)
        unread += (*it)->unread();
    
    d->unread = unread;
}

void Folder::slotMarkAllArticlesAsRead() 
{
    setNotificationMode(false);
    QValueList<TreeNode*>::ConstIterator en = d->children.end();
    for (QValueList<TreeNode*>::ConstIterator it = d->children.begin(); it != en; ++it)
        (*it)->slotMarkAllArticlesAsRead();
    setNotificationMode(true, true);
}
    
void Folder::slotChildChanged(TreeNode* /*node*/)
{
//    kdDebug() << "enter Folder::slotChildChanged child" << node->title() << endl;
    int oldUnread = d->unread;
    updateUnreadCount();    
   
    if (oldUnread != d->unread)
        modified();

//    kdDebug() << "leave Folder::slotChildChanged child"  << node->title() << endl;
}

void Folder::slotChildDestroyed(TreeNode* node)
{
    d->children.remove(node);
    updateUnreadCount();    
    modified();
}

void Folder::slotDeleteExpiredArticles()
{
    setNotificationMode(false);
    QValueList<TreeNode*>::ConstIterator en = d->children.end();
    for (QValueList<TreeNode*>::ConstIterator it = d->children.begin(); it != en; ++it)
        (*it)->slotDeleteExpiredArticles();
    setNotificationMode(true, true);
}

void Folder::slotAddToFetchQueue(FetchQueue* queue)
{
    QValueList<TreeNode*>::ConstIterator en = d->children.end();
    for (QValueList<TreeNode*>::ConstIterator it = d->children.begin(); it != en; ++it)
        (*it)->slotAddToFetchQueue(queue);
}

TreeNode* Folder::next()
{
    if ( firstChild() )
        return firstChild();

    if ( nextSibling() )
        return nextSibling();
    
    Folder* p = parent();
    while (p)
    {
        if ( p->nextSibling() )
            return p->nextSibling();
        else
            p = p->parent();
    }
    return 0;
}

} // namespace Akregator
#include "folder.moc"
