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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qwidget.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kxmlguifactory.h>

#include <qmap.h>
#include <qstring.h>
#include <qvaluelist.h>

#include "actionmanagerimpl.h"
#include "akregatorconfig.h"
#include "akregator_part.h"
#include "akregator_view.h"
#include "articlelistview.h"
#include "articleviewer.h"
#include "feed.h"
#include "feedlistview.h"
#include "fetchqueue.h"
#include "folder.h"
#include "kernel.h"
#include "speechclient.h"
#include "tag.h"
#include "tagaction.h"
#include "tagnode.h"
#include "tagset.h"
#include "trayicon.h"
#include "treenode.h"
#include "treenodevisitor.h"
#include "tabwidget.h"
#include "kstdaccel.h"



#include <kdebug.h>

namespace Akregator
{

class ActionManagerImpl::NodeSelectVisitor : public TreeNodeVisitor
{
    public:
    NodeSelectVisitor(ActionManagerImpl* manager) : m_manager(manager) {}

    virtual bool visitFeed(Feed* /*node*/)
    {
        KAction* remove = m_manager->action("feed_remove");
        if (remove)
            remove->setEnabled(true);
        KAction* hp = m_manager->action("feed_homepage");
        if (hp)
            remove->setEnabled(true);
        m_manager->action("feed_fetch")->setText("&Fetch Feed");
        m_manager->action("feed_remove")->setText("&Delete Feed");
        m_manager->action("feed_modify")->setText("&Edit Feed...");
        m_manager->action("feed_mark_all_as_read")->setText("&Mark Feed as Read");

        return true;
    }

    virtual bool visitFolder(Folder* node)
    {
        KAction* remove = m_manager->action("feed_remove");
        if (remove)
            remove->setEnabled(node->parent()); // root nodes must not be deleted
        KAction* hp = m_manager->action("feed_homepage");
        if (hp)
            hp->setEnabled(false);

        m_manager->action("feed_fetch")->setText("&Fetch Feeds");
        m_manager->action("feed_remove")->setText("&Delete Folder");
        m_manager->action("feed_modify")->setText("&Rename Folder");
        m_manager->action("feed_mark_all_as_read")->setText("&Mark Feeds as Read");

        return true;
    }

    virtual bool visitTagNode(TagNode* /*node*/)
    {
        KAction* remove = m_manager->action("feed_remove");
        if (remove)
            remove->setEnabled(true);
        KAction* hp = m_manager->action("feed_homepage");
        if (hp)
            hp->setEnabled(false);
        m_manager->action("feed_mark_all_as_read")->setText("&Mark Articles as Read");
        m_manager->action("feed_remove")->setText("&Delete Tag");
        m_manager->action("feed_modify")->setText("&Edit Tag...");

        return true;
    }
    private:
    ActionManagerImpl* m_manager;
};

class ActionManagerImpl::ActionManagerImplPrivate
{
public:

    NodeSelectVisitor* nodeSelectVisitor;
    ArticleListView* articleList;
    FeedListView* feedListView;
    View* view;
    ArticleViewer* articleViewer;
    Part* part;
    TrayIcon* trayIcon;
    KActionMenu* assignTagMenu;
    KActionMenu* removeTagMenu;
    KActionCollection* actionCollection;
    TagSet* tagSet;
    QMap<QString, KAction*> assignTagActions;
    QMap<QString, KAction*> removeTagActions;
	TabWidget* tabWidget;
    KAction* speakSelectedArticlesAction;
};

void ActionManagerImpl::slotUpdateRemoveTagMenu(const QStringList& tagIds)
{
    QValueList<KAction*> actions = d->removeTagActions.values();

    for (QValueList<KAction*>::ConstIterator it = actions.begin(); it != actions.end(); ++it)
        d->removeTagMenu->remove(*it);

    for (QStringList::ConstIterator it = tagIds.begin(); it != tagIds.end(); ++it)
        d->removeTagMenu->insert(d->removeTagActions[*it]);

}

void ActionManagerImpl::setTagSet(TagSet* tagSet)
{
    if (tagSet == d->tagSet)
        return;

    if (d->tagSet != 0)
    {
        disconnect(d->tagSet, SIGNAL(signalTagAdded(const Tag&)), this, SLOT(slotTagAdded(const Tag&)));
        disconnect(d->tagSet, SIGNAL(signalTagRemoved(const Tag&)), this, SLOT(slotTagRemoved(const Tag&)));
    }

    d->tagSet = tagSet;

    if (tagSet != 0)
    {
        connect(d->tagSet, SIGNAL(signalTagAdded(const Tag&)), this, SLOT(slotTagAdded(const Tag&)));
        connect(d->tagSet, SIGNAL(signalTagRemoved(const Tag&)), this, SLOT(slotTagRemoved(const Tag&)));
    }

    QValueList<KAction*> actions = d->assignTagActions.values();
    for (QValueList<KAction*>::ConstIterator it = actions.begin(); it != actions.end(); ++it)
    {
        d->assignTagMenu->remove(*it);
        delete *it;
    }

    actions = d->removeTagActions.values();
    for (QValueList<KAction*>::ConstIterator it = actions.begin(); it != actions.end(); ++it)
    {
        d->removeTagMenu->remove(*it);
        delete *it;
    }

    d->assignTagActions.clear();
    d->removeTagActions.clear();

    //TODO: remove actions from menus, delete actions, clear maps

    QValueList<Tag> list = tagSet->toMap().values();
    for (QValueList<Tag>::ConstIterator it = list.begin(); it != list.end(); ++it)
        slotTagAdded(*it);
}

void ActionManagerImpl::slotTagAdded(const Tag& tag)
{
    if (!d->assignTagActions.contains(tag.id()))
    {
        d->assignTagActions[tag.id()] = new TagAction(tag, d->view, SLOT(slotAssignTag(const Tag&)), d->assignTagMenu);
        d->removeTagActions[tag.id()] = new TagAction(tag, d->view, SLOT(slotRemoveTag(const Tag&)), d->removeTagMenu);
        d->assignTagMenu->insert(d->assignTagActions[tag.id()]);
        //TODO: add to assignTagMenu (sorted!)
    }
}

void ActionManagerImpl::slotTagRemoved(const Tag& tag)
{
    QString id = tag.id();
    d->assignTagMenu->remove(d->assignTagActions[id]);
    d->removeTagMenu->remove(d->removeTagActions[id]);
    d->assignTagActions.remove(id);
    d->removeTagActions.remove(id);
}

void ActionManagerImpl::slotNodeSelected(TreeNode* node)
{
    d->nodeSelectVisitor->visit(node);
}

ActionManagerImpl::ActionManagerImpl(Part* part, QObject* parent, const char* name) : ActionManager(parent, name), d(new ActionManagerImplPrivate)
{
    d->nodeSelectVisitor = new NodeSelectVisitor(this);
    d->part = part;
    d->tagSet = 0;
    d->feedListView = 0;
    d->articleList = 0;
    d->trayIcon = 0;
    d->articleViewer = 0;
    d->feedListView = 0;
    d->view = 0;
    d->tabWidget = 0;
    d->speakSelectedArticlesAction = 0;
    d->actionCollection = part->actionCollection();
    initPart();
}

ActionManagerImpl::~ActionManagerImpl()
{
    delete d;
    d = 0;
    delete d->nodeSelectVisitor;
}

void ActionManagerImpl::initTrayIcon(TrayIcon* trayIcon)
{
    if (d->trayIcon)
        return;
    else d->trayIcon = trayIcon;

    KPopupMenu* traypop = trayIcon->contextMenu();

    if (actionCollection()->action("feed_fetch_all"))
        actionCollection()->action("feed_fetch_all")->plug(traypop, 1);
    if (actionCollection()->action("akregator_configure_akregator"))
        actionCollection()->action("akregator_configure_akregator")->plug(traypop, 2);
}

void ActionManagerImpl::initPart()
{
    new KAction(i18n("&Import Feeds..."), "", "", d->part, SLOT(fileImport()), d->actionCollection, "file_import");
    new KAction(i18n("&Export Feeds..."), "", "", d->part, SLOT(fileExport()), d->actionCollection, "file_export");
    //new KAction(i18n("&Get Feeds From Web..."), "", "", d->part, SLOT(fileGetFeeds()), d->actionCollection, "file_getfromweb");

    new KAction(i18n("Send &Link Address..."), "mail_generic", "", d->part, SLOT(fileSendLink()), d->actionCollection, "file_sendlink");
    new KAction(i18n("Send &File..."), "mail_generic", "", d->part, SLOT(fileSendFile()), d->actionCollection, "file_sendfile");

    KStdAction::configureNotifications(d->part, SLOT(showKNotifyOptions()), d->actionCollection); // options_configure_notifications
    new KAction( i18n("Configure &Akregator..."), "configure", "", d->part, SLOT(showOptions()), d->actionCollection, "akregator_configure_akregator" );
}

void ActionManagerImpl::initView(View* view)
{
    if (d->view)
        return;
    else
        d->view = view;

    // tag actions
    new KAction(i18n("&New Tag..."), "", "", d->view, SLOT(slotNewTag()), actionCollection(), "tag_new");

    // Feed/Feed Group popup menu
    new KAction(i18n("&Open Homepage"), "", "Ctrl+H", d->view, SLOT(slotOpenHomepage()), actionCollection(), "feed_homepage");
    new KAction(i18n("&Add Feed..."), "bookmark_add", "Insert", d->view, SLOT(slotFeedAdd()), actionCollection(), "feed_add");
    new KAction(i18n("Ne&w Folder..."), "folder_new", "Shift+Insert", d->view, SLOT(slotFeedAddGroup()), actionCollection(), "feed_add_group");
    new KAction(i18n("&Delete Feed"), "editdelete", "Alt+Delete", d->view, SLOT(slotFeedRemove()), actionCollection(), "feed_remove");
    new KAction(i18n("&Edit Feed..."), "edit", "F2", d->view, SLOT(slotFeedModify()), actionCollection(), "feed_modify");
        KActionMenu* vm = new KActionMenu( i18n( "&View Mode" ), actionCollection(), "view_mode" );

    KRadioAction *ra = new KRadioAction(i18n("&Normal View"), "view_top_bottom", "Ctrl+Shift+1", d->view, SLOT(slotNormalView()), actionCollection(), "normal_view");
    ra->setExclusiveGroup( "ViewMode" );
    vm->insert(ra);

    ra = new KRadioAction(i18n("&Widescreen View"), "view_left_right", "Ctrl+Shift+2", d->view, SLOT(slotWidescreenView()), actionCollection(), "widescreen_view");
    ra->setExclusiveGroup( "ViewMode" );
    vm->insert(ra);

    ra = new KRadioAction(i18n("C&ombined View"), "view_text", "Ctrl+Shift+3", d->view, SLOT(slotCombinedView()), actionCollection(), "combined_view");
    ra->setExclusiveGroup( "ViewMode" );
    vm->insert(ra);

    // toolbar / feed menu
    new KAction(i18n("&Fetch Feed"), "down", KStdAccel::shortcut(KStdAccel::Reload), d->view, SLOT(slotFetchCurrentFeed()), actionCollection(), "feed_fetch");
    new KAction(i18n("Fe&tch All Feeds"), "bottom", "Ctrl+L", d->view, SLOT(slotFetchAllFeeds()), actionCollection(), "feed_fetch_all");

    KAction* stopAction = new KAction(i18n( "&Abort Fetches" ), "stop", Key_Escape, Kernel::self()->fetchQueue(), SLOT(slotAbort()), actionCollection(), "feed_stop");
    stopAction->setEnabled(false);

    new KAction(i18n("&Mark Feed as Read"), "goto", "Ctrl+R", d->view, SLOT(slotMarkAllRead()), actionCollection(), "feed_mark_all_as_read");
    new KAction(i18n("Ma&rk All Feeds as Read"), "goto", "Ctrl+Shift+R", d->view, SLOT(slotMarkAllFeedsRead()), actionCollection(), "feed_mark_all_feeds_as_read");

    // Settings menu
    KToggleAction* sqf = new KToggleAction(i18n("Show Quick Filter"), QString::null, 0, d->view, SLOT(slotToggleShowQuickFilter()), actionCollection(), "show_quick_filter");
    sqf->setChecked( Settings::showQuickFilter() );

    new KAction( i18n("Open in Tab"), "tab_new", "Shift+Return", d->view, SLOT(slotOpenCurrentArticle()), actionCollection(), "article_open" );
    new KAction( i18n("Open in Background Tab"), QString::null, "tab_new", d->view, SLOT(slotOpenCurrentArticleBackgroundTab()), actionCollection(), "article_open_background_tab" );
    new KAction( i18n("Open in External Browser"), "window_new", "Ctrl+Shift+Return", d->view, SLOT(slotOpenCurrentArticleExternal()), actionCollection(), "article_open_external" );
    new KAction( i18n("Copy Link Address"), QString::null, QString::null, d->view, SLOT(slotCopyLinkAddress()), actionCollection(), "article_copy_link_address" );

    new KAction(i18n("Pre&vious Unread Article"), "", Key_Minus, d->view, SLOT(slotPrevUnreadArticle()),actionCollection(), "go_prev_unread_article");
    new KAction(i18n("Ne&xt Unread Article"), "", Key_Plus, d->view, SLOT(slotNextUnreadArticle()),actionCollection(), "go_next_unread_article");

    new KAction(i18n("&Delete"), "editdelete", "Delete", d->view, SLOT(slotArticleDelete()), actionCollection(), "article_delete");

    d->assignTagMenu = new KActionMenu ( i18n( "&Assign Tag" ),
                                    actionCollection(), "article_assign_tag_menu" );

    d->removeTagMenu = new KActionMenu ( i18n( "&Remove Tag" ), actionCollection(), "article_remove_tag_menu" );

    KActionMenu* statusMenu = new KActionMenu ( i18n( "&Mark As" ),
                                    actionCollection(), "article_set_status" );

    d->speakSelectedArticlesAction = new KAction(i18n("&Speak Selected Articles"), "kttsd", "", d->view, SLOT(slotTextToSpeechRequest()), actionCollection(), "akr_texttospeech");
    
    KAction* abortTTS = new KAction(i18n( "&Stop Speaking" ), "player_stop", Key_Escape, SpeechClient::self(), SLOT(slotAbortJobs()), actionCollection(), "akr_aborttexttospeech");
    abortTTS->setEnabled(false);

    connect(SpeechClient::self(), SIGNAL(signalActivated(bool)),
    abortTTS, SLOT(setEnabled(bool)));

    statusMenu->insert(new KAction(KGuiItem(i18n("&Read"), "",
                       i18n("Mark selected article as read")),
    "Ctrl+E", d->view, SLOT(slotSetSelectedArticleRead()),
    actionCollection(), "article_set_status_read"));

    statusMenu->insert(new KAction(KGuiItem(i18n("&New"), "",
                        i18n("Mark selected article as new")),
    "Ctrl+N", d->view, SLOT(slotSetSelectedArticleNew()),
    actionCollection(), "article_set_status_new" ));


    statusMenu->insert(new KAction(KGuiItem(i18n("&Unread"), "",
                       i18n("Mark selected article as unread")),
    "Ctrl+U", d->view, SLOT(slotSetSelectedArticleUnread()),
    actionCollection(), "article_set_status_unread"));

    KToggleAction* importantAction = new KToggleAction(i18n("&Mark as Important"), "flag", "Ctrl+I", actionCollection(), "article_set_status_important");
    importantAction->setCheckedState(i18n("Remove &Important Mark"));
    connect(importantAction, SIGNAL(toggled(bool)), d->view, SLOT(slotArticleToggleKeepFlag(bool)));


    new KAction( i18n("Move Node Up"), QString::null, "Shift+Alt+Up", view, SLOT(slotMoveCurrentNodeUp()), d->actionCollection, "feedstree_move_up" );
    new KAction( i18n("Move Node Down"), QString::null,  "Shift+Alt+Down", view, SLOT(slotMoveCurrentNodeDown()), d->actionCollection, "feedstree_move_down" );
    new KAction( i18n("Move Node Left"), QString::null, "Shift+Alt+Left", view, SLOT(slotMoveCurrentNodeLeft()), d->actionCollection, "feedstree_move_left" );
    new KAction( i18n("Move Node Right"), QString::null, "Shift+Alt+Right", view, SLOT(slotMoveCurrentNodeRight()), d->actionCollection, "feedstree_move_right");
}

void ActionManagerImpl::initArticleViewer(ArticleViewer* articleViewer)
{
    if (d->articleViewer)
        return;
    else
        d->articleViewer = articleViewer;
}

void ActionManagerImpl::initArticleListView(ArticleListView* articleList)
{
    if (d->articleList)
        return;
    else
        d->articleList = articleList;

    new KAction( i18n("&Previous Article"), QString::null, "Left", articleList, SLOT(slotPreviousArticle()), actionCollection(), "go_previous_article" );
    new KAction( i18n("&Next Article"), QString::null, "Right", articleList, SLOT(slotNextArticle()), actionCollection(), "go_next_article" );
}

void ActionManagerImpl::initFeedListView(FeedListView* feedListView)
{
    if (d->feedListView)
        return;
    else
        d->feedListView = feedListView;

    new KAction(i18n("&Previous Feed"), "", "P", feedListView,  SLOT(slotPrevFeed()),actionCollection(), "go_prev_feed");
    new KAction(i18n("&Next Feed"), "", "N", feedListView, SLOT(slotNextFeed()),actionCollection(), "go_next_feed");
    new KAction(i18n("N&ext Unread Feed"), "", "Alt+Plus", feedListView, SLOT(slotNextUnreadFeed()),actionCollection(), "go_next_unread_feed");
    new KAction(i18n("Prev&ious Unread Feed"), "", "Alt+Minus", feedListView, SLOT(slotPrevUnreadFeed()),actionCollection(), "go_prev_unread_feed");

    new KAction( i18n("Go to Top of Tree"), QString::null, "Ctrl+Home", feedListView, SLOT(slotItemBegin()), d->actionCollection, "feedstree_home" );
    new KAction( i18n("Go to Bottom of Tree"), QString::null, "Ctrl+End", feedListView, SLOT(slotItemEnd()), d->actionCollection, "feedstree_end" );
    new KAction( i18n("Go Left in Tree"), QString::null, "Ctrl+Left", feedListView, SLOT(slotItemLeft()), d->actionCollection, "feedstree_left" );
    new KAction( i18n("Go Right in Tree"), QString::null, "Ctrl+Right", feedListView, SLOT(slotItemRight()), d->actionCollection, "feedstree_right" );
    new KAction( i18n("Go Up in Tree"), QString::null, "Ctrl+Up", feedListView, SLOT(slotItemUp()), d->actionCollection, "feedstree_up" );
    new KAction( i18n("Go Down in Tree"), QString::null, "Ctrl+Down", feedListView, SLOT(slotItemDown()), d->actionCollection, "feedstree_down" );
}

void ActionManagerImpl::initTabWidget(TabWidget* tabWidget)
{
    if (d->tabWidget)
        return;
    else
        d->tabWidget = tabWidget;

    new KAction(i18n("Select Next Tab"), "", "Ctrl+Period", d->tabWidget, SLOT(slotNextTab()),actionCollection(), "select_next_tab");
    new KAction(i18n("Select Previous Tab"), "", "Ctrl+Comma", d->tabWidget, SLOT(slotPreviousTab()),actionCollection(), "select_previous_tab");
    new KAction( i18n("Detach Tab"), "tab_breakoff", CTRL+SHIFT+Key_B, d->tabWidget, SLOT(slotDetachTab()), actionCollection(), "tab_detach" );
    new KAction( i18n("Copy Link Address"), QString::null, QString::null, d->tabWidget, SLOT(slotCopyLinkAddress()), actionCollection(), "tab_copylinkaddress" );
    new KAction( i18n("&Close Tab"), "tab_remove", KStdAccel::close(), d->tabWidget, SLOT(slotCloseTab()), actionCollection(), "tab_remove" );
}

QWidget* ActionManagerImpl::container(const char* name)
{
    return d->part->factory()->container(name, d->part);
}


KActionCollection* ActionManagerImpl::actionCollection()
{
    return d->actionCollection;
}
KAction* ActionManagerImpl::action(const char* name, const char* classname)
{
    return d->actionCollection != 0 ? d->actionCollection->action(name, classname) : 0;
}

} // namespace Akregator

#include "actionmanagerimpl.moc"
