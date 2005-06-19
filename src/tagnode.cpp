/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

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

#include "article.h"
#include "articlefilter.h"
#include "fetchqueue.h"
#include "folder.h"
#include "tag.h"
#include "tagnode.h"
#include "treenode.h"
#include "treenodevisitor.h"

#include <qdom.h>
#include <qstring.h>
#include <qvaluelist.h>

namespace Akregator {

class TagNode::TagNodePrivate
{
    public:
    TagFilter filter;
    TreeNode* observed;
    int unread;
    Tag tag;
    QValueList<Article> articles;
    QValueList<Article> addedArticlesNotify;
    QValueList<Article> removedArticlesNotify;
    QValueList<Article> updatedArticlesNotify;
};

TagNode::TagNode(const Tag& tag, TreeNode* observed) : d(new TagNodePrivate)
{
    d->tag = tag;
    d->filter = TagFilter(tag);
    setTitle(tag.name());
    d->observed = observed;
    d->unread = 0;
    
    connect(observed, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotObservedDestroyed(TreeNode*)));
    connect(observed, SIGNAL(signalArticlesAdded(TreeNode*, const QValueList<Article>&)), this, SLOT(slotArticlesAdded(TreeNode*, const QValueList<Article>&)) );
    connect(observed, SIGNAL(signalArticlesUpdated(TreeNode*, const QValueList<Article>&)), this, SLOT(slotArticlesUpdated(TreeNode*, const QValueList<Article>&)) );
    connect(observed, SIGNAL(signalArticlesRemoved(TreeNode*, const QValueList<Article>&)), this, SLOT(slotArticlesRemoved(TreeNode*, const QValueList<Article>&)) );

    d->articles = observed->articles(tag.id());
    // TODO: calc unread count
}


Tag TagNode::tag() const
{
    return d->tag;
}

TagNode::~TagNode()
{
    emitSignalDestroyed();
    delete d;
    d = 0;
}

bool TagNode::accept(TreeNodeVisitor* visitor)
{
    if (visitor->visitTagNode(this))
        return true;
    else
        return visitor->visitTreeNode(this);
}

int TagNode::unread() const
{
    return d->unread;
}


int TagNode::totalCount() const
{
    return d->articles.count();
}

    
QValueList<Article> TagNode::articles(const QString& tag)
{
    return d->articles;
}

QStringList TagNode::tags() const
{
   // TODO
   return QStringList();
}

QDomElement TagNode::toOPML( QDomElement parent, QDomDocument document ) const
{
    return QDomElement();
}    

TreeNode* TagNode::next()
{
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

void TagNode::slotDeleteExpiredArticles() 
{ 
// not our business
}
    
void TagNode::slotMarkAllArticlesAsRead()
{ 
    setNotificationMode(false);
    QValueList<Article>::Iterator en = d->articles.end();
    for (QValueList<Article>::Iterator it = d->articles.begin(); it != en; ++it)
        (*it).setStatus(Article::Read);
    setNotificationMode(true);
}
    
void TagNode::slotAddToFetchQueue(FetchQueue* /*queue*/, bool /*intervalFetchOnly*/)
{
// not our business
}

void TagNode::doArticleNotification()
{
    if (!d->addedArticlesNotify.isEmpty())
    {
        emit signalArticlesAdded(this, d->addedArticlesNotify);
        d->addedArticlesNotify.clear();
    }
    if (!d->updatedArticlesNotify.isEmpty())
    {
        emit signalArticlesUpdated(this, d->updatedArticlesNotify);
        d->updatedArticlesNotify.clear();
    }
    if (!d->removedArticlesNotify.isEmpty())
    {
        emit signalArticlesRemoved(this, d->removedArticlesNotify);
        d->removedArticlesNotify.clear();
    }
    TreeNode::doArticleNotification();
}

void TagNode::slotArticlesAdded(TreeNode* node, const QValueList<Article>& list)
{
    bool added = false;
    for (QValueList<Article>::ConstIterator it = list.begin(); it != list.end(); ++it)
    {
        if (!d->articles.contains(*it) && d->filter.matches(*it))
        {
            d->articles.append(*it);
            d->addedArticlesNotify.append(*it);
            added = true;
        }
    }

    if (added)
        articlesModified();
}

void TagNode::slotArticlesUpdated(TreeNode* node, const QValueList<Article>& list)
{
    bool updated = false;
    for (QValueList<Article>::ConstIterator it = list.begin(); it != list.end(); ++it)
    {
        if (d->articles.contains(*it))
        {
            if (!d->filter.matches(*it)) // articles is in list, but doesn't match our criteria anymore -> remove it
	    {
                d->articles.remove(*it);
                d->removedArticlesNotify.append(*it);
                updated = true;
            }
            else // otherwise the article remains in the list and we just forward the update
            {
                d->updatedArticlesNotify.append(*it);
                updated = true;
            }
        }
        else // article not in list
        { 
            if (d->filter.matches(*it)) // articles is not in list, but matches our criteria -> add it
            {
                d->articles.append(*it);
                d->addedArticlesNotify.append(*it);
                updated = true;
            }
        }
    }
    if (updated)
        articlesModified();
}

void TagNode::slotArticlesRemoved(TreeNode* node, const QValueList<Article>& list)
{
    bool removed = false;
    for (QValueList<Article>::ConstIterator it = list.begin(); it != list.end(); ++it)
    {
        if (d->articles.contains(*it))
        {
            d->articles.remove(*it);
            d->removedArticlesNotify.append(*it);
            removed = true;
        }
    }
    if (removed)
        articlesModified();
}

void TagNode::slotObservedDestroyed(TreeNode* /*observed*/)
{
    d->removedArticlesNotify = d->articles;
    d->articles.clear();
    articlesModified();
}

} // namespace Akregator

#include "tagnode.moc"
