/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
#include "articlesequence.h" 
#include "feedgroup.h"
#include "fetchtransaction.h"

#include <qlistview.h>
#include <qdom.h>

#include <kdebug.h>

using namespace Akregator;

FeedGroup* FeedGroup::fromOPML(QDomElement e)
{
    FeedGroup* fg = new FeedGroup(e.hasAttribute("text") ? e.attribute("text") : e.attribute("title"));
    fg->setOpen( e.attribute("isOpen") == "true" ? true : false );
    return fg;
}

FeedGroup::FeedGroup(const QString& title) : TreeNode(), m_unread(0)
{
    setTitle(title);
} 

FeedGroup::~FeedGroup()
{
    for (TreeNode* i = m_children.first(); i; i = m_children.next() )
        delete i;
    emit signalDestroyed(this);
}

ArticleSequence FeedGroup::articles()
{
    ArticleSequence seq;
    for (TreeNode* i = m_children.first(); i; i = m_children.next() )
        seq += i->articles();
     
    return seq;
}

QDomElement FeedGroup::toOPML( QDomElement parent, QDomDocument document ) const
{
    QDomElement el = document.createElement( "outline" );
    el.setAttribute( "text", title() );
    parent.appendChild( el );
    el.setAttribute("isOpen", m_open ? "true" : "false");
    
    // necessary because of const
    QPtrList<TreeNode> children = m_children;
    
    for (TreeNode* i = children.first(); i; i = children.next() )
        el.appendChild( i->toOPML(el, document) );
        
    return el;
}

void FeedGroup::insertChild(TreeNode* node, TreeNode* after)
{
    int pos = m_children.find(after);
    
    if (pos == -1)
        prependChild(node);        
    else 
        insertChild(pos+1, node);
}

void FeedGroup::insertChild(uint index, TreeNode* node)
{
//    kdDebug() << "enter FeedGroup::insertChild(int, node) " << node->title() << endl;
    if (node)
    {    
        m_children.insert(index, node);
        node->setParent(this);
        connect(node, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotChildChanged(TreeNode*)));
        connect(node, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotChildDestroyed(TreeNode*)));
        updateUnreadCount();    
        emit signalChildAdded(this, node);
        modified();
    }   
//    kdDebug() << "leave FeedGroup::insertChild(int, node) " << node->title() << endl; 
}

void FeedGroup::appendChild(TreeNode* node)
{
//    kdDebug() << "enter FeedGroup::appendChild() " << node->title() << endl;
    if (node)
    {
        m_children.append(node);    
        node->setParent(this);
        connect(node, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotChildChanged(TreeNode*)));
        connect(node, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotChildDestroyed(TreeNode*)));
        updateUnreadCount();    
        emit signalChildAdded(this, node);
        modified();
    }    
//    kdDebug() << "leave FeedGroup::appendChild() " << node->title() << endl;
}

void FeedGroup::prependChild(TreeNode* node)
{
//    kdDebug() << "enter FeedGroup::prependChild() " << node->title() << endl;
    if (node)
    {
        m_children.prepend(node);    
        node->setParent(this);
        connect(node, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotChildChanged(TreeNode*)));
        connect(node, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotChildDestroyed(TreeNode*)));
        updateUnreadCount();    
        emit signalChildAdded(this, node);
        modified();
    }    
//    kdDebug() << "leave FeedGroup::prependChild() " << node->title() << endl;
}

void FeedGroup::removeChild(TreeNode* node)
{
//    kdDebug() << "enter FeedGroup::removeChild() node:" << (node ? node->title() : "null") << endl;
    if (node && m_children.contains(node))
    {    
        node->setParent(0);
        m_children.remove(node);
        updateUnreadCount();    
        emit signalChildRemoved(this, node);
        modified();
    }
//    kdDebug() << "leave FeedGroup::removeChild() node: " << (node ? node->title() : "null") << endl;
}


TreeNode* FeedGroup::firstChild()
{
    return m_children.first();
}            

TreeNode* FeedGroup::lastChild()
{
    return m_children.last();
}
            
bool FeedGroup::isOpen() const
{
    return m_open;
}

void FeedGroup::setOpen(bool open)
{
    m_open = open;
}
            
int FeedGroup::unread() const
{
    return m_unread;
}

void FeedGroup::updateUnreadCount()
{
    int unread = 0;
  
    QPtrList<TreeNode> children = m_children;
    
    for (TreeNode* i = children.first(); i; i = children.next() )
        unread += i->unread();
    
    m_unread = unread;
}

void FeedGroup::slotMarkAllArticlesAsRead() 
{
    setNotificationMode(false);
    for (TreeNode* i = m_children.first(); i; i = m_children.next() )
        i->slotMarkAllArticlesAsRead();
    setNotificationMode(true, true);
}
    
void FeedGroup::slotChildChanged(TreeNode* /*node*/)
{
//    kdDebug() << "enter FeedGroup::slotChildChanged child" << node->title() << endl;
    int oldUnread = m_unread;
    updateUnreadCount();    
   
    if (oldUnread != m_unread)
        modified();

//    kdDebug() << "leave FeedGroup::slotChildChanged child"  << node->title() << endl;
}

void FeedGroup::slotChildDestroyed(TreeNode* node)
{
    m_children.remove(node);
    updateUnreadCount();    
    modified();
}

void FeedGroup::slotDeleteExpiredArticles()
{
    setNotificationMode(false);
    for (TreeNode* i = m_children.first(); i; i = m_children.next() )
        i->slotDeleteExpiredArticles();
    setNotificationMode(true, true);
}

void FeedGroup::slotAddToFetchTransaction(FetchTransaction* transaction)
{
    for (TreeNode* i = m_children.first(); i; i = m_children.next() )
        i->slotAddToFetchTransaction(transaction);
}

TreeNode* FeedGroup::next()
{
    if ( firstChild() )
        return firstChild();

    if ( nextSibling() )
        return nextSibling();
    
    FeedGroup* p = parent();
    while (p)
    {
        if ( p->nextSibling() )
            return p->nextSibling();
        else
            p = p->parent();
    }
    return 0;
}

#include "feedgroup.moc"
