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

#include "feedlist.h"
#include "tag.h"
#include "tagnode.h"
#include "tagnodelist.h"
#include "tagset.h"
#include "folder.h"
#include "tagfolder.h"

#include <qdom.h>
#include <qmap.h>
#include <qstring.h>
#include <qvaluelist.h>

#include <klocale.h>

namespace Akregator {

class TagNodeList::TagNodeListPrivate
{
    public:
    TagFolder* rootNode;
    FeedList* feedList;
    TagSet* tagSet;
    QMap<QString, TagNode*> idToNodeMap;
};

TagNodeList::TagNodeList(FeedList* feedList, TagSet* tagSet) : d(new TagNodeListPrivate)
{
    d->feedList = feedList;
    d->tagSet = tagSet;
    d->rootNode = new TagFolder(i18n("My Tags"));
    connect(d->rootNode, SIGNAL(signalChildAdded(TreeNode*)), this, SLOT(slotNodeAdded(TreeNode*)));
    connect(d->rootNode, SIGNAL(signalChildRemoved(Folder*, TreeNode*)), this, SLOT(slotNodeRemoved(Folder*, TreeNode*)));
    connect(d->tagSet, SIGNAL(signalTagAdded(const Tag&)), this, SLOT(slotTagAdded(const Tag&)));
    connect(d->tagSet, SIGNAL(signalTagRemoved(const Tag&)), this, SLOT(slotTagRemoved(const Tag&)));
    connect(d->tagSet, SIGNAL(signalTagUpdated(const Tag&)), this, SLOT(slotTagUpdated(const Tag&)));

    QValueList<Tag> list = tagSet->toMap().values();
    for (QValueList<Tag>::ConstIterator it = list.begin(); it != list.end(); ++it)
    {
       insert(new TagNode(*it, d->feedList->rootNode()));
    }
}

TagNodeList::~TagNodeList()
{
    delete d->rootNode;
    delete d;
    d = 0;
}

TagFolder* TagNodeList::rootNode()
{
    return d->rootNode;
}

bool TagNodeList::insert(TagNode* tagNode)
{
    QString id = tagNode->tag().id();
    if (!containsTagId(id))
    {
        rootNode()->appendChild(tagNode); // TODO: maintain sorting
        d->idToNodeMap[id] = tagNode;
        emit signalTagNodeAdded(tagNode);
        return true;
    }
    return false;
}

bool TagNodeList::remove(TagNode* tagNode)
{
    QString id = tagNode->tag().id();
    if (containsTagId(id))
    {
        d->rootNode->removeChild(tagNode);
        d->idToNodeMap.remove(id);
        emit signalTagNodeRemoved(tagNode);
        return true;
    }
    return false;
}

void TagNodeList::slotNodeDestroyed(TreeNode* node)
{
    TagNode* tagNode = dynamic_cast<TagNode*>(node);
    QString id = tagNode ? tagNode->tag().id() : QString::null;
    
    if (tagNode != 0 && containsTagId(id))
    {
        d->rootNode->removeChild(tagNode);
        d->idToNodeMap.remove(id);
        emit signalTagNodeRemoved(tagNode);
    }
}

void TagNodeList::slotNodeAdded(TreeNode* node)
{
    TagNode* tagNode = dynamic_cast<TagNode*>(node);
    QString id = tagNode ? tagNode->tag().id() : QString::null;
    
    if (tagNode != 0L && !containsTagId(id))
    {
       d->idToNodeMap[id] = tagNode;
       emit signalTagNodeAdded(tagNode);
    }
}

void TagNodeList::slotNodeRemoved(Folder* parent, TreeNode* node)
{
    
     TagNode* tagNode = dynamic_cast<TagNode*>(node);
     QString id = tagNode ? tagNode->tag().id() : QString::null;

     if (parent == d->rootNode && tagNode != 0L && containsTagId(id))
     {
         d->idToNodeMap.remove(id);
         emit signalTagNodeRemoved(tagNode);
     }
}

bool TagNodeList::containsTagId(const QString& tagId)
{
    return d->idToNodeMap.contains(tagId);
}

QValueList<TagNode*> TagNodeList::toList() const
{
    return d->idToNodeMap.values();
}

QDomDocument TagNodeList::toXML() const
{
    return QDomDocument();
}

void TagNodeList::slotTagAdded(const Tag& tag)
{
    if (!containsTagId(tag.id()))
    {
        insert(new TagNode(tag, d->feedList->rootNode()));
    }
}

void TagNodeList::slotTagUpdated(const Tag& tag)
{
    if (containsTagId(tag.id()))
    {
        d->idToNodeMap[tag.id()]->setTitle(tag.name());
    }
}
void TagNodeList::slotTagRemoved(const Tag& tag)
{
    if (containsTagId(tag.id()))
    {
        delete d->idToNodeMap[tag.id()];
        d->idToNodeMap[tag.id()] = 0;
    }
}

     
} // namespace Akregator

#include "tagnodelist.moc"
