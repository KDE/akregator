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
    QDomElement el = document.createElement( "outline" );
    el.setAttribute( "text", title() );
    parent.appendChild( el );
    return el;
    
    // iterate through m_children later
}

int FeedGroup::unread() const
{
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
    for (QListViewItem* i = m_item->firstChild(); i; i = i->nextSibling() )
    {
        TreeNode* child = static_cast<TreeNode*> (m_collection->find(i));
        child->slotMarkAllArticlesAsRead();
    }
}    

void FeedGroup::slotDeleteExpiredArticles()
{
    for (QListViewItem* i = m_item->firstChild(); i; i = i->nextSibling() )
    {
        TreeNode* child = static_cast<TreeNode*> (m_collection->find(i));
        child->slotDeleteExpiredArticles();
    }
    
    // iterate through m_children later    
}

#include "feedgroup.moc"
