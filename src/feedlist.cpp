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
#include "feedlist.h"

#include <qdatetime.h>
#include <qdom.h>
#include <qmap.h>
#include <qvaluelist.h>

#include <kdebug.h>
#include <klocale.h>

#include "article.h"
#include "feed.h"
#include "folder.h"
#include "treenode.h"
#include "treenodevisitor.h"

namespace Akregator {

class FeedList::FeedListPrivate
{
    public:
        uint idCounter;
        QMap<uint, TreeNode*> idMap;
        QMap<QString, QValueList<Feed*> > urlMap;
        QValueList<TreeNode*> flatList;
        Folder* rootNode;
        QString title;
        AddNodeVisitor* addNodeVisitor;
        RemoveNodeVisitor* removeNodeVisitor;
};

class FeedList::AddNodeVisitor : public TreeNodeVisitor
{
    public:
        AddNodeVisitor(FeedList* list) : m_list(list) {}


        virtual bool visitFeed(Feed* node)
        {
            if (!m_preserveID)
            node->setId(m_list->d->idCounter++);
            m_list->d->idMap[node->id()] = node;
            m_list->d->flatList.append(node);
            if (!m_list->d->urlMap[node->xmlUrl()].contains(node))
                    m_list->d->urlMap[node->xmlUrl()].append(node);

            connect(node, SIGNAL(signalDestroyed(TreeNode*)), m_list, SLOT(slotNodeDestroyed(TreeNode*) ));
            m_list->signalNodeAdded(node); // emit

            return true;
        }
        virtual bool visitFolder(Folder* node)
        {
            node->setId(m_list->d->idCounter++);
            m_list->d->idMap[node->id()] = node;
            m_list->d->flatList.append(node);

            connect(node, SIGNAL(signalDestroyed(TreeNode*)), m_list, SLOT(slotNodeDestroyed(TreeNode*) ));

            connect(node, SIGNAL(signalChildAdded(TreeNode*)), m_list, SLOT(slotNodeAdded(TreeNode*) ));
            connect(node, SIGNAL(signalChildRemoved(Folder*, TreeNode*)), m_list, SLOT(slotNodeRemoved(Folder*, TreeNode*) ));

            m_list->signalNodeAdded(node); // emit

            for (TreeNode* i = node->firstChild(); i && i != node; i = i->next() )
                m_list->slotNodeAdded(i);

            return true;
        }

        virtual void visit(TreeNode* node, bool preserveID)
        {
            m_preserveID = preserveID;
            TreeNodeVisitor::visit(node);
        }

    private:
        FeedList* m_list;
        bool m_preserveID;
};

class FeedList::RemoveNodeVisitor : public TreeNodeVisitor
{
    public:
        RemoveNodeVisitor(FeedList* list) : m_list(list) {}

