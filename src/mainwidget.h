/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
                  2004 Sashmit Bhaduri <smt@vfemail.net>
                  2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef AKREGATOR_MAINWIDGET_H
#define AKREGATOR_MAINWIDGET_H

#include "akregator_export.h"
#include "feed.h"

#include <kurl.h>

#include <QPixmap>
#include <QWidget>


class KConfig;
class KFileItem;
class K3ListView;
class KConfigGroup;

class QDomDocument;
class QSplitter;

namespace Akregator {

class AbstractSelectionController;
class ActionManagerImpl;
class ArticleListView;
class ArticleViewer;
class Folder;
class FeedList;
class Frame;
class Part;
class SearchBar;
class SubscriptionListView;
class TabWidget;

/**
    * This is the main widget of the view, containing tree view, article list, viewer etc.
    */
class AKREGATORPART_EXPORT MainWidget : public QWidget
{
    Q_OBJECT
    public:

        /** constructor
        @param part the Akregator::Part which contains this widget
        @param parent parent widget
        @param name the name of the widget (@ref QWidget )
        */
        MainWidget(Akregator::Part *part, QWidget *parent, ActionManagerImpl* actionManager, const char* name);

        /** destructor.  Note that cleanups should be done in
        slotOnShutdown(), so we don't risk accessing self-deleting objects after deletion. */
        ~MainWidget();

        /** saves settings. Make sure that the Settings singleton is not destroyed yet when saveSettings is called */
        void saveSettings();

        /** Adds the feeds in @c doc to the "Imported Folder"
        @param doc the DOM tree (OPML) of the feeds to import */
        bool importFeeds(const QDomDocument& doc);

        /** Parse OPML presentation of feeds and read in articles archive, if present. If @c parent is @c NULL, the current
        feed list is replaced by the parsed one
            @param doc QDomDocument generated from OPML
            @param parent The parent group the new nodes */
        bool loadFeeds(const QDomDocument& doc, Folder* parent = 0);

        /**
         * @return the displayed Feed List in OPML format
         */
        QDomDocument feedListToOPML();

        /**
         * Add a feed to a group.
         * @param url The URL of the feed to add.
         * @param group The name of the folder into which the feed is added.
         * If the group does not exist, it is created.
         * The feed is added as the last member of the group.
         */
        void addFeedToGroup(const QString& url, const QString& group);

        /** session management **/
        void readProperties(const KConfigGroup & config);
        void saveProperties(KConfigGroup & config);

    signals:
        /** emitted when the unread count of "All Feeds" was changed */
        void signalUnreadCountChanged(int);

    public slots:

        /** opens the current article (currentItem) in external browser
        TODO: use selected instead of current? */
        void slotOpenCurrentArticleInBrowser();

        /** opens current article in new tab, background/foreground depends on settings TODO: use selected instead of current? */
        void slotOpenCurrentArticle();

        void slotOnShutdown();

        /** selected tree node has changed */
        void slotNodeSelected(Akregator::TreeNode* node);

        /** the article selection has changed */
        void slotArticleSelected(const Akregator::Article&);

        /** Shows requested popup menu for feed tree */
        void slotFeedTreeContextMenu(K3ListView*, Akregator::TreeNode*, const QPoint&);

        /** emits @ref signalUnreadCountChanged(int) */
        void slotSetTotalUnread();

        /** copies the link of current article to clipboard
        */
        void slotCopyLinkAddress();

        void slotRequestNewFrame(int& frameId);

        /** called when URLs are dropped into the tree view */
        void slotFeedUrlDropped (KUrl::List &urls, Akregator::TreeNode* after, Akregator::Folder *parent);

        /** displays a URL in the status bar when the user moves the mouse over a link */
        void slotMouseOverInfo(const KFileItem& kifi);

        /** Feed has been fetched, populate article view if needed and update counters. */
        void slotFeedFetched(Akregator::Feed *);

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
        void slotArticleToggleKeepFlag(bool enabled);
        /** deletes the currently selected article */
        void slotArticleDelete();
        /** marks the currently selected article as read */
        void slotSetSelectedArticleRead();
        /** marks the currently selected article as unread */
        void slotSetSelectedArticleUnread();
        /** marks the currently selected article as new */
        void slotSetSelectedArticleNew();
        /** marks the currenctly selected article as read after a user-set delay */
        void slotSetCurrentArticleReadDelayed();

        /** reads the currently selected articles using KTTSD */
        void slotTextToSpeechRequest();

        /** switches view mode to normal view */
        void slotNormalView();
        /** switches view mode to widescreen view */
        void slotWidescreenView();
        /** switches view mode to combined view */
        void slotCombinedView();
        /** toggles the visibility of the filter bar */
        void slotToggleShowQuickFilter();

        /** selects the previous unread article in the article list */
        void slotPrevUnreadArticle();
        /** selects the next unread article in the article list */
        void slotNextUnreadArticle();

        void slotMoveCurrentNodeUp();
        void slotMoveCurrentNodeDown();
        void slotMoveCurrentNodeLeft();
        void slotMoveCurrentNodeRight();

        void slotSendLink() { sendArticle(); }
        void slotSendFile() { sendArticle(true); }

    protected:

        void sendArticle(bool attach=false);

        void addFeed(const QString& url, TreeNode* after, Folder* parent, bool autoExec = true);

        void connectToFeedList(FeedList* feedList);
        void disconnectFromFeedList(FeedList* feedList);

    protected slots:

        /** special behaviour in article list view (TODO: move code there?) */
        void slotMouseButtonPressed(int button, const Akregator::Article& article, const QPoint & pos, int c);

        /** opens the link of an article in the external browser */
        void slotOpenArticleInBrowser(const Akregator::Article& article);

        /** this is called by the ctor, does init steps which need a properly created view and part */

        void delayedInit();

        void setTabIcon(const QPixmap&);

        void slotDoIntervalFetches();
        void slotDeleteExpiredArticles();

        void slotFetchingStarted();
        void slotFetchingStopped();

    private:

        enum ViewMode { NormalView=0, WidescreenView, CombinedView };

        AbstractSelectionController* m_selectionController;
        FeedList* m_feedList;

        SubscriptionListView* m_feedListView;
        ArticleListView* m_articleListView;

        ArticleViewer *m_articleViewer;
        TabWidget* m_tabWidget;

        QWidget *m_mainTab;
        Frame *m_mainFrame;

        SearchBar* m_searchBar;

        QSplitter *m_articleSplitter;
        QSplitter *m_horizontalSplitter;

        Akregator::Part *m_part;
        ViewMode m_viewMode;

        QTimer *m_fetchTimer;
        QTimer* m_expiryTimer;
        QTimer *m_markReadTimer;

        bool m_shuttingDown;
        bool m_displayingAboutPage;

        ActionManagerImpl* m_actionManager;

        friend class EditNodePropertiesVisitor;
        class EditNodePropertiesVisitor;
        EditNodePropertiesVisitor* m_editNodePropertiesVisitor;
        friend class DeleteNodeVisitor;
        class DeleteNodeVisitor;
        DeleteNodeVisitor* m_deleteNodeVisitor;
};

} // namespace Akregator

#endif // AKREGATOR_MAINWIDGET_H
