/***************************************************************************
 *   Copyright (C) 2004 by Frank Osterfeld                                 *
 *   frank.osterfeld AT kdemail.net                                        *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
#ifndef AKREGATORFEEDLIST_H
#define AKREGATORFEEDLIST_H

#include <qobject.h>
#include <qstring.h>

#include <qmap.h>
#include <qvaluelist.h>

class QDomDocument;
class QDomNode;

namespace Akregator
{

class FeedGroup;
class TreeNode;

/** The model of a feed tree, represents an OPML document. Contains an additional root node "All Feeds" which isn't stored. Note that a node instance must not be in more than one FeedList at a time! When deleting the feed list, all contained nodes are deleted! */

class FeedList : public QObject
{
Q_OBJECT
public:

    /** creates a new FeedList from an OPML document, adds the "All Feeds" node.
        @param doc the OPML document to parse
        @return the parsed feed list, or @c NULL when a parsing error occured
    */
    static FeedList* fromOPML(const QDomDocument& doc);

    FeedList(QObject *parent = 0, const char *name = 0);

    /** Destructor. Contained nodes are deleted! */
    ~FeedList();

    /** returns the root node ("All Feeds"). */
    FeedGroup* rootNode() const;

    /** appends another feed list as sub tree. The root node of @c list is ignored. NOTE: nodes are _moved_ from @c list to this feed list, not copied */
    
    void append(FeedList* list, FeedGroup* parent=0, TreeNode* after=0);

    /** returns the title of the feed list (as used in the OPML document) */
    const QString& title() const;

    /** sets the title of the feed list */
    void setTitle(const QString& name);

    /** exports the feed list as OPML. The root node ("All Feeds") is ignored! */
    QDomDocument toOPML() const;

    TreeNode* findByID(uint id) const;

    /** returns whether the feed list is empty, root node is ignored */
    bool isEmpty() const;

signals:
    void signalDestroyed(FeedList*);
    
protected:

    /** connects a node's notification signals to the slots below. Used for Observer mechanism 
    @param node the node to connect to
    */
    void connectToNode(TreeNode* node);
    /** disconnects from a node's notification signals. @c node is not longer observed.
    @param the node to disconnect from
     */
    void disconnectFromNode(TreeNode* node);

protected slots:

    void slotNodeAdded(TreeNode* node);
//    void slotNodeChanged(TreeNode* node);
    void slotNodeDestroyed(TreeNode* node);
    void slotNodeRemoved(FeedGroup* parent, TreeNode* node);
    
private:

    static void parseChildNodes(QDomNode &node, FeedGroup* parent);
    
    // never call these
    FeedList(const FeedList&) : QObject() {}
    FeedList& operator=(const FeedList&) { return *this; }
    
    uint m_idCounter;
    QMap<uint, TreeNode*> m_idMap;
    QValueList<TreeNode*> m_flatList;
    FeedGroup* m_rootNode;
    QString m_title;
};

}

#endif
