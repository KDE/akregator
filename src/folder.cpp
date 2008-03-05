/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
                  2004-2005 Frank Osterfeld <osterfeld@kde.org>

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
#include "article.h"
#include "feed.h"
#include "fetchqueue.h"
#include "treenodevisitor.h"

#include <qdom.h>
#include <QList>

#include <KIcon>
#include <kdebug.h> 

#include <cassert>

using namespace Akregator;

class Folder::FolderPrivate
{
        Folder* const q;
    public:
        explicit FolderPrivate( Folder* qq );
        ~FolderPrivate();
        
        /** List of children */
        QList<TreeNode*> children;
        /** caching unread count of children */
        mutable int unread;
        /** whether or not the folder is expanded */
        bool open;

        /** caches guids for notifying added articles */
        QList<Article> addedArticlesNotify;
        /** caches guids for notifying removed articles */
        QList<Article> removedArticlesNotify;
};
  
Folder::FolderPrivate::FolderPrivate( Folder* qq ) : q( qq ), unread( 0 ), open( false )
{
}

Folder::FolderPrivate::~FolderPrivate()
{
    qDeleteAll( children );
    emit q->emitSignalDestroyed();
}

bool Folder::accept(TreeNodeVisitor* visitor)
{
    if (visitor->visitFolder(this))
        return true;
    else
        return visitor->visitTreeNode(this);
}

Folder* Folder::fromOPML(const QDomElement& e)
{
    Folder* fg = new Folder(e.hasAttribute(QString::fromLatin1("text")) ? e.attribute(QString::fromLatin1("text")) : e.attribute(QString::fromLatin1("title")));
    fg->setOpen( e.attribute(QString::fromLatin1("isOpen")) == QString::fromLatin1(("true")));
    fg->setId( e.attribute(QString::fromLatin1("id")).toUInt() );
    return fg;
}

Folder::Folder( const QString& title ) : TreeNode(), d( new FolderPrivate( this ) )
{
    setTitle( title );
} 

Folder::~Folder()
{
    delete d;
    d = 0;
}

QList<Article> Folder::articles()
{
    QList<Article> seq;
    Q_FOREACH( Feed* const i, feeds() )
        seq += i->articles();
    return seq;
}

QDomElement Folder::toOPML( QDomElement parent, QDomDocument document ) const
{
    QDomElement el = document.createElement( "outline" );
    el.setAttribute( "text", title() );
    parent.appendChild( el );
    el.setAttribute("isOpen", d->open ? "true" : "false");
    el.setAttribute( "id", QString::number(id()) );

    Q_FOREACH ( const Akregator::TreeNode* i, d->children )
        el.appendChild( i->toOPML(el, document) );
    return el;
}

        
QList<const TreeNode*> Folder::children() const
{
    QList<const TreeNode*> children;
    Q_FOREACH( const TreeNode* i, d->children )
        children.append( i );
    return children;
}
        
QList<TreeNode*> Folder::children()
{
    return d->children;
}


QVector<const Feed*> Folder::feeds() const
{
    QHash<int, const Feed*> feedsById;
    Q_FOREACH( const TreeNode* i, d->children )
        Q_FOREACH ( const Feed* j, i->feeds() )
            feedsById.insert( j->id(), j );
    return feedsById.values().toVector();
}

QVector<Feed*> Folder::feeds()
{
    QHash<int, Feed*> feedsById;
    Q_FOREACH( TreeNode* i, d->children )
        Q_FOREACH ( Feed* j, i->feeds() )
            feedsById.insert( j->id(), j );
    return feedsById.values().toVector();    
}

int Folder::indexOf( const TreeNode* node ) const
{
    return children().indexOf( node );
}

void Folder::insertChild(TreeNode* node, TreeNode* after)
{
    int pos = d->children.indexOf(after);
    
    if (pos < 0)
        prependChild(node);
    else 
        insertChild(pos+1, node);
}

QIcon Folder::icon() const
{
    return KIcon("folder");
}

void Folder::insertChild(int index, TreeNode* node)
{
//    kDebug() <<"enter Folder::insertChild(int, node)" << node->title();
    if (node)
    {
        if (index >= d->children.size())
            d->children.append(node);
        else
            d->children.insert(index, node);
        node->setParent(this);
        connectToNode(node);
        updateUnreadCount();
        emit signalChildAdded(node);
        d->addedArticlesNotify += node->articles();
        articlesModified();
        nodeModified(); 
    }   
//    kDebug() <<"leave Folder::insertChild(int, node)" << node->title(); 
}

void Folder::appendChild(TreeNode* node)
{
//    kDebug() <<"enter Folder::appendChild()" << node->title();
    if (node)
    {
        d->children.append(node);
        node->setParent(this);
        connectToNode(node);
        updateUnreadCount();
        emit signalChildAdded(node);
        d->addedArticlesNotify += node->articles();
        articlesModified();
        nodeModified();
    }    
//    kDebug() <<"leave Folder::appendChild()" << node->title();
}

