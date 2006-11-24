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
//#include "speechclient.h"
#include "tabwidget.h"
#include "tag.h"
#include "tagaction.h"
#include "tagnode.h"
#include "tagset.h"
#include "trayicon.h"
#include "treenode.h"
#include "treenodevisitor.h"

#include <kactionmenu.h>
#include <ktoolbarpopupaction.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmenu.h>
#include <kshortcut.h>
#include <kstdaccel.h>
#include <kstdaction.h>
#include <kxmlguifactory.h>
#include <kicon.h> 

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
        d->tagMenu->removeAction(*it);
        delete *it;
    }


    d->tagActions.clear();

    //TODO: remove actions from menus, delete actions, clear maps
    if (tagSet)
    {
        QList<Tag> list = tagSet->toHash().values();
        for (QList<Tag>::ConstIterator it = list.begin(); it != list.end(); ++it)
            slotTagAdded(*it);
    }
}

void ActionManagerImpl::slotTagAdded(const Tag& tag)
{
    if (!d->tagActions.contains(tag.id()))
    {
        d->tagActions[tag.id()] = new TagAction(tag, d->mainWidget, SLOT(slotAssignTag(const Tag&, bool)), d->actionCollection);
        d->tagMenu->addAction(d->tagActions[tag.id()]);
    }
}

void ActionManagerImpl::slotTagRemoved(const Tag& tag)
{
    QString id = tag.id();
    TagAction* action = d->tagActions[id];
    d->tagMenu->removeAction(action);
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
    d->tagMenu = 0;
    d->frameManager = 0;
    d->speakSelectedArticlesAction = 0;
    d->actionCollection = part->actionCollection();
    initPart();
}

ActionManagerImpl::~ActionManagerImpl()
{
    delete d->nodeSelectVisitor;
    delete d;
    d = 0;
}

void ActionManagerImpl::initTrayIcon(TrayIcon* trayIcon)
{
    if (d->trayIcon)
        return;
    else d->trayIcon = trayIcon;

    QMenu* traypop = trayIcon->contextMenu();

    if (actionCollection()->action("feed_fetch_all"))
        actionCollection()->action("feed_fetch_all")->plug(traypop, 1);
    if (actionCollection()->action("akregator_configure_akregator"))
        actionCollection()->action("akregator_configure_akregator")->plug(traypop, 2);
}

void ActionManagerImpl::initPart()
{
    KAction *action = new KAction(i18n("&Import Feeds..."), d->actionCollection, "file_import");
    connect(action, SIGNAL(triggered(bool)), d->part, SLOT(fileImport()));
    action = new KAction(i18n("&Export Feeds..."), d->actionCollection, "file_export");
    connect(action, SIGNAL(triggered(bool)), d->part, SLOT(fileExport()));
    //new KAction(i18n("&Get Feeds From Web..."), "", "", d->part, SLOT(fileGetFeeds()), d->actionCollection, "file_getfromweb");

    KStdAction::configureNotifications(d->part, SLOT(showKNotifyOptions()), d->actionCollection); // options_configure_notifications
    action = new KAction(KIcon("configure"),  i18n("Configure &Akregator..."), d->actionCollection, "akregator_configure_akregator" );
    connect(action, SIGNAL(triggered(bool)), d->part, SLOT(showOptions()));
}

