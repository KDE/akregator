/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
#ifndef AKREGATORFEEDGROUP_H
#define AKREGATORFEEDGROUP_H

#include "treenode.h"

#include "qptrdict.h"

class QListViewItem;
class QDomDocument;
class QDomElement;

namespace Akregator
{
    
    class ArticleSequence;
    class FeedsCollection;
    
    /**
     * This represents feed groups
     */
    class FeedGroup : public TreeNode
    {
        Q_OBJECT
        public:
            FeedGroup(QListViewItem *i, FeedsCollection *coll);
            ~FeedGroup();

            virtual ArticleSequence articles();
            virtual int unread() const;
                       
            /** helps the rest of the app to decide if node should be handled as group or not. Use only where necessary, use polymorphism where possible **/
            virtual bool isGroup() const { return true; }
            
            virtual QDomElement toOPML( QDomElement parent, QDomDocument document ) const;

            virtual QPtrDict<TreeNode> children() { return m_children; }
        public slots:
            
            virtual void slotDeleteExpiredArticles();                
            virtual void slotMarkAllArticlesAsRead();
            virtual void slotChildChanged();
            //virtual void slotFetch(int timeout);    
            //virtual void slotAbortFetch();
    
        protected:
            QPtrDict<TreeNode> m_children;
    };
};

#endif
