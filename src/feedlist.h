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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef AKREGATOR_FEEDLIST_H
#define AKREGATOR_FEEDLIST_H

#include "nodelist.h"
#include <kdepim_export.h>

class QDomDocument;
class QDomNode;
class QString;

namespace Akregator
{

class Article;
class Feed;
class Folder;
class TreeNode;

/** The model of a feed tree, represents an OPML document. Contains an additional root node "All Feeds" which isn't stored. Note that a node instance must not be in more than one FeedList at a time! When deleting the feed list, all contained nodes are deleted! */

class AKREGATOR_EXPORT FeedList : public NodeList
{
Q_OBJECT
public:

    FeedList(QObject *parent = 0);

    /** Destructor. Contained nodes are deleted! */
    ~FeedList();

    /** appends another feed list as sub tree. The root node of @c list is ignored. NOTE: nodes are _moved_ from @c list to this feed list, not copied */
    
    void append(FeedList* list, Folder* parent=0, TreeNode* after=0);

    /** reads an OPML document and appends the items to this list
        @param doc the OPML document to parse
        @return whether parsing was successful or not (TODO: make errors more detailed)
    */
    virtual bool readFromXML(const QDomDocument& doc);

    /** exports the feed list as OPML. The root node ("All Feeds") is ignored! */
    virtual QDomDocument toXML() const;

    /** returns a feed object for a given feed URL. If the feed list does not contain a feed with @c url, NULL is returned. If it contains the same feed multiple times, any of the Feed objects is returned. */
    Feed* findByURL(const QString& feedURL) const;

    Article findArticle(const QString& feedURL, const QString& guid) const;

signals:

    void signalDestroyed(FeedList*);

protected:

    virtual void addNode(TreeNode* node, bool preserveID);
    virtual void removeNode(TreeNode* node);

private:

    void parseChildNodes(QDomNode &node, Folder* parent);

    // never call these
    FeedList(const FeedList&) : NodeList() {}
    FeedList& operator=(const FeedList&) { return *this; }

    friend class AddNodeVisitor;
    class AddNodeVisitor;

    friend class RemoveNodeVisitor;
    class RemoveNodeVisitor;
    
    class FeedListPrivate;
    FeedListPrivate* d;
};

} // namespace Akregator

#endif // AKREGATOR_FEEDLIST_H
