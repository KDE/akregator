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

namespace Akregator
{
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

            virtual bool isGroup() { return true; }

            //virtual void open(QTextStream &ts) {} //?
            virtual void save(QTextStream &/*ts*/, int /*depth*/ = 0) {}

            QString title();
            void setTitle(const QString &title);

            QListViewItem *item() { return m_item; }

        private:
            QString           m_title;        ///< Feed/Group title

            QListViewItem    *m_item;         ///< Corresponding list view item.
            FeedsCollection  *m_collection;   ///< Parent collection.
    };
};

#endif