void ActionManagerImpl::initMainWidget(MainWidget* mainWidget)
{
    if (d->mainWidget)
        return;
    else
        d->mainWidget = mainWidget;

    // tag actions
    KAction *action = new KAction(i18n("&New Tag..."), actionCollection(), "tag_new");
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotNewTag()));

    // Feed/Feed Group popup menu
    action = new KAction(i18n("&Open Homepage"), actionCollection(), "feed_homepage");
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotOpenHomepage()));
    action->setShortcut(KShortcut( "Ctrl+H" ));
    action = new KAction(KIcon("bookmark_add"), i18n("&Add Feed..."), actionCollection(), "feed_add");
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotFeedAdd()));
    action->setShortcut(KShortcut( "Insert" ));
    action = new KAction(KIcon("folder_new"), i18n("Ne&w Folder..."), actionCollection(), "feed_add_group");
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotFeedAddGroup()));
    action->setShortcut(KShortcut( "Shift+Insert" ));
    action = new KAction(KIcon("editdelete"), i18n("&Delete Feed"), actionCollection(), "feed_remove");
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotFeedRemove()));
    action->setShortcut(KShortcut( "Alt+Delete" ));
    action = new KAction(KIcon("edit"), i18n("&Edit Feed..."), actionCollection(), "feed_modify");
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotFeedModify()));
    action->setShortcut(KShortcut( "F2" ));
    KActionMenu* vm = new KActionMenu( i18n( "&View Mode" ), actionCollection(), "view_mode" );

    QActionGroup* agViewMode = new QActionGroup(this);
    KToggleAction *ra = new KToggleAction(KIcon("view_top_bottom"), i18n("&Normal View"), actionCollection(), "normal_view");
    connect(ra, SIGNAL(triggered(bool) ), d->mainWidget, SLOT(slotNormalView()));
    ra->setShortcut(KShortcut( "Ctrl+Shift+1" ));
    ra->setActionGroup(agViewMode);
    vm->addAction(ra);

    ra = new KToggleAction(KIcon("view_left_right"), i18n("&Widescreen View"), actionCollection(), "widescreen_view");
    connect(ra, SIGNAL(triggered(bool) ), d->mainWidget, SLOT(slotWidescreenView()));
    ra->setShortcut(KShortcut( "Ctrl+Shift+2" ));
    ra->setActionGroup(agViewMode);
    vm->addAction(ra);

    ra = new KToggleAction(KIcon("view_text"), i18n("C&ombined View"), actionCollection(), "combined_view");
    connect(ra, SIGNAL(triggered(bool) ), d->mainWidget, SLOT(slotCombinedView()));
    ra->setShortcut(KShortcut( "Ctrl+Shift+3" ));
    ra->setActionGroup(agViewMode);
    vm->addAction(ra);

    // toolbar / feed menu
    action = new KAction(KIcon("down"), i18n("&Fetch Feed"), actionCollection(), "feed_fetch");
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotFetchCurrentFeed()));
    action->setShortcut(KStdAccel::shortcut(KStdAccel::Reload));
    action = new KAction(KIcon("bottom"), i18n("Fe&tch All Feeds"), actionCollection(), "feed_fetch_all");
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotFetchAllFeeds()));
    action->setShortcut(KShortcut( "Ctrl+L" ));

    KAction *stopAction = new KAction(KIcon("stop"), i18n( "&Abort Fetches" ), actionCollection(), "feed_stop");
    connect(stopAction, SIGNAL(triggered(bool)), Kernel::self()->fetchQueue(), SLOT(slotAbort()));
    stopAction->setShortcut(QKeySequence(Qt::Key_Escape));
    stopAction->setEnabled(false);

    action = new KAction(KIcon("goto"), i18n("&Mark Feed as Read"), actionCollection(), "feed_mark_all_as_read");
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotMarkAllRead()));
    action->setShortcut(KShortcut( "Ctrl+R" ));
    action = new KAction(KIcon("goto"), i18n("Ma&rk All Feeds as Read"), actionCollection(), "feed_mark_all_feeds_as_read");
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotMarkAllFeedsRead()));
    action->setShortcut(KShortcut( "Ctrl+Shift+R" ));

    // Settings menu
    KToggleAction *sqf = new KToggleAction(i18n("Show Quick Filter"), actionCollection(), "show_quick_filter");
    connect(sqf, SIGNAL(triggered(bool) ), d->mainWidget, SLOT(slotToggleShowQuickFilter()));
    sqf->setChecked( Settings::showQuickFilter() );

    action = new KAction(KIcon("tab_new"),  i18n("Open in Tab"), actionCollection(), "article_open" );
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotOpenCurrentArticle()));
    action->setShortcut(KShortcut( "Shift+Return" ));
    
    action = new KAction(KIcon("window_new"),  i18n("Open in External Browser"), actionCollection(), "article_open_external" );
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotOpenCurrentArticleInBrowser()));
    action->setShortcut(KShortcut( "Ctrl+Shift+Return" ));
    
    action = new KAction( i18n("Copy Link Address"), actionCollection(), "article_copy_link_address" );
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotCopyLinkAddress()));

    action = new KAction(i18n("Pre&vious Unread Article"), actionCollection(), "go_prev_unread_article");
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotPrevUnreadArticle()));
    action->setShortcut(QKeySequence(Qt::Key_Minus));
    action = new KAction(i18n("Ne&xt Unread Article"), actionCollection(), "go_next_unread_article");
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotNextUnreadArticle()));
    action->setShortcut(QKeySequence(Qt::Key_Plus));

    action = new KAction(KIcon("editdelete"), i18n("&Delete"), actionCollection(), "article_delete");
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotArticleDelete()));
    action->setShortcut(KShortcut( "Delete" ));

    d->tagMenu = new KActionMenu ( i18n( "&Set Tags" ),  actionCollection(), "article_tagmenu" );
    d->tagMenu->setIcon(KIcon("rss_tag"));
    d->tagMenu->setEnabled(false); // only enabled when articles are selected


    KActionMenu* statusMenu = new KActionMenu ( i18n( "&Mark As" ),
                                    actionCollection(), "article_set_status" );

    //d->speakSelectedArticlesAction = new KAction(KIcon("kttsd"), i18n("&Speak Selected Articles"), actionCollection(), "akr_texttospeech");
    //connect(d->speakSelectedArticlesAction, SIGNAL(triggered(bool) ), d->mainWidget, SLOT(slotTextToSpeechRequest()));

    //KAction *abortTTS = new KAction(KIcon("player_stop"), i18n( "&Stop Speaking" ), actionCollection(), "akr_aborttexttospeech");
    //connect(abortTTS, SIGNAL(triggered(bool)), SpeechClient::self(), SLOT(slotAbortJobs()));
    //abortTTS->setShortcut(QKeySequence(Qt::Key_Escape));
    //abortTTS->setEnabled(false);

    //connect(SpeechClient::self(), SIGNAL(signalActivated(bool)),
    //abortTTS, SLOT(setEnabled(bool)));

    action = new KAction(i18nc("as in: mark as read","&Read"), actionCollection(), "article_set_status_read");
    action->setToolTip(i18n("Mark selected article as read"));
    action->setShortcut(KShortcut( "Ctrl+E" ));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotSetSelectedArticleRead()));
    statusMenu->addAction(action);

    action = new KAction(i18n("&New"), actionCollection(), "article_set_status_new" );
    action->setShortcut(KShortcut( "Ctrl+N" ));
    action->setToolTip(i18n("Mark selected article as new"));
    connect(action, SIGNAL(triggered(bool)),  d->mainWidget, SLOT(slotSetSelectedArticleNew()));
    statusMenu->addAction(action);


    action = new KAction(i18n("&Unread"), actionCollection(), "article_set_status_unread");
    action->setToolTip(i18n("Mark selected article as unread"));
    action->setShortcut(KShortcut( "Ctrl+U" ));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotSetSelectedArticleUnread()));
    statusMenu->addAction(action);

    KToggleAction* importantAction = new KToggleAction(i18n("&Mark as Important"), "flag", KShortcut( "Ctrl+I" ), actionCollection(), "article_set_status_important");
    importantAction->setCheckedState(KGuiItem(i18n("Remove &Important Mark")));
    connect(importantAction, SIGNAL(toggled(bool)), d->mainWidget, SLOT(slotArticleToggleKeepFlag(bool)));


    action = new KAction( i18n("Move Node Up"), d->actionCollection, "feedstree_move_up" );
    connect(action, SIGNAL(triggered(bool)), mainWidget, SLOT(slotMoveCurrentNodeUp()));
    action->setShortcut(KShortcut( "Shift+Alt+Up" ));
    action = new KAction( i18n("Move Node Down"), d->actionCollection, "feedstree_move_down" );
    connect(action, SIGNAL(triggered(bool)), mainWidget, SLOT(slotMoveCurrentNodeDown()));
    action->setShortcut(KShortcut( "Shift+Alt+Down" ));
    action = new KAction( i18n("Move Node Left"), d->actionCollection, "feedstree_move_left" );
    connect(action, SIGNAL(triggered(bool)), mainWidget, SLOT(slotMoveCurrentNodeLeft()));
    action->setShortcut(KShortcut( "Shift+Alt+Left" ));
    action = new KAction( i18n("Move Node Right"), d->actionCollection, "feedstree_move_right");
    connect(action, SIGNAL(triggered(bool)), mainWidget, SLOT(slotMoveCurrentNodeRight()));
    action->setShortcut(KShortcut( "Shift+Alt+Right" ));

    action = new KAction(KIcon("mail_generic"), i18n("Send &Link Address..."), d->actionCollection, "file_sendlink");
    connect(action, SIGNAL(triggered(bool)), mainWidget, SLOT(slotSendLink()));
    action = new KAction(KIcon("mail_generic"), i18n("Send &File..."), d->actionCollection, "file_sendfile");
    connect(action, SIGNAL(triggered(bool)), mainWidget, SLOT(slotSendFile()));
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

    KAction *action = new KAction( i18n("&Previous Article"), actionCollection(), "go_previous_article" );
    connect(action, SIGNAL(triggered(bool)), articleList, SLOT(slotPreviousArticle()));
    action->setShortcut(KShortcut( "Left" ));
    action = new KAction( i18n("&Next Article"), actionCollection(), "go_next_article" );
    connect(action, SIGNAL(triggered(bool)), articleList, SLOT(slotNextArticle()));
    action->setShortcut(KShortcut( "Right" ));
}

