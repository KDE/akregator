/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef _AKREGATORPART_H_
#define _AKREGATORPART_H_

#include "feed.h"
#include "feedscollection.h"
#include <kparts/part.h>
#include <kaboutdata.h>

class QWidget;
class QSplitter;
class QDomDocument;
class QDomElement;
class QListViewItem;
class KListViewItem;
class KListView;

namespace Akregator
{
    class FeedsTree;
    class ArticleList;
    class ArticleViewer;
}

namespace Akregator
{
    /**
     * This is a RSS Aggregator "Part". It does all the real work.
     * It is also embeddable into other applications (e.g. for use in Kontact).
     */
    class aKregatorPart : public KParts::ReadWritePart
    {
        Q_OBJECT
        public:
            /**
             * Default constructor.
             */
            aKregatorPart(QWidget *parentWidget, const char *widgetName,
                          QObject *parent, const char *name, const QStringList&);

            /**
             * Destructor.
             */
            virtual ~aKregatorPart();

            /**
             * This is a virtual function inherited from KParts::ReadWritePart.
             * A shell will use this to inform this Part if it should act
             * read-only
             */
            virtual void setReadWrite(bool rw);

            /**
             * Reimplemented to disable and enable Save action
             */
            virtual void setModified(bool modified);


            /**
             * Create KAboutData for this KPart.
             */
            static KAboutData *createAboutData();

        protected:
            /**
             * This must be implemented by each part
             */
            virtual bool openFile();

            /**
             * Parse OPML presentation of feeds and read in articles archive, if present.
             * @param doc QDomDocument generated from OPML by openFile().
             */
            bool loadFeeds(const QDomDocument& doc);

            /**
             * Recursively parse child nodes of the opml document, building feeds tree.
             * @param node Current node to parse.
             * @param parent Parent listview item for newly created listview items.
             */
            void parseChildNodes(QDomNode &node, KListViewItem *parent = 0);

            /**
             * This must be implemented by each read-write part
             */
            virtual bool saveFile();

            /**
             * Write child items of item to node using QDom document document.
             * @param item Current feeds tree item which is being written.
             * @param node Parent node to append to.
             * @param document XML document used to create nodes.
             */
            void writeChildNodes( QListViewItem *item, QDomElement &node, QDomDocument &document );

        protected slots:
            /**
             * Current item has changed.
             */
            void slotItemChanged(QListViewItem*);

            /**
             * Shows requested popup menu
             */
            void slotContextMenu(KListView*, QListViewItem*, const QPoint&);

            /**
             * Feed has been fetched, populate article view if needed and update counters.
             */
            void slotFeedFetched(Feed *);

            /**
             * Populate article view with articles from selected feed.
             */
            void slotUpdateArticleList(Feed *);

            /**
             * Display article contents in ArticleViewer.
             */
            void slotArticleSelected(QListViewItem *);

            /**
             * This slot is called when user renames a feed in feeds tree.
             */
            void slotItemRenamed( QListViewItem * item ); //, const QString & text, int col

            void slotFeedAdd();
            void slotFeedAddGroup();
            void slotFeedRemove();
            void slotFeedModify();
            void slotFeedCopy();
            void slotFetchCurrentFeed();
            void slotFetchAllFeeds();

            void fileOpen();
            void fileSaveAs();

        private:
            /**
             * Reset to default values, removing all existing data.
             * @internal
             */
            void reset();

            /**
             * Add feed to feeds tree, given the existing list view item and alot of
             * feed parameters. FIXME: parameters better be bundled to FeedData?
             * @return Newly created feed.
             */
            Feed *aKregatorPart::addFeed_Internal(QListViewItem *elt, QString title,
                                                QString xmlUrl, QString htmlUrl,
                                                QString description, bool isLiveJournal,
                                                QString ljUserName,
                                                Feed::LJAuthMode ljAuthMode,
                                                QString ljLogin, QString ljPassword,
                                                bool updateTitle);


            /**
             * A tree of all feeds (Columns, Subscriptions).
             */
            FeedsTree *m_tree;

            /**
             * An internal collection of Feed items.
             */
            FeedsCollection m_feeds;

            /**
             * List of documents for currently selected feed.
             */
            ArticleList *m_articles;

            /**
             * Currently selected article renderer.
             */
            ArticleViewer *m_articleViewer;

            QSplitter *m_panner1, *m_panner2;
            QValueList<int> m_panner1Sep, m_panner2Sep;

            static KAboutData* s_about;
    };
}

#endif // _AKREGATORPART_H_
