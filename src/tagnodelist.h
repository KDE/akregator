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

#ifndef AKREGATOR_TAGNODELIST_H
#define AKREGATOR_TAGNODELIST_H

#include <qobject.h>

class QDomDocument;
template <class T> class QValueList;

namespace Akregator {

   class FeedList;
   class Folder;
   class TagNode;
   class TreeNode;
   class Tag;
   class TagSet;
   
   
   class TagNodeList : public QObject
   {
       Q_OBJECT
       public:


       TagNodeList(FeedList* feedList, TagSet* tagSet);
       virtual ~TagNodeList();
       
       bool insert(TagNode* tagNode);
       bool remove(TagNode* tagNode);
       bool containsTagId(const QString& tagId);
       QValueList<TagNode*> toList() const;

       QDomDocument toXML() const;

       Folder* rootNode();
       
       public slots:

       void slotNodeDestroyed(TreeNode* node);
       void slotNodeAdded(TreeNode* node);
       void slotNodeRemoved(Folder* parent, TreeNode* node);

       void slotTagAdded(const Tag& tag);
       void slotTagRemoved(const Tag& tag);
       
       signals:

       void signalTagNodeAdded(TagNode* node);
       void signalTagNodeRemoved(TagNode* node);
       
       private:
       
       class TagNodeListPrivate;
       TagNodeListPrivate* d;
   };

} // namespace Akregator

#endif
