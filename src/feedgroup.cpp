/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
#include "articlesequence.h" 
#include "feedgroup.h"
#include "feedscollection.h"
#include <qlistview.h>
#include <qdom.h>

#include <kdebug.h>

using namespace Akregator;

FeedGroup::FeedGroup(QListViewItem *i, FeedsCollection *coll)
    : TreeNode(i, coll)
{
}

FeedGroup::~FeedGroup()
{
    emit signalDestroyed();
}

ArticleSequence FeedGroup::articles()
{
    ArticleSequence seq;
    for (QListViewItem* i = m_item->firstChild(); i; i = i->nextSibling() )
    {
        TreeNode* child = static_cast<TreeNode*> (m_collection->find(i));
        seq += child->articles();
    }    
     return seq;
     // iterate through m_children later
}

QDomElement FeedGroup::toOPML( QDomElement parent, QDomDocument document ) const
{
    // TODO: iterate through m_children instead of using the collection
    QDomElement el = document.createElement( "outline" );
    el.setAttribute( "text", title() );
    parent.appendChild( el );
    el.setAttribute("isOpen", m_item->isOpen() ? "true" : "false");

    for (QListViewItem* i = m_item->firstChild(); i; i = i->nextSibling() )
    {
        TreeNode* child = static_cast<TreeNode*> (m_collection->find(i));
        el.appendChild( child->toOPML(el, document) );
    }
    
    return el;
    
    }

int FeedGroup::unread() const
{
    // TODO: iterate through m_children instead of using the collection
    int unread = 0;
    for (QListViewItem* i = m_item->firstChild(); i; i = i->nextSibling() )
    {
        TreeNode* child = static_cast<TreeNode*> (m_collection->find(i));
        unread += child->unread();
    }
    return unread;
}

void FeedGroup::slotMarkAllArticlesAsRead() 
{
    // TODO: iterate through m_children instead of using the collection
    for (QListViewItem* i = m_item->firstChild(); i; i = i->nextSibling() )
    {
        TreeNode* child = static_cast<TreeNode*> (m_collection->find(i));
        child->slotMarkAllArticlesAsRead();
    }
    
}    
void FeedGroup::slotChildChanged()
{
    // calculate m_unread etc. later
    emit signalChanged();
}

void FeedGroup::slotDeleteExpiredArticles()
{
    // TODO: iterate through m_children instead of using the collection
    for (QListViewItem* i = m_item->firstChild(); i; i = i->nextSibling() )
    {
        TreeNode* child = static_cast<TreeNode*> (m_collection->find(i));
        child->slotDeleteExpiredArticles();
    }
    
    // iterate through m_children later    
}

#include "feedgroup.moc"
