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

#include <qwidget.h>
#include <qpixmap.h>
#include <kurl.h>

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
            enum ViewMode { NormalView=0, WidescreenView, CombinedView };

            /**
             * Default constructor.
             */
            aKregatorView(aKregatorPart *part, QWidget *parent, const char *wName);
            ~aKregatorView();
            void saveSettings(bool quit);

            bool importFeeds(const QDomDocument& doc);
            
                      
            /**
             * Parse OPML presentation of feeds and read in articles archive, if present.
             * @param doc QDomDocument generated from OPML by openFile().
             */
            bool loadFeeds(const QDomDocument& doc, FeedGroup* parent = 0);

            /**
             * Recursively parse child nodes of the opml document, building feeds tree.
             * @param node Current node to parse.
             * @param parent Parent listview item for newly created listview items.
             */
            void parseChildNodes(QDomNode &node, FeedGroup* parent = 0);


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

            /**
             * Disables fetch actions and informs the frame to enable stop button
             */
            void startOperation();
            /**
             * Enables fetch actions and nforms the frame to disable stop button
             */
            void endOperation();
            void operationError(const QString &msg);

            void stopLoading();

            FetchTransaction* transaction() { return m_transaction; }

            /** session management **/
            virtual void readProperties(KConfig* config);
            virtual void saveProperties(KConfig* config);
            
        public slots:
            
             /**
             * Current item has changed.
             */
            void slotNodeSelected(TreeNode* node);
            void slotItemMoved();
            
            void slotNormalView();
            void slotWidescreenView();
            void slotCombinedView();
            void slotToggleShowQuickFilter();

            /** toggles the keep flag of the currently selected article */
            void slotArticleToggleKeepFlag();
                    
                     
            /**
             * Shows requested popup menu for feed tree
             */
            void slotFeedTreeContextMenu(KListView*, QListViewItem*, const QPoint&);

            /**
             * Shows requested popup menu for article list
             */
            void slotArticleListContextMenu(KListView*, QListViewItem* item, const QPoint& p);

	       void slotFetchesCompleted();
            /**
//              * Feed has been fetched, populate article view if needed and update counters.
             */
            void slotFeedFetched(Feed *);
            void slotFeedFetchError(Feed *feed);

            /**
             * Display article contents in ArticleViewer.
             */

            void slotMouseButtonPressed(int button, QListViewItem * item, const QPoint & pos, int c);
            void slotArticleSelected(MyArticle article);
            
            void slotOpenArticleExternal(QListViewItem* item, const QPoint&, int);
            void slotOpenCurrentArticleExternal();
            void slotOpenCurrentArticleBackgroundTab();
            void slotOpenCurrentArticleForegroundTab();

            /**
             * This slot is called when user renames a feed in feeds tree.
             */
            void slotFeedAdd();
            void slotFeedAddGroup();
            void slotFeedRemove();
            void slotFeedModify();
            void slotPrevUnreadFeed();
            void slotNextUnreadFeed();
            void slotPrevFeed();
            void slotNextFeed();
            void slotPrevUnreadArticle();
            void slotNextUnreadArticle();

            void slotMarkAllRead();
            void slotMarkAllFeedsRead();

            void slotFetchCurrentFeed();
            void slotFetchAllFeeds();
            void slotFeedURLDropped (KURL::List &urls, TreeNodeItem* after, FeedGroupItem *parent);
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

            void slotPreviousArticle();
            void slotNextArticle();
            void slotScrollViewerUp();
            void slotScrollViewerDown();
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
            void writeChildNodes( TreeNode* node, QDomElement& element, QDomDocument &document );

            void addFeed(const QString& url, TreeNode* after, FeedGroup* parent, bool autoExec = true);

            void updateSearch(const QString &s=QString::null);
            
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
    };
}

#endif // _AKREGATORVIEW_H_
