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

/** The model of a feed tree, represents an OPML document. Contains an additional root node "All Feeds" which isn't stored. */
class FeedList : public QObject
{
Q_OBJECT
public:

    /** creates a new FeedList from an OPML document, adds the "All Feeds" node.
        @param doc the OPML document to parse
        @return the parsed feed list, or 0 when a parsing error occured
    */
    static FeedList* fromOPML(const QDomDocument& doc);
    
    FeedList(QObject *parent = 0, const char *name = 0);

    /** Destructor. Contained nodes are NOT deleted. Use @c delete @c rootNode() to delete the tree. FIXME: change that later */
    ~FeedList();

    /** returns the root node "All Feeds". */
    FeedGroup* rootNode() const;

    /** appends another feed list as sub tree. The root node of @c list is ignored. NOTE: nodes are _moved_ from @c list to this feed list (should be copied actually) */
    
    void append(FeedList* list, FeedGroup* parent=0, TreeNode* after=0);

    const QString& title() const;
    
    void setTitle(const QString& name);

    /** exports the feed list as OPML. The root node is ignored! */
    QDomDocument toOPML() const;

//    TreeNode* findByID(uint id) const;
//  void setIDCounter(uint idCounter);
//  uint idCounter();

protected:
    
    void connectToNode(TreeNode* node);
    void disconnectFromNode(TreeNode* node);

protected slots:

    void slotNodeAdded(TreeNode* node);
//    void slotNodeChanged(TreeNode* node);
    void slotNodeDestroyed(TreeNode* node);
    void slotNodeRemoved(FeedGroup* parent, TreeNode* node);
    
private:

    static void parseChildNodes(QDomNode &node, FeedGroup* parent);
    
    // never call these
    FeedList(const FeedList& b) : QObject() {}
    FeedList& operator=(const FeedList& f) { return *this; }
    
//    uint m_idCounter;
//    QMap<uint, TreeNode*> m_idMap;
    QValueList<TreeNode*> m_flatList;
    FeedGroup* m_rootNode;
    QString m_title;
};

};

#endif
