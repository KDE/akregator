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
    class ArticleFilter;
    class ArticleList;
    class ArticleListItem;
    class ArticleViewer;
    class FeedGroup;
    class FeedGroupItem;
    class FeedList;
    class FeedsTree;
    class FetchTransaction;
    class Frame;
    class Part;
    class TabWidget;
    class TreeNodeItem;

    /**
     * This is the main widget of the view, containing tree view, article list, viewer etc.
     */
    class View : public QWidget
    {
        Q_OBJECT
        public:

            /** constructor
            @param part the Akregator::Part which contains this widget
            @param parent parent widget
            @param name the name of the widget (@ref QWidget )
            */
            View(Akregator::Part *part, QWidget *parent, const char* name);

            /** destructor.  Note that cleanups should be done in
            slotOnShutdown(), so we don't risk accessing self-deleting objects after deletion. */
            ~View();

            /** saves settings. Make sure that the Settings singleton is not destroyed yet when saveSettings is called */
            void saveSettings();

            /** Adds the feeds in @c doc to the "Imported Folder"
            @param doc the DOM tree (OPML) of the feeds to import */
            bool importFeeds(const QDomDocument& doc);
            
            /** Parse OPML presentation of feeds and read in articles archive, if present. If @c parent is @c NULL, the current
            feed list is replaced by the parsed one
             @param doc QDomDocument generated from OPML
             @param parent The parent group the new nodes */
            bool loadFeeds(const QDomDocument& doc, FeedGroup* parent = 0);

            /**
             @return the displayed Feed List in OPML format
             */
             QDomDocument feedListToOPML();

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

            void slotOnShutdown();
            
             /** selected tree node has changed */
            void slotNodeSelected(TreeNode* node);

            /** the article selection has changed */
            void slotArticleSelected(MyArticle article);
            
            /** Shows requested popup menu for article list */
            void slotArticleListContextMenu(KListView*, ArticleListItem* item, const QPoint& p);
            
            /** Shows requested popup menu for feed tree */
            void slotFeedTreeContextMenu(KListView*, TreeNodeItem*, const QPoint&);

            /** emits @ref signalUnreadCountChanged(int) */
            void slotSetTotalUnread();

            /** special behaviour in article list view (TODO: move code there?) */
            void slotMouseButtonPressed(int button, QListViewItem * item, const QPoint & pos, int c);

            /** opens article of @c item in external browser */
            void slotOpenArticleExternal(ArticleListItem* item, const QPoint&, int);

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

            void connectToFeedList(FeedList* feedList);
            void disconnectFromFeedList(FeedList* feedList);
            
        protected slots:

            /** this is called by the ctor, does init steps which need a properly created view and part */
            
            void delayedInit();
            
            void slotActivateSearch();
                    
            void connectFrame(Frame *);

            void slotRemoveFrame();
            
            QString getTitleNodeText(const QDomDocument &doc);

       	    void setTabIcon(const QPixmap&);

            void addFeed(const QString& url, TreeNode* after, FeedGroup* parent, bool autoExec = true);

            void updateSearch(const QString &s=QString::null);

            void showFetchStatus();

            /** Display article in external browser. */
            void displayInExternalBrowser(const KURL &url);

            void slotDoIntervalFetches();
            void slotDeleteExpiredArticles();
        private:

            enum ViewMode { NormalView=0, WidescreenView, CombinedView };  

            /** the model of the feed tree */
            FeedList* m_feedList;
            /**
             * A tree of all feeds (Columns, Subscriptions).
             */
            FeedsTree* m_tree;

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
            Akregator::Part *m_part;
            ArticleFilter *m_currentTextFilter;
            ArticleFilter *m_currentStatusFilter;
            ViewMode m_viewMode;
            
            QTimer *m_fetchTimer;
            QTimer* m_expiryTimer;

            bool m_shuttingDown;

            QPixmap m_keepFlagIcon;
    };
}

#endif // _AKREGATORVIEW_H_
