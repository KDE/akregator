/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
                  2004 Sashmit Bhaduri <smt@vfemail.net>
                  2005 Frank Osterfeld <osterfeld@kde.org>

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

#include "akregatorpart_export.h"
#include "feed.h"

#include <QUrl>

#include <QWidget>

#include <QSharedPointer>

class KConfig;
class KFileItem;
class KConfigGroup;

class QDomDocument;
class QNetworkConfigurationManager;
class QSplitter;

namespace Akregator
{

class AbstractSelectionController;
class ActionManagerImpl;
class ArticleListView;
class ArticleViewerWidget;
class Folder;
class FeedList;
class FeedListManagementImpl;
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
    MainWidget(Akregator::Part *part, QWidget *parent, ActionManagerImpl *actionManager, const char *name);

    /** destructor.  Note that cleanups should be done in
    slotOnShutdown(), so we don't risk accessing self-deleting objects after deletion. */
    ~MainWidget();

    /** saves settings. Make sure that the Settings singleton is not destroyed yet when saveSettings is called */
    void saveSettings();

    /** Adds the feeds in @c doc to the "Imported Folder"
    @param doc the DOM tree (OPML) of the feeds to import */
    void importFeedList(const QDomDocument &doc);

    /**
     * @return the displayed Feed List in OPML format
     */
    QDomDocument feedListToOPML();

    void setFeedList(const QSharedPointer<FeedList> &feedList);

    /**
     * Add a feed to a group.
     * @param url The URL of the feed to add.
     * @param group The name of the folder into which the feed is added.
     * If the group does not exist, it is created.
     * The feed is added as the last member of the group.
     */
    void addFeedToGroup(const QString &url, const QString &group);

    QSharedPointer<FeedList> allFeedsList()
    {
        return m_feedList;
    }

    /** session management **/
    void readProperties(const KConfigGroup &config);
    void saveProperties(KConfigGroup &config);

    //Returns true if networking is available
    bool isNetworkAvailable() const;

    enum ViewMode {
        NormalView = 0,
        WidescreenView,
        CombinedView
    };

    ViewMode viewMode() const
    {
        return m_viewMode;
    }

    QVector<Article> speakSelectedArticles();
    void currentArticleInfo(QString &link, QString &title);
Q_SIGNALS:
    /** emitted when the unread count of "All Feeds" was changed */
    void signalUnreadCountChanged(int);

    /** emitted when the articles selected changed */
    void signalArticlesSelected(const QVector<Akregator::Article> &);

public Q_SLOTS:

    /** opens the current article (currentItem) in external browser
    TODO: use selected instead of current? */
    void slotOpenSelectedArticlesInBrowser();

    /** opens current article in new tab, background/foreground depends on settings TODO: use selected instead of current? */
    void slotOpenSelectedArticles();
    void slotOpenSelectedArticlesInBackground();

    void slotOnShutdown();

    /** selected tree node has changed */
    void slotNodeSelected(Akregator::TreeNode *node);

    /** the article selection has changed */
    void slotArticleSelected(const Akregator::Article &);

    void ensureArticleTabVisible();

    /** emits @ref signalUnreadCountChanged(int) */
    void slotSetTotalUnread();

    /** copies the link of current article to clipboard
    */
    void slotCopyLinkAddress();

    void slotRequestNewFrame(int &frameId);


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

    /** reloads all open tabs */
    void slotReloadAllTabs();

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

    void slotSendLink()
    {
        sendArticle();
    }
    void slotSendFile()
    {
        sendArticle(true);
    }

    void slotNetworkStatusChanged(bool status);

protected:

    void sendArticle(bool attach = false);

    void addFeed(const QString &url, TreeNode *after, Folder *parent, bool autoExec = true);

protected Q_SLOTS:

    /** special behaviour in article list view (TODO: move code there?) */
    void slotMouseButtonPressed(int button, const QUrl &);

    /** opens the link of an article in the external browser */
    void slotOpenArticleInBrowser(const Akregator::Article &article);

    void slotDoIntervalFetches();
    void slotDeleteExpiredArticles();

    void slotFetchingStarted();
    void slotFetchingStopped();

    void slotFramesChanged();

private Q_SLOTS:
    void slotShowStatusBarMessage(const QString &msg);

private:
    void deleteExpiredArticles(const QSharedPointer<FeedList> &feedList);

    /** opens current article in new tab, background/foreground depends on settings TODO: use selected instead of current? */
    void openSelectedArticles(bool openInBackground);

    AbstractSelectionController *m_selectionController;
    QSharedPointer<FeedList> m_feedList;

    SubscriptionListView *m_feedListView;
    ArticleListView *m_articleListView;

    ArticleViewerWidget *m_articleViewer;
    TabWidget *m_tabWidget;

    QWidget *m_mainTab;
    Frame *m_mainFrame;

    SearchBar *m_searchBar;

    QSplitter *m_articleSplitter;
    QSplitter *m_horizontalSplitter;

    Akregator::Part *m_part;
    ViewMode m_viewMode;

    QTimer *m_fetchTimer;
    QTimer *m_expiryTimer;
    QTimer *m_markReadTimer;

    bool m_shuttingDown;
    bool m_displayingAboutPage;

    ActionManagerImpl *m_actionManager;
    FeedListManagementImpl *const m_feedListManagementInterface;

    QNetworkConfigurationManager *m_networkConfigManager;
};

} // namespace Akregator

#endif // AKREGATOR_MAINWIDGET_H