        virtual bool visitFeed(Feed* node)
        {
            m_list->d->idMap.remove(node->id());
            m_list->d->flatList.remove(node);
            m_list->d->urlMap[node->xmlUrl()].remove(node);

            disconnect(node, SIGNAL(signalDestroyed(TreeNode*)), m_list, SLOT(slotNodeDestroyed(TreeNode*) ));

            m_list->signalNodeRemoved(node); // emit signal

            return true;
        }
        virtual bool visitFolder(Folder* node)
        {
            m_list->d->idMap.remove(node->id());
            m_list->d->flatList.remove(node);

            disconnect(node, SIGNAL(signalDestroyed(TreeNode*)), m_list, SLOT(slotNodeDestroyed(TreeNode*) ));
            disconnect(node, SIGNAL(signalChildAdded(TreeNode*)), m_list, SLOT(slotNodeAdded(TreeNode*) ));
            disconnect(node, SIGNAL(signalChildRemoved(Folder*, TreeNode*)), m_list, SLOT(slotNodeRemoved(Folder*, TreeNode*) ));
            disconnect(node, SIGNAL(signalDestroyed(TreeNode*)), m_list, SLOT(slotNodeDestroyed(TreeNode*) ));

            m_list->signalNodeRemoved(node); // emit signal

            return true;
        }
    private:
        FeedList* m_list;
};

FeedList::FeedList(QObject *parent, const char *name)
    : QObject(parent, name), d(new FeedListPrivate)
{
    d->addNodeVisitor = new AddNodeVisitor(this);
    d->removeNodeVisitor = new RemoveNodeVisitor(this);

    d->idCounter = 2;
    d->rootNode = new Folder(i18n("All Feeds"));
    d->rootNode->setId(1);
    d->addNodeVisitor->visit(d->rootNode, true);
}


const QValueList<TreeNode*>& FeedList::asFlatList() const
{
    return d->flatList;
}

void FeedList::parseChildNodes(QDomNode &node, Folder* parent)
{
    QDomElement e = node.toElement(); // try to convert the node to an element.

    if( !e.isNull() )
    {
        QString title = e.hasAttribute("text") ? e.attribute("text") : e.attribute("title");

        if (e.hasAttribute("xmlUrl") || e.hasAttribute("xmlurl"))
        {
            Feed* feed = Feed::fromOPML(e);
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

bool FeedList::readFromOPML(const QDomDocument& doc)
{
    QDomElement root = doc.documentElement();

    kdDebug() << "loading OPML feed " << root.tagName().lower() << endl;

    kdDebug() << "measuring startup time: START" << endl;
    QTime spent;
    spent.start();

    if (root.tagName().lower() != "opml")
    {
        return false;
    }
    QDomNode bodyNode = root.firstChild();

    while (!bodyNode.isNull() && bodyNode.toElement().tagName().lower() != "body")
        bodyNode = bodyNode.nextSibling();


    if (bodyNode.isNull())
    {
        kdDebug() << "Failed to acquire body node, markup broken?" << endl;
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
        if (i->id() >= d->idCounter)
            d->idCounter = i->id() + 1;

    for (TreeNode* i = rootNode()->firstChild(); i && i != rootNode(); i = i->next() )
        if (i->id() == 0)
    {
            uint id = d->idCounter++;
            i->setId(id);
            d->idMap[id] = i;
    }

    kdDebug() << "measuring startup time: STOP, " << spent.elapsed() << "ms" << endl;
    kdDebug() << "Number of articles loaded: " << rootNode()->totalCount() << endl;
    return true;
}

FeedList::~FeedList()
{
    emit signalDestroyed(this);
    delete d->rootNode;
    d->rootNode = 0;
    delete d->addNodeVisitor;
    delete d->removeNodeVisitor;
    delete d;
    d = 0;
}

TreeNode* FeedList::findByID(uint id) const
{
    return d->idMap.contains(id) ? d->idMap[id] : 0;
}

Feed* FeedList::findByURL(const QString& feedURL) const
{
    if (d->urlMap[feedURL].isEmpty())
        return 0;
    else
        return *(d->urlMap[feedURL].begin());
}

Article FeedList::findArticle(const QString& feedURL, const QString& guid) const
{
    Feed* feed = findByURL(feedURL);

    return feed ? feed->findArticle(guid) : Article();
}

bool FeedList::isEmpty() const
{
    return d->rootNode->firstChild() == 0;
}

Folder* FeedList::rootNode() const
{
    return d->rootNode;
}

void FeedList::append(FeedList* list, Folder* parent, TreeNode* after)
{
    if ( list == this )
        return;

    if ( !d->flatList.contains(parent) )
        parent = rootNode();

    QValueList<TreeNode*> children = list->rootNode()->children();

    QValueList<TreeNode*>::ConstIterator end(  children.end() );
    for (QValueList<TreeNode*>::ConstIterator it = children.begin(); it != end; ++it)
    {
        list->rootNode()->removeChild(*it);
        parent->insertChild(*it, after);
        after = *it;
    }
}

QDomDocument FeedList::toOPML() const
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

    QValueList<TreeNode*> children = rootNode()->children();

    QValueList<TreeNode*>::ConstIterator end(  children.end() );

    for (QValueList<TreeNode*>::ConstIterator it = children.begin(); it != end; ++it)
        body.appendChild( (*it)->toOPML(body, doc) );

    return doc;
}

const QString& FeedList::title() const
{
    return d->title;
}

void FeedList::setTitle(const QString& title)
{
    d->title = title;
}

void FeedList::slotNodeAdded(TreeNode* node)
{
    Folder* parent = node->parent();
    if ( !node || !d->flatList.contains(parent) || d->flatList.contains(node) )
        return;

    d->addNodeVisitor->visit(node, false);
}

//void FeedList::slotNodeChanged(TreeNode* node)
//{}

void FeedList::slotNodeDestroyed(TreeNode* node)
{
    if ( !node || !d->flatList.contains(node) )
        return;

    d->removeNodeVisitor->visit(node);
}

void FeedList::slotNodeRemoved(Folder* /*parent*/, TreeNode* node)
{
    if ( !node || !d->flatList.contains(node) )
        return;

    d->removeNodeVisitor->visit(node);
}

} // namespace Akregator
#include "feedlist.moc"
