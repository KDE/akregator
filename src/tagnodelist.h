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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef AKREGATOR_TAGNODELIST_H
#define AKREGATOR_TAGNODELIST_H

#include "tagfolder.h"

class QDomDocument;
template <class T> class QList;

namespace Akregator {

class FeedList;
class Folder;
class TagFolder;
class TagNode;
class TreeNode;
class Tag;
class TagSet;


class TagNodeList : public NodeList
{
    Q_OBJECT
    public:


    TagNodeList(FeedList* feedList, TagSet* tagSet);
    virtual ~TagNodeList();
    
    FeedList* feedList() const;
    bool insert(TagNode* tagNode);
    bool remove(TagNode* tagNode);
    bool containsTagId(const QString& tagId);
    QList<TagNode*> toList() const;
    
    TagNode* findByTagID(const QString& tagID);

    virtual bool readFromXML(const QDomDocument& doc);
    virtual QDomDocument toXML() const;

    virtual TagFolder* rootNode() const;
    
    public slots:

    void slotTagAdded(const Tag& tag);
    void slotTagUpdated(const Tag& tag);
    void slotTagRemoved(const Tag& tag);
    
    signals:

    void signalDestroyed(TagNodeList*);
    void signalTagNodeAdded(TagNode* node);
    void signalTagNodeRemoved(TagNode* node);

    protected slots:

    virtual void slotNodeDestroyed(TreeNode* node);
    virtual void slotNodeAdded(TreeNode* node);
    virtual void slotNodeRemoved(Folder* parent, TreeNode* node);

    private:
    
    class TagNodeListPrivate;
    TagNodeListPrivate* d;
};

} // namespace Akregator

#endif // AKREGATOR_TAGNODELIST_H
