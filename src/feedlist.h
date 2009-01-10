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

#ifndef AKREGATOR_FEEDLIST_H
#define AKREGATOR_FEEDLIST_H

#include "akregator_export.h"

#include "feedlistmanagementinterface.h"

#include <QObject>
#include <QPointer>

#include <boost/shared_ptr.hpp>

class QDomDocument;
class QDomNode;
template <class T> class QList;
template <class K,class T> class QHash;
class QString;

class KJob;

namespace Akregator {

class Article;
class Feed;
class FeedList;
class FetchQueue;
class Folder;
class TreeNode;

namespace Backend {
    class Storage;
}


class AKREGATORPART_EXPORT FeedListManagementImpl : public FeedListManagementInterface {
public:
    explicit FeedListManagementImpl( const boost::shared_ptr<FeedList>& list=boost::shared_ptr<FeedList>() );
    void setFeedList( const boost::shared_ptr<FeedList>& list );

    /* reimp */ QStringList categories() const;
    /* reimp */ QStringList feeds( const QString& catId ) const;
    /* reimp */ void addFeed( const QString& url, const QString& catId );
    /* reimp */ void removeFeed( const QString& url, const QString& catId );
    /* reimp */ QString addCategory( const QString& name, const QString& parentId ) const;
    /* reimp */ QString getCategoryName( const QString& catId ) const;

private:
    boost::shared_ptr<FeedList> m_feedList;
};

/** The model of a feed tree, represents an OPML document. Contains an additional root node "All Feeds" which isn't stored. Note that a node instance must not be in more than one FeedList at a time! When deleting the feed list, all contained nodes are deleted! */

class AKREGATORPART_EXPORT FeedList : public QObject
{
    Q_OBJECT
public:

    explicit FeedList(Akregator::Backend::Storage* storage);

    /** Destructor. Contained nodes are deleted! */
    ~FeedList();

    const Folder* rootNode() const;
    Folder* rootNode();

    bool isEmpty() const;

    const TreeNode* findByID(int id) const;
    TreeNode* findByID(int id);

    QList<const TreeNode*> findByTitle(const QString &title) const;
    QList<TreeNode*> findByTitle(const QString &title);


    /** returns the title of the feed list (as used in the OPML document) */
    QString title() const;

    /** sets the title of the feed list */
    void setTitle(const QString& name);

    /**
     * returns all feeds in this list
     */
    QVector<const Feed*> feeds() const;
    QVector<Feed*> feeds();

    QVector<int> feedIds() const;

    /**
     * returns all folders in this list
     */
    QVector<const Folder*> folders() const;
    QVector<Folder*> folders();

    /** appends another feed list as sub tree. The root node of @c list is ignored. NOTE: nodes are _moved_ from @c list to this feed list, not copied */

    void append(FeedList* list, Folder* parent=0, TreeNode* after=0);

    /** reads an OPML document and appends the items to this list
        @param doc the OPML document to parse
        @return whether parsing was successful or not (TODO: make errors more detailed)
    */
    bool readFromOpml(const QDomDocument& doc);

    /** exports the feed list as OPML. The root node ("All Feeds") is ignored! */
    QDomDocument toOpml() const;

    /** returns a feed object for a given feed URL. If the feed list does not contain a feed with @c url, NULL is returned. If it contains the same feed multiple times, any of the Feed objects is returned. */
    const Feed* findByURL(const QString& feedURL) const;
    Feed* findByURL(const QString& feedURL);

    const Article findArticle(const QString& feedURL, const QString& guid) const;

    int unread() const;

    void addToFetchQueue( FetchQueue* queue, bool intervalOnly=false );
    KJob* createMarkAsReadJob();

signals:

    void signalDestroyed(Akregator::FeedList*);

    /** emitted when a node was added to the list */
    void signalNodeAdded(Akregator::TreeNode*);

    /** emitted when a node was removed from the list */
    void signalNodeRemoved(Akregator::TreeNode*);

    void signalAboutToRemoveNode( Akregator::TreeNode* );

    void signalNodeChanged( Akregator::TreeNode* );

    /** emitted when fetching started */
    void fetchStarted(Akregator::Feed*);

    /** emitted when feed finished fetching */
    void fetched(Akregator::Feed *);

    /** emitted when a fetch error occurred */
    void fetchError(Akregator::Feed *);
    /** emitted when a feed URL was found by auto discovery */
    void fetchDiscovery(Akregator::Feed *);

    /** emitted when a fetch is aborted */
    void fetchAborted(Akregator::Feed *);

    void unreadCountChanged( int unread );

private:

    void addNode(TreeNode* node, bool preserveID);
    void removeNode(TreeNode* node);

    int generateID() const;
    void setRootNode(Folder* folder);

    void parseChildNodes(QDomNode &node, Folder* parent);

private slots:

    void slotNodeDestroyed(Akregator::TreeNode* node);
    void slotNodeAdded(Akregator::TreeNode* node);
    void slotNodeRemoved(Akregator::Folder* parent, Akregator::TreeNode* node);
    void rootNodeChanged();

private:
    friend class AddNodeVisitor;
    class AddNodeVisitor;

    friend class RemoveNodeVisitor;
    class RemoveNodeVisitor;

    class Private;
    Private* const d;
};

} // namespace Akregator

#endif // AKREGATOR_FEEDLIST_H