void ActionManagerImpl::initListTabWidget(ListTabWidget* listTabWidget)
{
    if (d->listTabWidget)
        return;
    else
        d->listTabWidget = listTabWidget;

    KAction *action = new KAction(i18n("&Previous Feed"), actionCollection(), "go_prev_feed");
    connect(action, SIGNAL(triggered(bool)), listTabWidget, SLOT(slotPrevFeed()));
    action->setShortcut(KShortcut( "P" ));
    action = new KAction(i18n("&Next Feed"), actionCollection(), "go_next_feed");
    connect(action, SIGNAL(triggered(bool)), listTabWidget, SLOT(slotNextFeed()));
    action->setShortcut(KShortcut( "N" ));
    action = new KAction(i18n("N&ext Unread Feed"), actionCollection(), "go_next_unread_feed");
    connect(action, SIGNAL(triggered(bool)), listTabWidget, SLOT(slotNextUnreadFeed()));
    action->setShortcut(KShortcut( "Alt+Plus" ));
    action = new KAction(i18n("Prev&ious Unread Feed"), actionCollection(), "go_prev_unread_feed");
    connect(action, SIGNAL(triggered(bool)), listTabWidget, SLOT(slotPrevUnreadFeed()));
    action->setShortcut(KShortcut( "Alt+Minus" ));

    action = new KAction( i18n("Go to Top of Tree"), d->actionCollection, "feedstree_home" );
    connect(action, SIGNAL(triggered(bool)), listTabWidget, SLOT(slotItemBegin()));
    action->setShortcut(KShortcut( "Ctrl+Home" ));
    action = new KAction( i18n("Go to Bottom of Tree"), d->actionCollection, "feedstree_end" );
    connect(action, SIGNAL(triggered(bool)), listTabWidget, SLOT(slotItemEnd()));
    action->setShortcut(KShortcut( "Ctrl+End" ));
    action = new KAction( i18n("Go Left in Tree"), d->actionCollection, "feedstree_left" );
    connect(action, SIGNAL(triggered(bool)), listTabWidget, SLOT(slotItemLeft()));
    action->setShortcut(KShortcut( "Ctrl+Left" ));
    action = new KAction( i18n("Go Right in Tree"), d->actionCollection, "feedstree_right" );
    connect(action, SIGNAL(triggered(bool)), listTabWidget, SLOT(slotItemRight()));
    action->setShortcut(KShortcut( "Ctrl+Right" ));
    action = new KAction( i18n("Go Up in Tree"), d->actionCollection, "feedstree_up" );
    connect(action, SIGNAL(triggered(bool)), listTabWidget, SLOT(slotItemUp()));
    action->setShortcut(KShortcut( "Ctrl+Up" ));
    action = new KAction( i18n("Go Down in Tree"), d->actionCollection, "feedstree_down" );
    connect(action, SIGNAL(triggered(bool)), listTabWidget, SLOT(slotItemDown()));
    action->setShortcut(KShortcut( "Ctrl+Down" ));
}

