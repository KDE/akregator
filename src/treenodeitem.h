/***************************************************************************
 *   Copyright (C) 2004 by Frank Osterfeld                                 *
 *   frank.osterfeld AT kdemail.net                                      *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef AKREGATORTREENODEITEM_H
#define AKREGATORTREENODEITEM_H

#include <klistview.h>


namespace Akregator {

class FeedGroupItem;
class TreeNode;
/**
    abstract base class for all items in the feeds tree
*/
class TreeNodeItem : public KListViewItem
{
    
public:
     
    TreeNodeItem(FeedGroupItem* parent, TreeNode* node);
    TreeNodeItem(FeedGroupItem* parent, TreeNodeItem* after, TreeNode* node);
    TreeNodeItem(KListView* parent, TreeNode* node);
    ~TreeNodeItem();
    virtual TreeNode* node();
    
    virtual void showContextMenu() = 0; 
    virtual void showEditProperties() = 0;
    virtual void showAskDelete() = 0;
    virtual void nodeChanged();
 
    virtual TreeNodeItem* firstChild() const; 
    virtual TreeNodeItem* nextSibling() const;
    virtual FeedGroupItem* parent() const;

    protected:
    
    TreeNode* m_node;
    
    virtual void paintCell( QPainter * p, const QColorGroup & cg,
                            int column, int width, int align );

    private:
        void updateParentsRecursive(); //FIXME: remove
};

};

#endif
