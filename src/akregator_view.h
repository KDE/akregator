/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny, Sashmit Bhaduri            *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *   smt@vfemail.net (Sashmit Bhaduri)                                     *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef _AKREGATORVIEW_H_
#define _AKREGATORVIEW_H_

#include "feed.h"
#include "feedscollection.h"

#include <qwidget.h>
#include <kurl.h>


class QSplitter;
class QDomDocument;
class QDomElement;
class QToolButton;
class QListViewItem;
class KListViewItem;
class KListView;
class KFileItem;
class KComboBox;
class KLineEdit;

namespace Akregator
{
    class aKregatorPart;
    class FeedsTree;
    class FeedsTreeViewItem;
    class ArticleList;
    class ArticleListItem;
    class ArticleViewer;
    class ArticleFilter;
    class TabWidget;
}

namespace Akregator
{
    /**
     * This is a RSS Aggregator "Part". It does all the real work.
     * It is also embeddable into other applications (e.g. for use in Kontact).
     */
    class aKregatorView : public QWidget
    {
        Q_OBJECT
        public:
            enum ViewMode { NormalView=0, WidescreenView, CombinedView };

            /**
             * Default constructor.
             */
            aKregatorView(aKregatorPart *part, QWidget *parent, const char *wName);

            void saveSettings(bool quit);

            bool importFeeds(const QDomDocument& doc);

            /**
             * Parse OPML presentation of feeds and read in articles archive, if present.
             * @param doc QDomDocument generated from OPML by openFile().
             */
            bool loadFeeds(const QDomDocument& doc, QListViewItem *parent = 0);

            /**
             * Recursively parse child nodes of the opml document, building feeds tree.
             * @param node Current node to parse.
             * @param parent Parent listview item for newly created listview items.
             */
            void parseChildNodes(QDomNode &node, QListViewItem *parent = 0);


            /**
             * Store whole feeds tree to given node @c node of document @c document.
             * @param node Node to save to.
             * @param document Containing document.
             */
            void storeTree( QDomElement &node, QDomDocument &document );

            bool event(QEvent *e);

        public slots:
            /**
             * Current item has changed.
             */
            void slotItemChanged(QListViewItem*);
            void slotItemMoved();

            void slotNormalView();
            void slotWidescreenView();
            void slotCombinedView();

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
            void slotUpdateArticleList(FeedGroup *, bool onlyUpdateNew=false);
            void slotUpdateArticleList(Feed *, bool clear=true, bool onlyUpdateNew=false);

            /**
             * Display article contents in ArticleViewer.
             */
            
            void slotMouseButtonPressed(int button, QListViewItem * item, const QPoint & pos, int c);
            void slotArticleSelected(QListViewItem *);
            void slotArticleDoubleClicked(QListViewItem *, const QPoint &, int);

            /**
             * This slot is called when user renames a feed in feeds tree.
             */
            void slotItemRenamed( QListViewItem * item ); //, const QString & text, int col

            void slotFeedAdd();
            void slotFeedAddGroup();
            void slotFeedRemove();
            void slotFeedModify();
            void slotMarkAllRead();
            void slotFetchCurrentFeed();
            void slotFetchAllFeeds();
            void slotFeedURLDropped (KURL::List &urls, QListViewItem *after, QListViewItem *parent);

            void slotSearchComboChanged(int index);
            void slotSearchTextChanged(const QString &search);
            void activateSearch();

            void slotMouseOverInfo(const KFileItem *kifi);

            void slotOpenTab(const KURL& url);
            void slotRemoveTab();
            void slotTabChanged(QWidget *w);
            void slotTabCaption(const QString &capt);

        private:
            /**
             * Reset to default values, removing all existing data.
             * @internal
             */
            void reset();

            /**
             * Write child items of item to node using QDom document document.
             * @param item Current feeds tree item which is being written.
             * @param node Parent node to append to.
             * @param document XML document used to create nodes.
             */
            void writeChildNodes( QListViewItem *item, QDomElement &node, QDomDocument &document );

            void addFeed(QString url, QListViewItem *after, QListViewItem* parent);
            /**
             * Add feed to feeds tree, given the existing list view item and alot of
             * feed parameters. FIXME: parameters better be bundled to FeedData?
             * @return Newly created feed.
             */
            Feed *aKregatorView::addFeed_Internal(Feed *f, QListViewItem *elt, QString title,
                                                QString xmlUrl, QString htmlUrl,
                                                QString description,
                                                bool updateTitle);

            /**
             * Mark all items in item and subitems as read.
             * @param item Item to start marking at.
             */
            void markAllRead(QListViewItem *item);

            void updateSearch(const QString &s=QString::null);
            void checkItem(QListViewItem *i);
            bool itemMatches (ArticleListItem *item);
            void itemAdded(ArticleListItem *item);

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

            /**
             * A tab widget for multiple viewers.
             */
            TabWidget *m_tabs;
            QToolButton *m_tabsClose;
            QWidget *m_mainTab;

            KComboBox *m_searchCombo;
            KLineEdit *m_searchLine;
            int m_queuedSearches;
            QString m_queuedSearch;

            QSplitter *m_panner1, *m_panner2;
            aKregatorPart *m_part;
            ArticleFilter *m_currentTextFilter;
            ArticleFilter *m_currentStatusFilter;
            ViewMode m_viewMode;
    };
}

#endif // _AKREGATORVIEW_H_
