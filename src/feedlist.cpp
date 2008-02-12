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
#include "storage.h"
#include "feedlist.h"
#include "article.h"
#include "feed.h"
#include "folder.h"
#include "treenode.h"
#include "treenodevisitor.h"

#include <kdebug.h>
#include <klocale.h>
#include <krandom.h>

#include <qdom.h>
#include <QHash>
#include <QTime>

namespace Akregator {

class FeedList::FeedListPrivate
{
public:
    Akregator::Backend::Storage* storage;
    QList<TreeNode*> flatList;
    Folder* rootNode;
    QString title;
    QHash<int, TreeNode*> idMap;
    AddNodeVisitor* addNodeVisitor;
    RemoveNodeVisitor* removeNodeVisitor;
    QHash<QString, QList<Feed*> > urlMap;
};

class FeedList::AddNodeVisitor : public TreeNodeVisitor
{
    public:
        AddNodeVisitor(FeedList* list) : m_list(list) {}


        bool visitFeed(Feed* node)
        {
            m_list->d->idMap.insert(node->id(), node);
            m_list->d->flatList.append(node);
            m_list->d->urlMap[node->xmlUrl()].append(node);
            connect( node, SIGNAL( fetchStarted( Akregator::Feed* ) ),
                     m_list, SIGNAL( fetchStarted( Akregator::Feed* ) ) );
            connect( node, SIGNAL( fetched( Akregator::Feed* ) ),
                     m_list, SIGNAL( fetched( Akregator::Feed* ) ) );
            connect( node, SIGNAL( fetchAborted( Akregator::Feed* ) ),
                     m_list, SIGNAL( fetchAborted( Akregator::Feed* ) ) );
            connect( node, SIGNAL( fetchError( Akregator::Feed* ) ),
                     m_list, SIGNAL( fetchError( Akregator::Feed* ) ) );
            connect( node, SIGNAL( fetchDiscovery( Akregator::Feed* ) ),
                     m_list, SIGNAL( fetchDiscovery( Akregator::Feed* ) ) );
            
                    
            visitTreeNode(node);
            return true;
        }

        void visit(TreeNode* node, bool preserveID)
        {
            m_preserveID = preserveID;
            TreeNodeVisitor::visit(node);
        }

        bool visitTreeNode(TreeNode* node)
        {
            if (!m_preserveID)
            	node->setId(m_list->generateID());
            m_list->d->idMap[node->id()] = node;
            m_list->d->flatList.append(node);
            
            connect(node, SIGNAL(signalDestroyed(Akregator::TreeNode*)), m_list, SLOT(slotNodeDestroyed(Akregator::TreeNode*) ));
            connect( node, SIGNAL( signalChanged( Akregator::TreeNode* ) ), m_list, SIGNAL( signalNodeChanged(Akregator::TreeNode* ) ) );
            emit m_list->signalNodeAdded(node);

            return true;
        }

        bool visitFolder(Folder* node)
        {
            connect(node, SIGNAL(signalChildAdded(Akregator::TreeNode*)), m_list, SLOT(slotNodeAdded(Akregator::TreeNode*) ));
            connect( node, SIGNAL( signalAboutToRemoveChild( Akregator::TreeNode* ) ), m_list, SIGNAL( signalAboutToRemoveNode( Akregator::TreeNode* ) ) );
            connect(node, SIGNAL(signalChildRemoved(Akregator::Folder*, Akregator::TreeNode*)), m_list, SLOT(slotNodeRemoved(Akregator::Folder*, Akregator::TreeNode*) ));

            visitTreeNode(node);

            for (TreeNode* i = node->firstChild(); i && i != node; i = i->next() )
                m_list->slotNodeAdded(i);

            return true;
        }

    private:
        FeedList* m_list;
        bool m_preserveID;
};

class FeedList::RemoveNodeVisitor : public TreeNodeVisitor
{
    public:
        RemoveNodeVisitor(FeedList* list) : m_list(list) {}

        bool visitFeed(Feed* node)
        {
            visitTreeNode( node );
            m_list->d->urlMap[node->xmlUrl()].removeAll(node);
            return true;
        }

