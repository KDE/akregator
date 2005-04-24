/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <kaction.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kxmlguiclient.h>

#include <qstring.h>

#include "actionmanager.h"
#include "akregatorconfig.h"
#include "akregator_part.h"
#include "akregator_view.h"
#include "articlelist.h"
#include "articleviewer.h"
#include "feedstree.h"
#include "fetchtransaction.h"
#include "trayicon.h"

#include <kdebug.h>

namespace Akregator {

ActionManager* ActionManager::m_self = 0;

ActionManager* ActionManager::getInstance()
{
    return m_self;
}

void ActionManager::setInstance(ActionManager* manager)
{
    m_self = manager;
}

ActionManager::ActionManager(KXMLGUIClient* client, QObject* parent, const char* name) : QObject(parent, name), m_client(client)
{
    m_part = 0;
    m_feedListView = 0;
    m_articleList = 0;
    m_trayIcon = 0;
    m_articleViewer = 0;
    m_feedListView = 0;
    m_view = 0;
    m_actionCollection = client->actionCollection();
}

ActionManager::~ActionManager()
{
}

void ActionManager::initTrayIcon(TrayIcon* trayIcon)
{
    if (m_trayIcon)
        return;
    else m_trayIcon = trayIcon;

    KPopupMenu* traypop = trayIcon->contextMenu();

    if (actionCollection()->action("feed_fetch_all"))
        actionCollection()->action("feed_fetch_all")->plug(traypop, 1);
    if (actionCollection()->action("akregator_configure_akregator"))
        actionCollection()->action("akregator_configure_akregator")->plug(traypop, 2);
}

void ActionManager::initPart(Part* part)
{
    if (m_part)
        return;
    else m_part = part;
        
    new KAction(i18n("&Import Feeds..."), "", "", part, SLOT(fileImport()), m_actionCollection, "file_import");
    new KAction(i18n("&Export Feeds..."), "", "", part, SLOT(fileExport()), m_actionCollection, "file_export");
    new KAction(i18n("&Get Feeds From Web..."), "", "", part, SLOT(fileGetFeeds()), m_actionCollection, "file_getfromweb");
    
    KStdAction::configureNotifications(part, SLOT(showKNotifyOptions()), m_actionCollection); // options_configure_notifications
    new KAction( i18n("Configure &Akregator..."), "configure", "", part, SLOT(showOptions()), m_actionCollection, "akregator_configure_akregator" );
}

void ActionManager::initView(View* view)
{
    if (m_view)
        return;
    else
        m_view = view;
 /* --- Feed/Feed Group popup menu */
    new KAction(i18n("&Open Homepage"), "", "Ctrl+H", m_view, SLOT(slotOpenHomepage()), actionCollection(), "feed_homepage");
    new KAction(i18n("&Add Feed..."), "bookmark_add", "Insert", m_view, SLOT(slotFeedAdd()), actionCollection(), "feed_add");
    new KAction(i18n("Ne&w Folder..."), "folder_new", "Shift+Insert", m_view, SLOT(slotFeedAddGroup()), actionCollection(), "feed_add_group");
    new KAction(i18n("&Delete Feed"), "editdelete", "Alt+Delete", m_view, SLOT(slotFeedRemove()), actionCollection(), "feed_remove");
    new KAction(i18n("&Edit Feed..."), "edit", "F2", m_view, SLOT(slotFeedModify()), actionCollection(), "feed_modify");
        KActionMenu* vm = new KActionMenu( i18n( "&View Mode" ), actionCollection(), "view_mode" );
    
    KRadioAction *ra = new KRadioAction(i18n("&Normal View"), "view_top_bottom", "Ctrl+Shift+1", m_view, SLOT(slotNormalView()), actionCollection(), "normal_view");
    ra->setExclusiveGroup( "ViewMode" );
    vm->insert(ra);
    
    ra = new KRadioAction(i18n("&Widescreen View"), "view_left_right", "Ctrl+Shift+2", m_view, SLOT(slotWidescreenView()), actionCollection(), "widescreen_view");
    ra->setExclusiveGroup( "ViewMode" );
    vm->insert(ra);
    
    ra = new KRadioAction(i18n("C&ombined View"), "view_text", "Ctrl+Shift+3", m_view, SLOT(slotCombinedView()), actionCollection(), "combined_view");
    ra->setExclusiveGroup( "ViewMode" );
    vm->insert(ra);

    // toolbar / feed menu
    new KAction(i18n("&Fetch Feed"), "down", "Ctrl+L", m_view, SLOT(slotFetchCurrentFeed()), actionCollection(), "feed_fetch");
    new KAction(i18n("Fe&tch All Feeds"), "bottom", "Ctrl+Shift+L", m_view, SLOT(slotFetchAllFeeds()), actionCollection(), "feed_fetch_all");
        
    KAction* stopAction = new KAction(i18n( "&Abort Fetches" ), "stop", Key_Escape, FetchQueue::self(), SLOT(slotAbort()), actionCollection(), "feed_stop");
    stopAction->setEnabled(false);
    
    new KAction(i18n("&Mark Feed as Read"), "apply", "Ctrl+R", m_view, SLOT(slotMarkAllRead()), actionCollection(), "feed_mark_all_as_read");
    new KAction(i18n("Ma&rk All Feeds as Read"), "apply", "Ctrl+Shift+R", m_view, SLOT(slotMarkAllFeedsRead()), actionCollection(), "feed_mark_all_feeds_as_read");

    // Settings menu
    KToggleAction* sqf = new KToggleAction(i18n("Show Quick Filter"), QString::null, 0, m_view, SLOT(slotToggleShowQuickFilter()), actionCollection(), "show_quick_filter");
    sqf->setChecked( Settings::showQuickFilter() );

    new KAction( i18n("Open Article in Tab"), "tab_new", "Shift+Return", m_view, SLOT(slotOpenCurrentArticle()), actionCollection(), "article_open" );
    new KAction( i18n("Open Article in Background Tab"), QString::null, "tab_new", m_view, SLOT(slotOpenCurrentArticleBackgroundTab()), actionCollection(), "article_open_background_tab" );
    new KAction( i18n("Open Article in External Browser"), "window_new", "Ctrl+Shift+Return", m_view, SLOT(slotOpenCurrentArticleExternal()), actionCollection(), "article_open_external" );

    new KAction(i18n("Pre&vious Unread Article"), "", Key_Minus, m_view, SLOT(slotPrevUnreadArticle()),actionCollection(), "go_prev_unread_article");
    new KAction(i18n("Ne&xt Unread Article"), "", Key_Plus, m_view, SLOT(slotNextUnreadArticle()),actionCollection(), "go_next_unread_article");

    new KAction(i18n("Select Next Tab"), "", "Ctrl+Period", m_view, SLOT(slotNextTab()),actionCollection(), "select_next_tab");
    new KAction(i18n("Select Previous Tab"), "", "Ctrl+Comma", m_view, SLOT(slotPreviousTab()),actionCollection(), "select_previous_tab");

    new KAction(i18n("&Toggle Keep Flag"), "flag", "Ctrl+K", m_view, SLOT(slotArticleToggleKeepFlag()), actionCollection(), "article_toggle_keep");
    
    new KAction(i18n("&Delete"), "editdelete", "Delete", m_view, SLOT(slotArticleDelete()), actionCollection(), "article_delete");
    

    KActionMenu* statusMenu = new KActionMenu ( i18n( "&Mark" ),
                                    actionCollection(), "article_set_status" );

    statusMenu->insert(new KAction(KGuiItem(i18n("Mark as &Read"), "",
                       i18n("Mark selected article as read")),
    "Ctrl+E", m_view, SLOT(slotSetSelectedArticleRead()),
    actionCollection(), "article_set_status_read"));
    
    statusMenu->insert(new KAction(KGuiItem(i18n("Mark as &Unread"), "",
                       i18n("Mark selected article as unread")),
    "Ctrl+U", m_view, SLOT(slotSetSelectedArticleUnread()),
    actionCollection(), "article_set_status_unread"));

    statusMenu->insert(new KAction(KGuiItem(i18n("Mark as &New"), "",
                        i18n("Mark selected article as new")),
    "Ctrl+N", m_view, SLOT(slotSetSelectedArticleNew()),
    actionCollection(), "article_set_status_new" ));

    new KAction( i18n("Move Node Up"), QString::null, "Shift+Alt+Up", view, SLOT(slotMoveCurrentNodeUp()), m_actionCollection, "feedstree_move_up" );
    new KAction( i18n("Move Node Down"), QString::null,  "Shift+Alt+Down", view, SLOT(slotMoveCurrentNodeDown()), m_actionCollection, "feedstree_move_down" );
    new KAction( i18n("Move Node Left"), QString::null, "Shift+Alt+Left", view, SLOT(slotMoveCurrentNodeLeft()), m_actionCollection, "feedstree_move_left" );
    new KAction( i18n("Move Node Right"), QString::null, "Shift+Alt+Right", view, SLOT(slotMoveCurrentNodeRight()), m_actionCollection, "feedstree_move_right");
}

void ActionManager::initArticleViewer(ArticleViewer* articleViewer)
{
    if (m_articleViewer)
        return;
    else
        m_articleViewer = articleViewer;
}

void ActionManager::initArticleListView(ArticleListView* articleList)
{
    if (m_articleList)
        return;
    else
        m_articleList = articleList;

    new KAction( i18n("&Previous Article"), QString::null, "Left", articleList, SLOT(slotPreviousArticle()), actionCollection(), "go_previous_article" );
    new KAction( i18n("&Next Article"), QString::null, "Right", articleList, SLOT(slotNextArticle()), actionCollection(), "go_next_article" );
}

void ActionManager::initFeedListView(FeedListView* feedListView)
{
    if (m_feedListView)
        return;
    else
        m_feedListView = feedListView;

    new KAction(i18n("&Previous Feed"), "", "P", feedListView,  SLOT(slotPrevFeed()),actionCollection(), "go_prev_feed");
    new KAction(i18n("&Next Feed"), "", "N", feedListView, SLOT(slotNextFeed()),actionCollection(), "go_next_feed");
    new KAction(i18n("N&ext Unread Feed"), "", "Alt+Plus", feedListView, SLOT(slotNextUnreadFeed()),actionCollection(), "go_next_unread_feed");
    new KAction(i18n("Prev&ious Unread Feed"), "", "Alt+Minus", feedListView, SLOT(slotPrevUnreadFeed()),actionCollection(), "go_prev_unread_feed");

    new KAction( i18n("Go to Top of Tree"), QString::null, "Alt+Home", feedListView, SLOT(slotItemBegin()), m_actionCollection, "feedstree_home" );
    new KAction( i18n("Go to Bottom of Tree"), QString::null, "Alt+End", feedListView, SLOT(slotItemEnd()), m_actionCollection, "feedstree_end" );
    new KAction( i18n("Go Left in Tree"), QString::null, "Alt+Left", feedListView, SLOT(slotItemLeft()), m_actionCollection, "feedstree_left" );
    new KAction( i18n("Go Right in Tree"), QString::null, "Alt+Right", feedListView, SLOT(slotItemRight()), m_actionCollection, "feedstree_right" );    
    new KAction( i18n("Go Up in Tree"), QString::null, "Alt+Up", feedListView, SLOT(slotItemUp()), m_actionCollection, "feedstree_up" );
    new KAction( i18n("Go Down in Tree"), QString::null, "Alt+Down", feedListView, SLOT(slotItemDown()), m_actionCollection, "feedstree_down" );
}

KActionCollection* ActionManager::actionCollection()
{
    return m_actionCollection;
}
KAction* ActionManager::action(const char* name, const char* classname)
{
    return m_actionCollection != 0 ? m_actionCollection->action(name, classname) : 0;
}

} // namespace Akregator

#include "actionmanager.moc"
