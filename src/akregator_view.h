/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny, Sashmit Bhaduri            *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *   smt@vfemail.net (Sashmit Bhaduri)                                     *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef _AKREGATORVIEW_H_
#define _AKREGATORVIEW_H_

#include <qpixmap.h>
#include <qwidget.h>

#include <kurl.h>

#include "feed.h"

class QSplitter;
class QDomDocument;
class QDomElement;
class QHBox;
class QToolButton;
class QListViewItem;
class KComboBox;
class KConfig;
class KFileItem;
class KLineEdit;
class KListView;
class KListViewItem;

namespace Akregator
{
    class aKregatorPart;
    class FetchTransaction;
    class FeedsTree;
    class FeedGroup;
    class FeedGroupItem;
    class ArticleList;
    class ArticleListItem;
    class ArticleViewer;
    class ArticleFilter;
    class TabWidget;
    class TreeNodeItem;
    class Frame;

    /**
     * This is the main widget of the view, containing tree view, article list, viewer etc.
     */
    class aKregatorView : public QWidget
    {
        Q_OBJECT
        public:

            /** constructor
            @param part the aKregatorPart which contains this widget
            @param parent parent widget
            @param name the name of the widget (@ref QWidget )
            */
            aKregatorView(aKregatorPart *part, QWidget *parent, const char* name);
            ~aKregatorView();

            /** saves settings. Make sure that the Settings singleton is not destroyed yet when saveSettings is called */
            void saveSettings();

            /** Adds the feeds in @c doc to the "Imported Folder"
            @param doc the DOM tree (OPML) of the feeds to import */
            bool importFeeds(const QDomDocument& doc);
            
            /** Parse OPML presentation of feeds and read in articles archive, if present.
             @param doc QDomDocument generated from OPML
             @param parent The parent group the new nodes */
            bool loadFeeds(const QDomDocument& doc, FeedGroup* parent = 0);

            /**
             Store whole feeds tree to given node @c node of document @c document.
             @param node Node to save to.
             @param document Containing document.
             */
            void storeTree( QDomElement &node, QDomDocument &document );

            /**
             Add a feed to a group.
             @param url The URL of the feed to add.
             @param group The name of the folder into which the feed is added.
             If the group does not exist, it is created.  The feed is added as the last member of the group.
             */
            void addFeedToGroup(const QString& url, const QString& group);

            /** Disables fetch actions and informs the frame to enable stop button */
            void startOperation();
            
            /** Enables fetch actions and informs the frame to disable stop button */
            void endOperation();
            
            /** stops a running operation and resets progress bar, re-enables actions etc. */
            void operationError(/*const QString &msg*/);

            /** stops the loading of the feed list */
            void stopLoading();

            /** returns the fetch transaction object */
            FetchTransaction* transaction() { return m_transaction; }

            /** session management **/
            virtual void readProperties(KConfig* config);
            virtual void saveProperties(KConfig* config);

	    Frame* currentFrame() const { return m_currentFrame; }
             
        signals:
            /** emitted when the unread count of "All Feeds" was changed */
            void signalUnreadCountChanged(int);

        public slots:
            
             /** selected tree node has changed */
            void slotNodeSelected(TreeNode* node);

            /** the article selection has changed */
            void slotArticleSelected(MyArticle article);
            
            /** Shows requested popup menu for article list */
            void slotArticleListContextMenu(KListView*, QListViewItem* item, const QPoint& p);
            
            /** Shows requested popup menu for feed tree */
            void slotFeedTreeContextMenu(KListView*, QListViewItem*, const QPoint&);

            /** emits @ref signalUnreadCountChanged(int) TODO: remove this) */
            void setTotalUnread();

            /** special behaviour in article list view (TODO: move code there?) */
            void slotMouseButtonPressed(int button, QListViewItem * item, const QPoint & pos, int c);

            /** opens article of @c item in external browser */
            void slotOpenArticleExternal(QListViewItem* item, const QPoint&, int);

            /** opens the current article (currentItem) in external browser
            TODO: use selected instead of current? */
            void slotOpenCurrentArticleExternal();
            
            /** opens the current article (currentItem) in background tab
            TODO: use selected instead of current? */
            void slotOpenCurrentArticleBackgroundTab();

            /** opens current article in new tab, background/foreground depends on settings TODO: use selected instead of current? */
            void slotOpenCurrentArticle();

            /** opens a page viewer in a new tab and loads an URL
             @param url the url to load
             @param background whether the tab should be opened in the background or in the foreground (activated after creation) */
            void slotOpenTab(const KURL& url, bool background = false);

            /** called when another part/frame is activated. Updates progress bar, caption etc. accordingly
            @param f the activated frame */
            void slotFrameChanged(Frame *f);

            /** sets the caption of a tab (for page viewers) */
            void slotTabCaption(const QString &capt);
            
            /** sets the window caption after a frame change */
            void slotCaptionChanged(const QString &);

            /** called when URLs are dropped into the tree view */
            void slotFeedURLDropped (KURL::List &urls, TreeNodeItem* after, FeedGroupItem *parent);
     
            void slotSearchComboChanged(int index);
            void slotSearchTextChanged(const QString &search);