void ActionManagerImpl::initTabWidget(TabWidget* tabWidget)
{
    if (d->tabWidget)
        return;
    else
        d->tabWidget = tabWidget;

    KAction *action = new KAction(i18n("Select Next Tab"), actionCollection(), "select_next_tab");
    connect(action, SIGNAL(triggered(bool)), d->tabWidget, SLOT(slotNextTab()));
    action->setShortcut(KShortcut( "Ctrl+Period" ));
    action = new KAction(i18n("Select Previous Tab"), actionCollection(), "select_previous_tab");
    connect(action, SIGNAL(triggered(bool)), d->tabWidget, SLOT(slotPreviousTab()));
    action->setShortcut(KShortcut( "Ctrl+Comma" ));
    action = new KAction(KIcon("tab_breakoff"),  i18n("Detach Tab"), actionCollection(), "tab_detach" );
    connect(action, SIGNAL(triggered(bool)), d->tabWidget, SLOT(slotDetachTab()));
    action->setShortcut(QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_B));
    action = new KAction( i18n("Copy Link Address"), actionCollection(), "tab_copylinkaddress" );
    connect(action, SIGNAL(triggered(bool)), d->tabWidget, SLOT(slotCopyLinkAddress()));
    action = new KAction(KIcon("tab_remove"),  i18n("&Close Tab"), actionCollection(), "tab_remove" );
    connect(action, SIGNAL(triggered(bool)), d->tabWidget, SLOT(slotCloseTab()));
    action->setShortcut(KStdAccel::close());
}

