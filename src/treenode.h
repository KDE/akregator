//
// C++ Interface: %{MODULE}
//
// Description: 
//
//
// Author: Frank Osterfeld, f_osterf AT informatik.uni-kl.de, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef AKREGATORTREENODE_H
#define AKREGATORTREENODE_H

#include <qobject.h>
#include <qstring.h>

// These will be removed when TreeNodeItem is ready
#include "feedscollection.h"
#include <qlistview.h>

class QDomDocument;
class QDomElement;

namespace Akregator 
{

class ArticleSequence;

/**
    abstract base class for feeds and feed groups 

*/
class TreeNode : public QObject
{
Q_OBJECT
public:
     TreeNode(TreeNode* parent = 0);
     ~TreeNode();
     
     // This will be removed when TreeNodeItem is ready
     TreeNode(QListViewItem *i, FeedsCollection *coll) : m_title(""), m_parent(0), m_item(i), m_collection(coll) {}
    
    
    virtual int unread() const = 0;
    virtual QString title() const;
    virtual void setTitle(QString title);
    virtual TreeNode* parent() const;
    virtual void setParent(TreeNode* parent);
    virtual ArticleSequence articles() = 0; // insert a const version as well
    
    /** helps the rest of the app to decide if node should be handled as group or not. Only use where necessary, use polymorphism where possible **/
    
    virtual bool isGroup() const = 0;
    
    /** exports node and child nodes to OPML (with akregator settings) **/
    
    virtual QDomElement toOPML( QDomElement parent, QDomDocument document ) const = 0;
      
    // These will be removed when TreeNodeItem is ready
    
    QListViewItem* item() { return m_item; } const
    void setItem(QListViewItem* i) { m_item = i; }
    FeedsCollection* collection() { return m_collection;}
    void setCollection(FeedsCollection* c) { m_collection = c; }

    
    public slots:    
    
    // move to PhysicalTreeNode later
    
    virtual void slotDeleteExpiredArticles() = 0;
    virtual void slotMarkAllArticlesAsRead() = 0;
    //virtual void slotFetch(int timeout) = 0;    
    //virtual void slotAbortFetch() = 0;
      
signals:
        
    void signalChanged();
    void signalDestroyed();   
    void signalFetched();
    void signalFetchAborted();
    void signalFetchTimeout();

protected:    
    
    QString m_title;
    TreeNode* m_parent;
    
    // These will be removed when TreeNodeItem is ready
    QListViewItem* m_item;
    FeedsCollection* m_collection;
};

};

#endif
