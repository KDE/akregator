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
#include <qpixmap.h>
#include <kurl.h>
#include <qtimer.h>


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
    class FetchTransaction;
    class FeedsTree;
    class FeedsTreeViewItem;
    class ArticleList;
    class ArticleListItem;
    class ArticleViewer;
    class ArticleFilter;
    class TabWidget;
    class Frame;
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

            /**
             * Add a feed to a group.
             * @param url The URL of the feed to add.
             * @param group The name of the folder into which the feed is added.
             *
             * If the group does not exist, it is created.  The feed is added as the last member
             * of the group.
             */
            void addFeedToGroup(const QString& url, const QString& group);

	    void startOperation();
            void endOperation();
            void operationError(const QString &msg);

            void stopLoading();

	    FetchTransaction* transaction(){return m_transaction;}

            /** session management **/
            virtual void readProperties(KConfig* config);
            virtual void saveProperties(KConfig* config);
            
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

	    void slotFetchesCompleted();
            /**
//              * Feed has been fetched, populate article view if needed and update counters.
             */
            void slotFeedFetched(Feed *);
            void slotFeedFetchError(Feed *feed);

            /**
             * Populate article view with articles from selected feed.
             */
            void slotUpdateArticleList(FeedGroup *, bool);
            void slotUpdateArticleList(Feed *, bool);

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
            void slotNextUnread();
            void slotMarkAllRead();
            void slotMarkAllFeedsRead();

            void slotFetchCurrentFeed();
            void slotFetchAllFeeds();
            void slotFeedURLDropped (KURL::List &urls, QListViewItem *after, QListViewItem *parent);
            void slotOpenHomepage();

            void slotSearchComboChanged(int index);
            void slotSearchTextChanged(const QString &search);
            void activateSearch();

            void slotMouseOverInfo(const KFileItem *kifi);

            void slotOpenTab(const KURL& url, bool background);
            void slotRemoveFrame();
            void slotFrameChanged(Frame *f);

	    void slotStatusText(const QString &);
            void slotCaptionChanged(const QString &);
            void slotStarted();
            void slotCanceled(const QString &);
            void slotCompleted();
            void slotLoadingProgress(int);

	    void slotTabCaption(const QString &capt);

            void slotDoIntervalFetches();
            void slotDeleteExpiredArticles();

        private:
            /**
             * Reset to default values, removing all existing data.
             * @internal
             */
            void reset();

	    void connectFrame(Frame *);

	    QString getTitleNodeText(const QDomDocument &doc);

            /**
             * Write child items of item to node using QDom document document.
             * @param item Current feeds tree item which is being written.
             * @param node Parent node to append to.
             * @param document XML document used to create nodes.
             */
            void writeChildNodes( QListViewItem *item, QDomElement &node, QDomDocument &document );

            void addFeed(QString url, QListViewItem *after, QListViewItem* parent, bool autoExec = false);
            /**
             * Add feed to feeds tree, given the existing list view item and alot of
             * feed parameters. FIXME: parameters better be bundled to FeedData?
             * @return Newly created feed.
             */
            Feed *aKregatorView::addFeed_Internal(Feed *f, QListViewItem *elt, QString title,
                                                QString xmlUrl, QString htmlUrl,
                                                QString description,
                                                bool autoFetch, int fetchIntervL,
         					Feed::ArchiveMode mode,
                                                int maxArticleAge,
                                                int maxArticleNumber);

            /**
             * Mark all items in item and subitems as read.
             * @param item Item to start marking at.
             */
            void markAllRead(QListViewItem *item);

            void fetchItem(QListViewItem *item);

            void updateSearch(const QString &s=QString::null);
            void checkItem(QListViewItem *i);
            bool itemMatches (ArticleListItem *item);
            bool itemAdded(ArticleListItem *item);

            void showFetchStatus();

            /**
             * Display article in external browser.
             */
            void displayInExternalBrowser(const KURL &url);

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
            Frame *m_mainFrame;
            Frame *m_currentFrame;

            void setTotalUnread();

            KComboBox *m_searchCombo;
            KLineEdit *m_searchLine;
            int m_queuedSearches;
            QString m_queuedSearch;
            FetchTransaction *m_transaction;
            
            QSplitter *m_feedSplitter, *m_articleSplitter;
            aKregatorPart *m_part;
            ArticleFilter *m_currentTextFilter;
            ArticleFilter *m_currentStatusFilter;
            ViewMode m_viewMode;
            QTimer *m_globalFetchTimer;
            QTimer *m_fetchTimer;
            QTimer* m_expiryTimer;

            bool m_stopLoading;

            QPixmap m_feedTreePixmap;
            QPixmap m_folderTreePixmap;
            QPixmap m_errorTreePixmap;
    };
}

#endif // _AKREGATORVIEW_H_
