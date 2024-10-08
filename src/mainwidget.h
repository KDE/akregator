/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
    SPDX-FileCopyrightText: 2004 Sashmit Bhaduri <smt@vfemail.net>
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "akregatorpart_export.h"
#include "articleviewer-ng/webengine/articleviewerwebenginewidgetng.h"
#include "feed.h"

#include <QUrl>

#include <QPointer>
#include <QWidget>

#include <QSharedPointer>

class KConfig;
class KConfigGroup;

class QDomDocument;
class QSplitter;

namespace Akregator
{
class WebEngineFrame;
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
class MainFrame;
class DownloadArticleJob;
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
    @param actionManager Akregator specific implementation of ActionManager
    @param name the name of the widget (@ref QWidget )
    */
    MainWidget(Akregator::Part *part, QWidget *parent, ActionManagerImpl *actionManager, const QString &name);

    /** destructor.  Note that cleanups should be done in
    slotOnShutdown(), so we don't risk accessing self-deleting objects after deletion. */
    ~MainWidget() override;

    /** saves settings. Make sure that the Settings singleton is not destroyed yet when saveSettings is called */
    void saveSettings();

    /** Adds the feeds in @c doc to the "Imported Folder"
    @param doc the DOM tree (OPML) of the feeds to import */
    void importFeedList(const QDomDocument &doc);

    /**
     * @return the displayed Feed List in OPML format
     */
    [[nodiscard]] QDomDocument feedListToOPML();

    void setFeedList(const QSharedPointer<FeedList> &feedList);

    /**
     * Add a feed to a group.
     * @param url The URL of the feed to add.
     * @param group The name of the folder into which the feed is added.
     * If the group does not exist, it is created.
     * The feed is added as the last member of the group.
     */
    void addFeedToGroup(const QString &url, const QString &group);

    [[nodiscard]] QSharedPointer<FeedList> allFeedsList()
    {
        return m_feedList;
    }

    /** session management **/
    void readProperties(const KConfigGroup &config);
    void saveProperties(KConfigGroup &config);

    // Returns true if networking is available
    [[nodiscard]] bool isNetworkAvailable() const;

    enum ViewMode {
        NormalView = 0,
        WidescreenView,
        CombinedView,
    };

    [[nodiscard]] ViewMode viewMode() const
    {
        return m_viewMode;
    }

    void currentArticleInfo(QString &link, QString &title);
    void updateQuickSearchLineText();
Q_SIGNALS:
    /** emitted when the unread count of "All Feeds" was changed */
    void signalUnreadCountChanged(int);

    /** emitted when the articles selected changed */
    void signalArticlesSelected(const QList<Akregator::Article> &);

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
    /** marks the currently selected article as read after a user-set delay */
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

    void slotSendLink();
    void slotSendFile();

    void slotNetworkStatusChanged(bool status);

    void slotFocusQuickSearch();

    void slotWhatsNew();

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
    void slotCurrentFrameChanged(int frameId);
    void slotArticleAction(Akregator::ArticleViewerWebEngine::ArticleAction type, const QString &articleId, const QString &feed);
    void slotSettingsChanged();

private:
    void slotSetFocusToViewer();
    void sendArticle(const QByteArray &text, const QString &title, bool attach);
    void deleteExpiredArticles(const QSharedPointer<FeedList> &feedList);
    void connectFrame(Akregator::WebEngineFrame *frame);
    void cleanUpDownloadFile();
    /** ask for confirmation when marking feed(s) as read */
    [[nodiscard]] bool confirmMarkFeedAsRead(bool isSingleFeed, bool isGroup);

    /** opens current article in new tab, background/foreground depends on settings TODO: use selected instead of current? */
    void openSelectedArticles(bool openInBackground);

    AbstractSelectionController *m_selectionController;
    QSharedPointer<FeedList> m_feedList;

    SubscriptionListView *m_feedListView = nullptr;
    ArticleListView *m_articleListView = nullptr;

    ArticleViewerWidget *m_articleViewer = nullptr;
    TabWidget *m_tabWidget = nullptr;

    QWidget *m_mainTab = nullptr;
    MainFrame *m_mainFrame = nullptr;

    SearchBar *m_searchBar = nullptr;

    QSplitter *m_articleSplitter = nullptr;
    QSplitter *m_horizontalSplitter = nullptr;

    Akregator::Part *m_part = nullptr;
    ViewMode m_viewMode = NormalView;

    QTimer *m_fetchTimer = nullptr;
    QTimer *m_expiryTimer = nullptr;
    QTimer *m_markReadTimer = nullptr;

    bool m_shuttingDown = false;
    bool m_displayingAboutPage = false;

    ActionManagerImpl *m_actionManager = nullptr;
    FeedListManagementImpl *const m_feedListManagementInterface = nullptr;

    QWidget *m_articleWidget = nullptr;
    QList<QPointer<Akregator::DownloadArticleJob>> mListDownloadArticleJobs;
};
} // namespace Akregator
