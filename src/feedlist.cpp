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

#include <qdom.h>
#include <qmap.h>
#include <qptrlist.h>
#include <qvaluelist.h>

#include <kdebug.h>
#include <klocale.h>

#include "archive.h"
#include "feed.h"
#include "feedgroup.h"



using namespace Akregator;

FeedList::FeedList(QObject *parent, const char *name)
    : QObject(parent, name), m_idCounter(2)
{
    m_rootNode = new FeedGroup(i18n("All Feeds"));
    m_rootNode->setId(1);
    m_idMap[1] = m_rootNode;
    m_flatList.append(m_rootNode);
    connectToNode(m_rootNode);
}

void FeedList::parseChildNodes(QDomNode &node, FeedGroup* parent)
{
    QDomElement e = node.toElement(); // try to convert the node to an element.
    
    if( !e.isNull() )
    {
        QString title = e.hasAttribute("text") ? e.attribute("text") : e.attribute("title");

        if (e.hasAttribute("xmlUrl") || e.hasAttribute("xmlurl"))
        {
            Feed* feed = Feed::fromOPML(e);
            parent->appendChild(feed);
            Archive::load(feed);
        }
        else
        {
            FeedGroup* fg = FeedGroup::fromOPML(e);
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

FeedList* FeedList::fromOPML(const QDomDocument& doc)
{
    FeedList* list = new FeedList();

    QDomElement root = doc.documentElement();

    kdDebug() << "loading OPML feed " << root.tagName().lower() << endl;
    
    if (root.tagName().lower() != "opml")
    {
        delete list;
        return 0;
    }
    QDomNode bodyNode = root.firstChild();
    
    while (!bodyNode.isNull() && bodyNode.toElement().tagName().lower() != "body")
        bodyNode = bodyNode.nextSibling();
   

    if (bodyNode.isNull())
    {
        kdDebug() << "Failed to acquire body node, markup broken?" << endl;
        delete list;
        return 0;
    }
    
    QDomElement body = bodyNode.toElement();

    QDomNode i = body.firstChild();

    list->m_idCounter = 0;
    
    while( !i.isNull() )
    {
        parseChildNodes(i, list->rootNode());
        i = i.nextSibling();
    }

    list->m_idCounter = 2;
    
    for (TreeNode* i = list->rootNode()->firstChild(); i && i != list->rootNode(); i = i->next() )
        if (i->id() >= list->m_idCounter)
            list->m_idCounter = i->id() + 1;

    for (TreeNode* i = list->rootNode()->firstChild(); i && i != list->rootNode(); i = i->next() )
        if (i->id() == 0)
    {
            uint id = list->m_idCounter++;
            i->setId(id);
            list->m_idMap[id] = i;
    }          
    return list;
}

FeedList::~FeedList()
{
    emit signalDestroyed(this);
    delete m_rootNode;
    m_rootNode = 0;
}

TreeNode* FeedList::findByID(uint id) const
{
    return m_idMap.contains(id) ? m_idMap[id] : 0;
}

bool FeedList::isEmpty() const
{
    return m_rootNode->firstChild() == 0;
}

FeedGroup* FeedList::rootNode() const
{
    return m_rootNode;
}
    
void FeedList::append(FeedList* list, FeedGroup* parent, TreeNode* after)
{
    if ( list == this )
        return;

    if ( !m_flatList.contains(parent) )
        parent = rootNode();

    QPtrList<TreeNode> children = list->rootNode()->children();
    
    for (TreeNode* i = children.first(); i; i = children.next() )
    {
        list->rootNode()->removeChild(i);
        parent->insertChild(i, after);
        after = i;
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

    QPtrList<TreeNode> children = rootNode()->children();
    for (TreeNode* i = children.first(); i; i = children.next() )
        body.appendChild( i->toOPML(body, doc) );

    return doc;
}

const QString& FeedList::title() const
{
    return m_title;
}

void FeedList::setTitle(const QString& title)
{
    m_title = title;
}
    
void FeedList::slotNodeAdded(TreeNode* node)
{
    FeedGroup* parent = node->parent();
    if ( !node || !m_flatList.contains(parent) || m_flatList.contains(node) )
        return;


    if (m_idCounter != 0)
    {
        node->setId(m_idCounter++);
        m_idMap[node->id()] = node;
    }
    
    m_flatList.append(node);
    connectToNode(node);

    if ( !node->isGroup() )
        return;
    
    // if adding a feed group, also connect to sub tree
    FeedGroup* fg = static_cast<FeedGroup*> (node);
    for (TreeNode* i = fg->firstChild(); i && i != fg; i = i->next() )
    {
        m_flatList.append(i);
        connectToNode(i);
    }
}

//void FeedList::slotNodeChanged(TreeNode* node)
//{}

void FeedList::slotNodeDestroyed(TreeNode* node)
{
    if ( !node || !m_flatList.contains(node) )
        return;
    
    m_idMap.remove(node->id());
    m_flatList.remove(node);
}

void FeedList::slotNodeRemoved(FeedGroup* /*parent*/, TreeNode* node)
{
    if ( !node || !m_flatList.contains(node) )
        return;
    
    m_idMap.remove(node->id());
    disconnectFromNode(node);
    m_flatList.remove(node);
    
}

void FeedList::connectToNode(TreeNode* node)
{
    if (node->isGroup())
    {
        FeedGroup* fg = static_cast<FeedGroup*>(node);
                       
        connect(fg, SIGNAL(signalChildAdded(TreeNode*)), this, SLOT(slotNodeAdded(TreeNode*) ));
        connect(fg, SIGNAL(signalChildRemoved(FeedGroup*, TreeNode*)), this, SLOT(slotNodeRemoved(FeedGroup*, TreeNode*) ));
        connect(fg, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotNodeDestroyed(TreeNode*) ));
//        connect(fg, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotNodeChanged(TreeNode*) ));
    }
    else
    {
        Feed* f = static_cast<Feed*> (node);
        
        connect(f, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotNodeDestroyed(TreeNode*) ));
//        connect(f, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotNodeChanged(TreeNode*) ));
    }
}
            
void FeedList::disconnectFromNode(TreeNode* node)
{
    if (node->isGroup())
    {
        FeedGroup* fg = static_cast<FeedGroup*> (node);
        disconnect(fg, SIGNAL(signalChildAdded(TreeNode*)), this, SLOT(slotNodeAdded(TreeNode*) ));
        disconnect(fg, SIGNAL(signalChildRemoved(FeedGroup*, TreeNode*)), this, SLOT(slotNodeRemoved(FeedGroup*, TreeNode*) ));
        disconnect(fg, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotNodeDestroyed(TreeNode*) ));
//        disconnect(fg, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotNodeChanged(TreeNode*) ));
    }
    else
    {
        Feed* f = static_cast<Feed*> (node);
        disconnect(f, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotNodeDestroyed(TreeNode*) ));
//        disconnect(f, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotNodeChanged(TreeNode*) ));
    }
}
    
#include "feedlist.moc"