            /** displays a URL in the status bar when the user moves the mouse over a link */
            void slotMouseOverInfo(const KFileItem *kifi);
            
            /** sets the status bar text to a given string */
	        void slotStatusText(const QString &);
            
            void slotStarted();
            void slotCanceled(const QString &);
            void slotCompleted();
            void slotLoadingProgress(int);

            
            void slotFetchesCompleted();
           
            /** Feed has been fetched, populate article view if needed and update counters. */
            void slotFeedFetched(Feed *);
            void slotFeedFetchError(Feed *feed);


            /** prints the content of the article viewer */
            void slotPrint();
            /** adds a new feed to the feed tree */
            void slotFeedAdd();
            /** adds a feed group to the feed tree */
            void slotFeedAddGroup();
            /** removes the currently selected feed (ask for confirmation)*/
            void slotFeedRemove();
            /** calls the properties dialog for feeds, starts renaming for feed groups */
            void slotFeedModify();
            /** fetches the currently selected feed */
            void slotFetchCurrentFeed();
            /** starts fetching of all feeds in the tree */
            void slotFetchAllFeeds();
            /** marks all articles in the currently selected feed as read */
            void slotMarkAllRead();
            /** marks all articles in all feeds in the tree as read */
            void slotMarkAllFeedsRead();
            /** opens the homepage of the currently selected feed */
            void slotOpenHomepage();
            
            /** toggles the keep flag of the currently selected article */
            void slotArticleToggleKeepFlag();
            /** deletes the currently selected article */
            void slotArticleDelete();
            /** marks the currently selected article as unread */
            void slotSetSelectedArticleUnread();
            /** marks the currently selected article as new */
            void slotSetSelectedArticleNew();

            /** switches view mode to normal view */
            void slotNormalView();
            /** switches view mode to widescreen view */
            void slotWidescreenView();
            /** switches view mode to combined view */
            void slotCombinedView();
            /** toggles the visibility of the filter bar */
            void slotToggleShowQuickFilter();

            /** scrolls the article viewer up*/
            void slotScrollViewerUp();
            /** scrolls the article viewer down */
            void slotScrollViewerDown();

            /** selects the previous article in the article list */
            void slotPreviousArticle();
            /** selects the next article in the article list */
            void slotNextArticle();
            /** selects the previous unread article in the article list */
            void slotPrevUnreadArticle();
            /** selects the next unread article in the article list */
            void slotNextUnreadArticle();
            /** selects the previous (pre-order) feed with unread articles in the tree view */
            void slotPrevUnreadFeed();
            /** selects the next (pre-order) feed with unread articles in the tree view */
            void slotNextUnreadFeed();
            /** selects the previous (pre-order) feed in the tree view */
            void slotPrevFeed();
            /** selects the next (pre-order) feed in the tree view */
            void slotNextFeed();

            void slotFeedsTreeUp();
            void slotFeedsTreeDown();
            void slotFeedsTreeLeft();
            void slotFeedsTreeRight();
            void slotMoveCurrentNodeUp();
            void slotMoveCurrentNodeDown();
            void slotMoveCurrentNodeLeft();
            void slotMoveCurrentNodeRight();
            void slotFeedsTreePageUp();
            void slotFeedsTreePageDown();
            void slotFeedsTreeHome();
            void slotFeedsTreeEnd();


        protected:

            /**
             Recursively parse child nodes of the opml document, building feeds tree.
             @param node Current node to parse.
             @param parent Parent listview item for newly created listview items.
             */
            void parseChildNodes(QDomNode &node, FeedGroup* parent = 0);

        protected slots:
            
            void slotActivateSearch();
                    
            void connectFrame(Frame *);

            void slotRemoveFrame();
            
            QString getTitleNodeText(const QDomDocument &doc);

            /**
             * Write child items of item to node using QDom document document.
             * @param item Current feeds tree item which is being written.
             * @param node Parent node to append to.
             * @param document XML document used to create nodes.
             */
            void writeChildNodes( TreeNode* node, QDomElement& element, QDomDocument &document );

            void addFeed(const QString& url, TreeNode* after, FeedGroup* parent, bool autoExec = true);

            void updateSearch(const QString &s=QString::null);

            void showFetchStatus();

            /** Display article in external browser. */
            void displayInExternalBrowser(const KURL &url);

            void slotDoIntervalFetches();
            void slotDeleteExpiredArticles();
        private:

            enum ViewMode { NormalView=0, WidescreenView, CombinedView };  
    
            /**
             * A tree of all feeds (Columns, Subscriptions).
             */
            FeedsTree *m_tree;

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

            KComboBox *m_searchCombo;
            KLineEdit *m_searchLine;
            QHBox* m_searchBar;
            
            int m_queuedSearches;
            QString m_queuedSearch;
            FetchTransaction *m_transaction;
            
            QSplitter *m_feedSplitter, *m_articleSplitter;
            aKregatorPart *m_part;
            ArticleFilter *m_currentTextFilter;
            ArticleFilter *m_currentStatusFilter;
            ViewMode m_viewMode;
            
            QTimer *m_fetchTimer;
            QTimer* m_expiryTimer;

            bool m_stopLoading;
            bool m_shuttingDown;

            QPixmap m_keepFlagIcon;
    };
}

#endif // _AKREGATORVIEW_H_
