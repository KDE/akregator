/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
#ifndef AKREGATORFEEDGROUP_H
#define AKREGATORFEEDGROUP_H

#include <qobject.h>

class QListViewItem;
class QDomDocument;
class QDomElement;

namespace Akregator
{
    class ArticleSequence;
    class FeedsCollection;
    
    /**
     * This is a dummy feed used to represent feed groups.
     */
    class FeedGroup : public QObject
    {
        Q_OBJECT
        public:
            FeedGroup(QListViewItem *i, FeedsCollection *coll);
            ~FeedGroup();

            void destroy();

            virtual bool isGroup() const { return true; }

            virtual QDomElement toXml( QDomElement parent, QDomDocument document ) const;

            QString title() const { return m_title; }
            void setTitle(const QString &title);

            QListViewItem *item() { return m_item; }
            void setItem(QListViewItem *i);
            
            FeedsCollection *collection() { return m_collection;}
            void setCollection(FeedsCollection *);
     
            virtual ArticleSequence articles() const;
            
            signals:       
                
            void signalDestroyed();    
            void signalChanged();

        public slots:
            
            virtual void deleteExpiredArticles();                
        private:
            QString           m_title;        ///< Feed/Group title

            QListViewItem    *m_item;         ///< Corresponding list view item.
            FeedsCollection  *m_collection;   ///< Parent collection.
     
    };
};

#endif