void Folder::prependChild(TreeNode* node)
{
//    kDebug() <<"enter Folder::prependChild()" << node->title();
    if (node)
    {
        d->children.prepend(node);
        node->setParent(this);
        connectToNode(node);
        updateUnreadCount();
        emit signalChildAdded(node);
        d->addedArticlesNotify += node->articles();
        articlesModified();
        nodeModified();
    }    
//    kDebug() <<"leave Folder::prependChild()" << node->title();
}

void Folder::removeChild(TreeNode* node)
{
    if (!node || !d->children.contains(node))
        return;
     
    emit signalAboutToRemoveChild( node );
    node->setParent(0);
    d->children.removeAll(node);
    disconnectFromNode(node);
    updateUnreadCount();    
    emit signalChildRemoved(this, node);
    d->removedArticlesNotify += node->articles();
    articlesModified(); // articles were removed, TODO: add guids to a list
    nodeModified();
}


TreeNode* Folder::firstChild()
{
    return d->children.isEmpty() ? 0 : children().first();
}            

const TreeNode* Folder::firstChild() const
{
    return d->children.isEmpty() ? 0 : children().first();
}            

TreeNode* Folder::lastChild()
{
    return d->children.isEmpty() ? 0 : children().last();
}

const TreeNode* Folder::lastChild() const
{
    return d->children.isEmpty() ? 0 : children().last();
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
    int total = 0;
    Q_FOREACH( const Feed* const i, feeds() )
        total += i->totalCount();
    return total;
}

void Folder::updateUnreadCount() const
{
    int unread = 0;
    Q_FOREACH ( const Feed* const i, feeds() )
        unread += i->unread();
    d->unread = unread;
}

void Folder::slotMarkAllArticlesAsRead() 
{
    setNotificationMode(false);
    Q_FOREACH( Feed* const i, feeds() )
        i->slotMarkAllArticlesAsRead();
    setNotificationMode(true);
}
    
void Folder::slotChildChanged(TreeNode* /*node*/)
{
    updateUnreadCount();
    nodeModified();
}

void Folder::slotChildDestroyed(TreeNode* node)
{
    d->children.removeAll(node);
    updateUnreadCount();    
    nodeModified();
}

void Folder::deleteExpiredArticles( Akregator::ArticleDeleteJob* job )
{
    setNotificationMode(false);
    Q_FOREACH( Feed* const i, feeds() )
        i->deleteExpiredArticles( job );
    setNotificationMode(true);
}

bool Folder::subtreeContains( const TreeNode* node ) const
{
    if ( node == this )
        return false;
    const Folder* parent = node ? node->parent() : 0;
    while ( parent )
    {
        if ( parent == this )
            return true;
        parent = parent->parent();
    }
    
    return false;
}

void Folder::slotAddToFetchQueue(FetchQueue* queue, bool intervalFetchOnly)
{
    Q_FOREACH( Feed* const i, feeds() )
        i->slotAddToFetchQueue( queue, intervalFetchOnly );
}

void Folder::doArticleNotification()
{
}

void Folder::connectToNode(TreeNode* child)
{
    connect(child, SIGNAL(signalChanged(Akregator::TreeNode*)), this, SLOT(slotChildChanged(Akregator::TreeNode*)));
    connect(child, SIGNAL(signalDestroyed(Akregator::TreeNode*)), this, SLOT(slotChildDestroyed(Akregator::TreeNode*)));
    connect(child, SIGNAL(signalArticlesAdded(Akregator::TreeNode*, QList<Akregator::Article>)), this, SIGNAL(signalArticlesAdded(Akregator::TreeNode*, QList<Akregator::Article>)));
    connect(child, SIGNAL(signalArticlesRemoved(Akregator::TreeNode*, QList<Akregator::Article>)), this, SIGNAL(signalArticlesRemoved(Akregator::TreeNode*, QList<Akregator::Article>)));
    connect(child, SIGNAL(signalArticlesUpdated(Akregator::TreeNode*, QList<Akregator::Article>)), this, SIGNAL(signalArticlesUpdated(Akregator::TreeNode*, QList<Akregator::Article>)));
}

void Folder::disconnectFromNode(TreeNode* child)
{
    assert( child );
    child->disconnect( this );
}


TreeNode* Folder::childAt( int pos )
{
    if ( pos < 0 || pos >= d->children.count() )
        return 0;
    return d->children.at( pos );
}

const TreeNode* Folder::childAt( int pos ) const
{
    if ( pos < 0 || pos >= d->children.count() )
        return 0;
    return d->children.at( pos );
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

const TreeNode* Folder::next() const
{
    if ( firstChild() )
        return firstChild();

    if ( nextSibling() )
        return nextSibling();
    
    const Folder* p = parent();
    while (p)
    {
        if ( p->nextSibling() )
            return p->nextSibling();
        else
            p = p->parent();
    }
    return 0;
}

#include "folder.moc"