void ActionManagerImpl::initFrameManager(FrameManager* frameManager)
{
    if (d->frameManager)
        return;

    d->frameManager = frameManager;

    KToolBarPopupAction* forward = new KToolBarPopupAction(i18n("Forward"),"forward", KShortcut( "Alt+Right" ),
                            frameManager, SLOT(slotBrowserForward()),
                            d->actionCollection,"browser_forward");

    connect(forward->menu(), SIGNAL(aboutToShow()), frameManager, SLOT(slotBrowserForwardAboutToShow()));
     
    KToolBarPopupAction* back = new KToolBarPopupAction(i18n("Back"), "back", KShortcut( "Alt+Left" ),
                            frameManager, SLOT(slotBrowserBack()),
                            d->actionCollection, "browser_back");

    connect(back->menu(), SIGNAL(aboutToShow()), frameManager, SLOT(slotBrowserBackAboutToShow()));
    
    KAction *action = new KAction(KIcon("reload"), i18n("Reload"), d->actionCollection, "browser_reload");
    connect(action, SIGNAL(triggered(bool) ), frameManager, SLOT(slotBrowserReload()));

    action = new KAction(KIcon("stop"), i18n("Stop"), d->actionCollection, "browser_stop");
    connect(action, SIGNAL(triggered(bool) ), frameManager, SLOT(slotBrowserStop()));
}

QWidget* ActionManagerImpl::container(const char* name)
{
    return d->part->factory()->container(name, d->part);
}


KActionCollection* ActionManagerImpl::actionCollection()
{
    return d->actionCollection;
}

KAction* ActionManagerImpl::action(const char* name)
{
    return d->actionCollection != 0 ? d->actionCollection->action(name) : 0;
}

} // namespace Akregator

#include "actionmanagerimpl.moc"