        bool visitTreeNode(TreeNode* node)
        {
            m_list->d->idMap.remove(node->id());
            m_list->d->flatList.removeAll(node);
            m_list->disconnect( node );
            return true;
        }

        bool visitFolder(Folder* node)
        {
            visitTreeNode(node);
          
            return true;
        }

    private:
        FeedList* m_list;
};

FeedList::FeedList(Akregator::Backend::Storage* storage, QObject *parent)
    : QObject(parent), d(new FeedListPrivate)
{
    Q_ASSERT( storage );
    d->storage = storage;
    d->rootNode = 0;
    d->addNodeVisitor = new AddNodeVisitor(this);
    d->removeNodeVisitor = new RemoveNodeVisitor(this);
    Folder* rootNode = new Folder(i18n("All Feeds"));
    rootNode->setId(1);
    setRootNode(rootNode);
    addNode(rootNode, true);
}

void FeedList::addNode(TreeNode* node, bool preserveID)
{
    d->addNodeVisitor->visit(node, preserveID);
}

void FeedList::removeNode(TreeNode* node)
{
    d->removeNodeVisitor->visit(node);
}

void FeedList::parseChildNodes(QDomNode &node, Folder* parent)
{
    QDomElement e = node.toElement(); // try to convert the node to an element.

    if( !e.isNull() )
    {
        QString title = e.hasAttribute("text") ? e.attribute("text") : e.attribute("title");

        if (e.hasAttribute("xmlUrl") || e.hasAttribute("xmlurl") || e.hasAttribute("xmlURL") )
        {
            Feed* feed = Feed::fromOPML(e, d->storage);
            if (feed)
            {
                if (!d->urlMap[feed->xmlUrl()].contains(feed))
                    d->urlMap[feed->xmlUrl()].append(feed);
                parent->appendChild(feed);
            }
        }
        else
        {
            Folder* fg = Folder::fromOPML(e);
            parent->appendChild(fg);

            if (e.hasChildNodes())
            {
                QDomNode child = e.firstChild();
                while(!child.isNull())
                {
                    parseChildNodes(child, fg);
                    child = child.nextSibling();
                }
            }
        }
    }
}

bool FeedList::readFromXML(const QDomDocument& doc)
{
    QDomElement root = doc.documentElement();

    kDebug() <<"loading OPML feed" << root.tagName().toLower();

    kDebug() <<"measuring startup time: START";
    QTime spent;
    spent.start();

    if (root.tagName().toLower() != "opml")
    {
        return false;
    }
    QDomNode bodyNode = root.firstChild();

    while (!bodyNode.isNull() && bodyNode.toElement().tagName().toLower() != "body")
        bodyNode = bodyNode.nextSibling();


    if (bodyNode.isNull())
    {
        kDebug() <<"Failed to acquire body node, markup broken?";
        return false;
    }

    QDomElement body = bodyNode.toElement();

    QDomNode i = body.firstChild();

    while( !i.isNull() )
    {
        parseChildNodes(i, rootNode());
        i = i.nextSibling();
    }

    for (TreeNode* i = rootNode()->firstChild(); i && i != rootNode(); i = i->next() )
        if (i->id() == 0)
    {
            uint id = generateID();
            i->setId(id);
            d->idMap.insert(id, i);
    }

    kDebug() <<"measuring startup time: STOP," << spent.elapsed() <<"ms";
    kDebug() <<"Number of articles loaded:" << rootNode()->totalCount();
    return true;
}

FeedList::~FeedList()
{
    emit signalDestroyed(this);
    setRootNode(0);
    delete d->addNodeVisitor;
    delete d->removeNodeVisitor;
    delete d;
    d = 0;
}

const Feed* FeedList::findByURL(const QString& feedURL) const
{
    if (d->urlMap[feedURL].isEmpty())
        return 0;
    else
        return *(d->urlMap[feedURL].begin());
}

Feed* FeedList::findByURL(const QString& feedURL)
{
    if (d->urlMap[feedURL].isEmpty())
        return 0;
    else
        return *(d->urlMap[feedURL].begin());
}

const Article FeedList::findArticle(const QString& feedURL, const QString& guid) const
{
    const Feed* feed = findByURL(feedURL);
    return feed ? feed->findArticle(guid) : Article();
}

void FeedList::append(FeedList* list, Folder* parent, TreeNode* after)
{
    if ( list == this )
        return;

    if ( !d->flatList.contains(parent) )
        parent = rootNode();

    QList<TreeNode*> children = list->rootNode()->children();

    QList<TreeNode*>::ConstIterator end(  children.end() );
    for (QList<TreeNode*>::ConstIterator it = children.begin(); it != end; ++it)
    {
        list->rootNode()->removeChild(*it);
        parent->insertChild(*it, after);
        after = *it;
    }
}

QDomDocument FeedList::toXML() const
{
    QDomDocument doc;
    doc.appendChild( doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );

    QDomElement root = doc.createElement( "opml" );
    root.setAttribute( "version", "1.0" );
    doc.appendChild( root );

    QDomElement head = doc.createElement( "head" );
    root.appendChild( head );

    QDomElement ti = doc.createElement( "text" );
    head.appendChild( ti );

    QDomText t = doc.createTextNode( title() );
    ti.appendChild( t );

    QDomElement body = doc.createElement( "body" );
    root.appendChild( body );

    foreach( const TreeNode* const i, rootNode()->children() )
        body.appendChild( i->toOPML(body, doc) );

    return doc;
}

QString FeedList::title() const
{
    return d->title;
}

const TreeNode* FeedList::findByID(int id) const
{
    return d->idMap[id];
}

TreeNode* FeedList::findByID(int id)
{
    return d->idMap[id];
}

void FeedList::setTitle(const QString& title)
{
    d->title = title;
}

const Folder* FeedList::rootNode() const
{
    return d->rootNode;
}

Folder* FeedList::rootNode()
{
    return d->rootNode;
}

QList<TreeNode*> FeedList::asFlatList()
{
    return d->flatList;
}

bool FeedList::isEmpty() const
{
    return d->rootNode->firstChild() == 0;
}

void FeedList::clear()
{
    Q_ASSERT(rootNode());

    QList<TreeNode*> children = rootNode()->children();

    for (QList<TreeNode*>::ConstIterator it = children.begin(); it != children.end(); ++it)
        delete *it; // emits signal "emitSignalDestroyed"
}

void FeedList::setRootNode(Folder* folder)
{
    delete d->rootNode;
    d->rootNode = folder;

    if (d->rootNode)
    {
        d->rootNode->setOpen(true);
        connect(d->rootNode, SIGNAL(signalChildAdded(Akregator::TreeNode*)), this, SLOT(slotNodeAdded(Akregator::TreeNode*)));
        connect(d->rootNode, SIGNAL(signalAboutToRemoveChild(Akregator::TreeNode*)), this, SIGNAL(signalAboutToRemoveNode(Akregator::TreeNode*)));
        connect(d->rootNode, SIGNAL(signalChildRemoved(Akregator::Folder*, Akregator::TreeNode*)), this, SLOT(slotNodeRemoved(Akregator::Folder*, Akregator::TreeNode*)));
        connect( d->rootNode, SIGNAL( signalChanged(Akregator::TreeNode* ) ), this, SIGNAL( signalNodeChanged(Akregator::TreeNode* ) ) );
    }
}

int FeedList::generateID() const
{
    return KRandom::random();
}

void FeedList::slotNodeAdded(TreeNode* node)
{
    if (!node) return;

    Folder* parent = node->parent();
    if ( !parent || !d->flatList.contains(parent) || d->flatList.contains(node) )
        return;

    addNode(node, false);
}

void FeedList::slotNodeDestroyed(TreeNode* node)
{
    if ( !node || !d->flatList.contains(node) )
        return;
    removeNode(node);
}

void FeedList::slotNodeRemoved(Folder* /*parent*/, TreeNode* node)
{
    if ( !node || !d->flatList.contains(node) )
        return;
    removeNode(node);
    emit signalNodeRemoved( node );
}

} // namespace Akregator

#include "feedlist.moc"
