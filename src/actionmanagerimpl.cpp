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

#include "actionmanagerimpl.h"
#include "akregatorconfig.h"
#include "akregator_part.h"
#include "articlelistview.h"
#include "articleviewer.h"
#include "feed.h"
#include "feedlistview.h"
#include "fetchqueue.h"
#include "folder.h"
#include "framemanager.h"
#include "kernel.h"
#include "listtabwidget.h"
#include "mainwidget.h"
#include "speechclient.h"
#include "tabwidget.h"
#include "tag.h"
#include "tagaction.h"
#include "tagnode.h"
#include "tagset.h"
#include "trayicon.h"
#include "treenode.h"
#include "treenodevisitor.h"

#include <kaction.h>
#include <kactioncollection.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmenu.h>
#include <kshortcut.h>
#include <kstdaccel.h>
#include <kxmlguifactory.h>

#include <QHash>
#include <QList>
#include <QString>
#include <QWidget>

namespace Akregator
{

class ActionManagerImpl::NodeSelectVisitor : public TreeNodeVisitor
{
    public:
    NodeSelectVisitor(ActionManagerImpl* manager) : m_manager(manager) {}

    virtual bool visitFeed(Feed* node)
    {
        KAction* remove = m_manager->action("feed_remove");
        if (remove)
            remove->setEnabled(true);
        KAction* hp = m_manager->action("feed_homepage");
        if (hp)
            hp->setEnabled(!node->htmlUrl().isEmpty());
        m_manager->action("feed_fetch")->setText(i18n("&Fetch Feed"));
        m_manager->action("feed_remove")->setText(i18n("&Delete Feed"));
        m_manager->action("feed_modify")->setText(i18n("&Edit Feed..."));
        m_manager->action("feed_mark_all_as_read")->setText(i18n("&Mark Feed as Read"));

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

        m_manager->action("feed_fetch")->setText(i18n("&Fetch Feeds"));
        m_manager->action("feed_remove")->setText(i18n("&Delete Folder"));
        m_manager->action("feed_modify")->setText(i18n("&Rename Folder"));
        m_manager->action("feed_mark_all_as_read")->setText(i18n("&Mark Feeds as Read"));

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
        m_manager->action("feed_mark_all_as_read")->setText(i18n("&Mark Articles as Read"));
        m_manager->action("feed_remove")->setText(i18n("&Delete Tag"));
        m_manager->action("feed_modify")->setText(i18n("&Edit Tag..."));

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
    ListTabWidget* listTabWidget;
    MainWidget* mainWidget;
    ArticleViewer* articleViewer;
    Part* part;
    TrayIcon* trayIcon;
    KActionMenu* tagMenu;
    KActionCollection* actionCollection;
    TagSet* tagSet;
    QHash<QString, TagAction*> tagActions;
    TabWidget* tabWidget;
    KAction* speakSelectedArticlesAction;
    FrameManager* frameManager;
};

void ActionManagerImpl::slotUpdateTagActions(bool enabled, const QStringList& tagIds)
{
    d->tagMenu->setEnabled(enabled);
    QList<TagAction*> actions = d->tagActions.values();

    for (QList<TagAction*>::ConstIterator it = actions.begin(); it != actions.end(); ++it)
    {
        (*it)->setChecked(tagIds.contains((*it)->tag().id()));
    }
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

    QList<TagAction*> actions = d->tagActions.values();
    for (QList<TagAction*>::ConstIterator it = actions.begin(); it != actions.end(); ++it)
    {
        d->tagMenu->remove(*it);
        delete *it;
    }

    
    d->tagActions.clear();

    //TODO: remove actions from menus, delete actions, clear maps

    QList<Tag> list = tagSet->toHash().values();
    for (QList<Tag>::ConstIterator it = list.begin(); it != list.end(); ++it)
        slotTagAdded(*it);
}

void ActionManagerImpl::slotTagAdded(const Tag& tag)
{
    if (!d->tagActions.contains(tag.id()))
    {
        d->tagActions[tag.id()] = new TagAction(tag, d->mainWidget, SLOT(slotAssignTag(const Tag&, bool)), d->actionCollection);
        d->tagMenu->insert(d->tagActions[tag.id()]);
    }
}

void ActionManagerImpl::slotTagRemoved(const Tag& tag)
{
    QString id = tag.id();
    TagAction* action = d->tagActions[id];
    d->tagMenu->remove(action);
    d->tagActions.remove(id);
    delete action;
}

void ActionManagerImpl::slotNodeSelected(TreeNode* node)
{
    if (node != 0)
        d->nodeSelectVisitor->visit(node);
}

ActionManagerImpl::ActionManagerImpl(Part* part, QObject* parent, const char* name) : ActionManager(parent, name), d(new ActionManagerImplPrivate)
{
    d->nodeSelectVisitor = new NodeSelectVisitor(this);
    d->part = part;
    d->tagSet = 0;
    d->listTabWidget = 0;
    d->articleList = 0;
    d->trayIcon = 0;
    d->articleViewer = 0;
    d->mainWidget = 0;
    d->tabWidget = 0;
    d->frameManager = 0;
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

    KMenu* traypop = trayIcon->contextMenu();

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

    KStdAction::configureNotifications(d->part, SLOT(showKNotifyOptions()), d->actionCollection); // options_configure_notifications
    new KAction( i18n("Configure &Akregator..."), "configure", "", d->part, SLOT(showOptions()), d->actionCollection, "akregator_configure_akregator" );
}

void ActionManagerImpl::initMainWidget(MainWidget* mainWidget)
{
    if (d->mainWidget)
        return;
    else
        d->mainWidget = mainWidget;

    // tag actions
    new KAction(i18n("&New Tag..."), "", "", d->mainWidget, SLOT(slotNewTag()), actionCollection(), "tag_new");

    // Feed/Feed Group popup menu
    new KAction(i18n("&Open Homepage"), "", "Ctrl+H", d->mainWidget, SLOT(slotOpenHomepage()), actionCollection(), "feed_homepage");
    new KAction(i18n("&Add Feed..."), "bookmark_add", "Insert", d->mainWidget, SLOT(slotFeedAdd()), actionCollection(), "feed_add");
    new KAction(i18n("Ne&w Folder..."), "folder_new", "Shift+Insert", d->mainWidget, SLOT(slotFeedAddGroup()), actionCollection(), "feed_add_group");
    new KAction(i18n("&Delete Feed"), "editdelete", "Alt+Delete", d->mainWidget, SLOT(slotFeedRemove()), actionCollection(), "feed_remove");
    new KAction(i18n("&Edit Feed..."), "edit", "F2", d->mainWidget, SLOT(slotFeedModify()), actionCollection(), "feed_modify");
        KActionMenu* vm = new KActionMenu( i18n( "&View Mode" ), actionCollection(), "mainWidget_mode" );

    KRadioAction *ra = new KRadioAction(i18n("&Normal View"), "mainWidget_top_bottom", "Ctrl+Shift+1", d->mainWidget, SLOT(slotNormalView()), actionCollection(), "normal_mainWidget");
    ra->setExclusiveGroup( "ViewMode" );
    vm->insert(ra);

    ra = new KRadioAction(i18n("&Widescreen View"), "mainWidget_left_right", "Ctrl+Shift+2", d->mainWidget, SLOT(slotWidescreenView()), actionCollection(), "widescreen_mainWidget");
    ra->setExclusiveGroup( "ViewMode" );
    vm->insert(ra);

    ra = new KRadioAction(i18n("C&ombined View"), "mainWidget_text", "Ctrl+Shift+3", d->mainWidget, SLOT(slotCombinedView()), actionCollection(), "combined_mainWidget");
    ra->setExclusiveGroup( "ViewMode" );
    vm->insert(ra);

    // toolbar / feed menu
    new KAction(i18n("&Fetch Feed"), "down", KStdAccel::shortcut(KStdAccel::Reload), d->mainWidget, SLOT(slotFetchCurrentFeed()), actionCollection(), "feed_fetch");
    new KAction(i18n("Fe&tch All Feeds"), "bottom", "Ctrl+L", d->mainWidget, SLOT(slotFetchAllFeeds()), actionCollection(), "feed_fetch_all");

    KAction* stopAction = new KAction(i18n( "&Abort Fetches" ), "stop", Qt::Key_Escape, Kernel::self()->fetchQueue(), SLOT(slotAbort()), actionCollection(), "feed_stop");
    stopAction->setEnabled(false);

    new KAction(i18n("&Mark Feed as Read"), "goto", "Ctrl+R", d->mainWidget, SLOT(slotMarkAllRead()), actionCollection(), "feed_mark_all_as_read");
    new KAction(i18n("Ma&rk All Feeds as Read"), "goto", "Ctrl+Shift+R", d->mainWidget, SLOT(slotMarkAllFeedsRead()), actionCollection(), "feed_mark_all_feeds_as_read");

    // Settings menu
    KToggleAction* sqf = new KToggleAction(i18n("Show Quick Filter"), QString::null, 0, d->mainWidget, SLOT(slotToggleShowQuickFilter()), actionCollection(), "show_quick_filter");
    sqf->setChecked( Settings::showQuickFilter() );

    new KAction( i18n("Open in Tab"), "tab_new", "Shift+Return", d->mainWidget, SLOT(slotOpenCurrentArticle()), actionCollection(), "article_open" );
    new KAction( i18n("Open in Background Tab"), QString::null, "tab_new", d->mainWidget, SLOT(slotOpenCurrentArticleBackgroundTab()), actionCollection(), "article_open_background_tab" );
    new KAction( i18n("Open in External Browser"), "window_new", "Ctrl+Shift+Return", d->mainWidget, SLOT(slotOpenCurrentArticleExternal()), actionCollection(), "article_open_external" );
    new KAction( i18n("Copy Link Address"), "", "", d->mainWidget, SLOT(slotCopyLinkAddress()), actionCollection(), "article_copy_link_address" );

    new KAction(i18n("Pre&vious Unread Article"), "", Qt::Key_Minus, d->mainWidget, SLOT(slotPrevUnreadArticle()),actionCollection(), "go_prev_unread_article");
    new KAction(i18n("Ne&xt Unread Article"), "", Qt::Key_Plus, d->mainWidget, SLOT(slotNextUnreadArticle()),actionCollection(), "go_next_unread_article");

    new KAction(i18n("&Delete"), "editdelete", "Delete", d->mainWidget, SLOT(slotArticleDelete()), actionCollection(), "article_delete");

    d->tagMenu = new KActionMenu ( i18n( "&Set Tags" ), "rss_tag",  actionCollection(), "article_tagmenu" );
    d->tagMenu->setEnabled(false); // only enabled when articles are selected

    KActionMenu* statusMenu = new KActionMenu ( i18n( "&Mark As" ),
                                    actionCollection(), "article_set_status" );

    d->speakSelectedArticlesAction = new KAction(i18n("&Speak Selected Articles"), "kttsd", "", d->mainWidget, SLOT(slotTextToSpeechRequest()), actionCollection(), "akr_texttospeech");
    
    KAction* abortTTS = new KAction(i18n( "&Stop Speaking" ), "player_stop", Qt::Key_Escape, SpeechClient::self(), SLOT(slotAbortJobs()), actionCollection(), "akr_aborttexttospeech");
    abortTTS->setEnabled(false);

    connect(SpeechClient::self(), SIGNAL(signalActivated(bool)),
    abortTTS, SLOT(setEnabled(bool)));

    statusMenu->insert(new KAction(KGuiItem(i18n("&Read"), "",
                       i18n("Mark selected article as read")),
    "Ctrl+E", d->mainWidget, SLOT(slotSetSelectedArticleRead()),
    actionCollection(), "article_set_status_read"));

    statusMenu->insert(new KAction(KGuiItem(i18n("&New"), "",
                        i18n("Mark selected article as new")),
    "Ctrl+N", d->mainWidget, SLOT(slotSetSelectedArticleNew()),
    actionCollection(), "article_set_status_new" ));


    statusMenu->insert(new KAction(KGuiItem(i18n("&Unread"), "",
                       i18n("Mark selected article as unread")),
    "Ctrl+U", d->mainWidget, SLOT(slotSetSelectedArticleUnread()),
    actionCollection(), "article_set_status_unread"));

    KToggleAction* importantAction = new KToggleAction(i18n("&Mark as Important"), "flag", "Ctrl+I", actionCollection(), "article_set_status_important");
    importantAction->setCheckedState(i18n("Remove &Important Mark"));
    connect(importantAction, SIGNAL(toggled(bool)), d->mainWidget, SLOT(slotArticleToggleKeepFlag(bool)));


    new KAction( i18n("Move Node Up"), QString::null, "Shift+Alt+Up", mainWidget, SLOT(slotMoveCurrentNodeUp()), d->actionCollection, "feedstree_move_up" );
    new KAction( i18n("Move Node Down"), QString::null,  "Shift+Alt+Down", mainWidget, SLOT(slotMoveCurrentNodeDown()), d->actionCollection, "feedstree_move_down" );
    new KAction( i18n("Move Node Left"), QString::null, "Shift+Alt+Left", mainWidget, SLOT(slotMoveCurrentNodeLeft()), d->actionCollection, "feedstree_move_left" );
    new KAction( i18n("Move Node Right"), QString::null, "Shift+Alt+Right", mainWidget, SLOT(slotMoveCurrentNodeRight()), d->actionCollection, "feedstree_move_right");

    new KAction(i18n("Send &Link Address..."), "mail_generic", "", mainWidget, SLOT(slotSendLink()), d->actionCollection, "file_sendlink");
    new KAction(i18n("Send &File..."), "mail_generic", "", mainWidget, SLOT(slotSendFile()), d->actionCollection, "file_sendfile");
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

void ActionManagerImpl::initListTabWidget(ListTabWidget* listTabWidget)
{
    if (d->listTabWidget)
        return;
    else
        d->listTabWidget = listTabWidget;

    new KAction(i18n("&Previous Feed"), "", "P", listTabWidget,  SLOT(slotPrevFeed()),actionCollection(), "go_prev_feed");
    new KAction(i18n("&Next Feed"), "", "N", listTabWidget, SLOT(slotNextFeed()),actionCollection(), "go_next_feed");
    new KAction(i18n("N&ext Unread Feed"), "", "Alt+Plus", listTabWidget, SLOT(slotNextUnreadFeed()),actionCollection(), "go_next_unread_feed");
    new KAction(i18n("Prev&ious Unread Feed"), "", "Alt+Minus", listTabWidget, SLOT(slotPrevUnreadFeed()),actionCollection(), "go_prev_unread_feed");

    new KAction( i18n("Go to Top of Tree"), QString::null, "Ctrl+Home", listTabWidget, SLOT(slotItemBegin()), d->actionCollection, "feedstree_home" );
    new KAction( i18n("Go to Bottom of Tree"), QString::null, "Ctrl+End", listTabWidget, SLOT(slotItemEnd()), d->actionCollection, "feedstree_end" );
    new KAction( i18n("Go Left in Tree"), QString::null, "Ctrl+Left", listTabWidget, SLOT(slotItemLeft()), d->actionCollection, "feedstree_left" );
    new KAction( i18n("Go Right in Tree"), QString::null, "Ctrl+Right", listTabWidget, SLOT(slotItemRight()), d->actionCollection, "feedstree_right" );
    new KAction( i18n("Go Up in Tree"), QString::null, "Ctrl+Up", listTabWidget, SLOT(slotItemUp()), d->actionCollection, "feedstree_up" );
    new KAction( i18n("Go Down in Tree"), QString::null, "Ctrl+Down", listTabWidget, SLOT(slotItemDown()), d->actionCollection, "feedstree_down" );
}

void ActionManagerImpl::initTabWidget(TabWidget* tabWidget)
{
    if (d->tabWidget)
        return;
    else
        d->tabWidget = tabWidget;

    new KAction(i18n("Select Next Tab"), "", "Ctrl+Period", d->tabWidget, SLOT(slotNextTab()),actionCollection(), "select_next_tab");
    new KAction(i18n("Select Previous Tab"), "", "Ctrl+Comma", d->tabWidget, SLOT(slotPreviousTab()),actionCollection(), "select_previous_tab");
    new KAction( i18n("Detach Tab"), "tab_breakoff", Qt::CTRL+Qt::SHIFT+Qt::Key_B, d->tabWidget, SLOT(slotDetachTab()), actionCollection(), "tab_detach" );
    new KAction( i18n("Copy Link Address"), "", "", d->tabWidget, SLOT(slotCopyLinkAddress()), actionCollection(), "tab_copylinkaddress" );
    new KAction( i18n("&Close Tab"), "tab_remove", KStdAccel::close(), d->tabWidget, SLOT(slotCloseTab()), actionCollection(), "tab_remove" );
}

void ActionManagerImpl::initFrameManager(FrameManager* frameManager)
{
    if (d->frameManager)
        return;

    d->frameManager = frameManager;

    new KToolBarPopupAction(i18n("Forward"), "forward", "Alt+Right", this, SLOT(slotBrowserForward()), d->actionCollection, "browser_forward");

    new KToolBarPopupAction(i18n("Back"), "back", "Alt+Left", this, SLOT(slotBrowserBack()), d->actionCollection, "browser_back");

    new KAction(i18n("Reload"), "reload", 0,                            frameManager, SLOT(slotBrowserReload()),
                            d->actionCollection, "browser_reload");

    new KAction(i18n("Stop"), "stop", 0, frameManager, SLOT(slotBrowserStop()),
                                 d->actionCollection, "browser_stop");
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
