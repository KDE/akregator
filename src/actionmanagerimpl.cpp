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
#include <kstandardshortcut.h>
#include <kstandardaction.h>
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
        QAction* remove = m_manager->action("feed_remove");
        if (remove)
            remove->setEnabled(true);
        QAction* hp = m_manager->action("feed_homepage");
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
        QAction* remove = m_manager->action("feed_remove");
        if (remove)
            remove->setEnabled(node->parent()); // root nodes must not be deleted
        QAction* hp = m_manager->action("feed_homepage");
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
        QAction* remove = m_manager->action("feed_remove");
        if (remove)
            remove->setEnabled(true);
        QAction* hp = m_manager->action("feed_homepage");
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
        disconnect(d->tagSet, SIGNAL(signalTagAdded(const Akregator::Tag&)), this, SLOT(slotTagAdded(const Akregator::Tag&)));
        disconnect(d->tagSet, SIGNAL(signalTagRemoved(const Akregator::Tag&)), this, SLOT(slotTagRemoved(const Akregator::Tag&)));
    }

    d->tagSet = tagSet;

    if (tagSet != 0)
    {
        connect(d->tagSet, SIGNAL(signalTagAdded(const Akregator::Tag&)), this, SLOT(slotTagAdded(const Akregator::Tag&)));
        connect(d->tagSet, SIGNAL(signalTagRemoved(const Akregator::Tag&)), this, SLOT(slotTagRemoved(const Akregator::Tag&)));
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
        d->tagActions[tag.id()] = new TagAction(tag, d->mainWidget, SLOT(slotAssignTag(const Akregator::Tag&, bool)), d->actionCollection);
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
        traypop->addAction(actionCollection()->action("feed_fetch_all"));
    if (actionCollection()->action("akregator_configure_akregator"))
        traypop->addAction(actionCollection()->action("akregator_configure_akregator"));
}

void ActionManagerImpl::initPart()
{
    QAction *action = d->actionCollection->addAction("file_import");
    action->setText(i18n("&Import Feeds..."));
    connect(action, SIGNAL(triggered(bool)), d->part, SLOT(fileImport()));
    action = d->actionCollection->addAction("file_export");
    action->setText(i18n("&Export Feeds..." ));
    connect(action, SIGNAL(triggered(bool)), d->part, SLOT(fileExport()));
    //new KAction(i18n("&Get Feeds From Web..."), "", "", d->part, SLOT(fileGetFeeds()), d->actionCollection, "file_getfromweb");

    KStandardAction::configureNotifications(d->part, SLOT(showKNotifyOptions()), d->actionCollection); // options_configure_notifications
    action = d->actionCollection->addAction("akregator_configure_akregator");
    action->setIcon(KIcon("configure"));
    action->setText(i18n("Configure &Akregator..."));
    connect(action, SIGNAL(triggered(bool)), d->part, SLOT(showOptions()));
}

void ActionManagerImpl::initMainWidget(MainWidget* mainWidget)
{
    if (d->mainWidget)
        return;
    else
        d->mainWidget = mainWidget;

    KActionCollection *coll = actionCollection();

    // tag actions
    QAction *action = coll->addAction("tag_new");
    action->setText(i18n("&New Tag..."));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotNewTag()));

    // Feed/Feed Group popup menu
    action = coll->addAction("feed_homepage");
    action->setText(i18n("&Open Homepage"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotOpenHomepage()));
    action->setShortcuts(KShortcut( "Ctrl+H" ));

    action = coll->addAction("feed_add");
    action->setIcon(KIcon("bookmark-new"));
    action->setText(i18n("&Add Feed..."));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotFeedAdd()));
    action->setShortcuts(KShortcut( "Insert" ));

    action = coll->addAction("feed_add_group");
    action->setIcon(KIcon("folder-new"));
    action->setText(i18n("Ne&w Folder..."));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotFeedAddGroup()));
    action->setShortcuts(KShortcut( "Shift+Insert" ));

    action = coll->addAction("feed_remove");
    action->setIcon(KIcon("edit-delete"));
    action->setText(i18n("&Delete Feed"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotFeedRemove()));
    action->setShortcuts(KShortcut( "Alt+Delete" ));

    action = coll->addAction("feed_modify");
    action->setIcon(KIcon("edit"));
    action->setText(i18n("&Edit Feed..."));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotFeedModify()));
    action->setShortcuts(KShortcut( "F2" ));

    KActionMenu* vm = coll->add<KActionMenu>("view_mode");
    vm->setText(i18n("&View Mode"));

    QActionGroup* agViewMode = new QActionGroup(this);
    KToggleAction *ra = coll->add<KToggleAction>("normal_view");
    ra->setIcon(KIcon("view-top-bottom"));
    ra->setText(i18n("&Normal View"));
    connect(ra, SIGNAL(triggered(bool) ), d->mainWidget, SLOT(slotNormalView()));
    ra->setShortcuts(KShortcut( "Ctrl+Shift+1" ));
    ra->setActionGroup(agViewMode);
    vm->addAction(ra);

    ra = coll->add<KToggleAction>("widescreen_view");
    ra->setIcon(KIcon("view-left-right"));
    ra->setText(i18n("&Widescreen View"));
    connect(ra, SIGNAL(triggered(bool) ), d->mainWidget, SLOT(slotWidescreenView()));
    ra->setShortcuts(KShortcut( "Ctrl+Shift+2" ));
    ra->setActionGroup(agViewMode);
    vm->addAction(ra);

    ra = coll->add<KToggleAction>("combined_view");
    ra->setIcon(KIcon("fileview-text"));
    ra->setText(i18n("C&ombined View"));
    connect(ra, SIGNAL(triggered(bool) ), d->mainWidget, SLOT(slotCombinedView()));
    ra->setShortcuts(KShortcut( "Ctrl+Shift+3" ));
    ra->setActionGroup(agViewMode);
    vm->addAction(ra);

    // toolbar / feed menu
    action = coll->addAction("feed_fetch");
    action->setIcon(KIcon("go-down"));
    action->setText(i18n("&Fetch Feed"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotFetchCurrentFeed()));
    action->setShortcuts(KStandardShortcut::shortcut(KStandardShortcut::Reload));

    action = coll->addAction("feed_fetch_all");
    action->setIcon(KIcon("go-bottom"));
    action->setText(i18n("Fe&tch All Feeds"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotFetchAllFeeds()));
    action->setShortcuts(KShortcut( "Ctrl+L" ));

    QAction *stopAction = coll->addAction("feed_stop");
    stopAction->setIcon(KIcon("process-stop"));
    stopAction->setText(i18n("&Abort Fetches"));
    connect(stopAction, SIGNAL(triggered(bool)), Kernel::self()->fetchQueue(), SLOT(slotAbort()));
    stopAction->setShortcut(QKeySequence(Qt::Key_Escape));
    stopAction->setEnabled(false);

    action = coll->addAction("feed_mark_all_as_read");
    action->setIcon(KIcon("goto-page"));
    action->setText(i18n("&Mark Feed as Read"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotMarkAllRead()));
    action->setShortcuts(KShortcut( "Ctrl+R" ));

    action = coll->addAction("feed_mark_all_feeds_as_read");
    action->setIcon(KIcon("goto-page"));
    action->setText(i18n("Ma&rk All Feeds as Read"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotMarkAllFeedsRead()));
    action->setShortcuts(KShortcut( "Ctrl+Shift+R" ));

    // Settings menu
    KToggleAction *sqf = coll->add<KToggleAction>("show_quick_filter");
    sqf->setText(i18n("Show Quick Filter"));
    connect(sqf, SIGNAL(triggered(bool) ), d->mainWidget, SLOT(slotToggleShowQuickFilter()));
    sqf->setChecked( Settings::showQuickFilter() );

    action = coll->addAction("article_open" );
    action->setIcon(KIcon("tab-new"));
    action->setText(i18n("Open in Tab"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotOpenCurrentArticle()));
    action->setShortcuts(KShortcut( "Shift+Return" ));
    
    action = coll->addAction("article_open_external" );
    action->setIcon(KIcon("window-new"));
    action->setText(i18n("Open in External Browser"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotOpenCurrentArticleInBrowser()));
    action->setShortcuts(KShortcut( "Ctrl+Shift+Return" ));
    
    action = coll->addAction("article_copy_link_address" );
    action->setText(i18n("Copy Link Address"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotCopyLinkAddress()));

    action = coll->addAction("go_prev_unread_article");
    action->setText(i18n("Pre&vious Unread Article"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotPrevUnreadArticle()));
    action->setShortcut(QKeySequence(Qt::Key_Minus));

    action = coll->addAction("go_next_unread_article");
    action->setText(i18n("Ne&xt Unread Article"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotNextUnreadArticle()));
    action->setShortcut(QKeySequence(Qt::Key_Plus));

    action = coll->addAction("article_delete");
    action->setIcon(KIcon("edit-delete"));
    action->setText(i18n("&Delete"));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotArticleDelete()));
    action->setShortcuts(KShortcut( "Delete" ));

    d->tagMenu = coll->add<KActionMenu>("article_tagmenu");
    d->tagMenu->setText(i18n("&Set Tags"));
    d->tagMenu->setIcon(KIcon("rss-tag"));
    d->tagMenu->setEnabled(false); // only enabled when articles are selected


    KActionMenu* statusMenu = coll->add<KActionMenu>("article_set_status");
    statusMenu->setText(i18n("&Mark As"));

    //d->speakSelectedArticlesAction = new KAction(KIcon("kttsd"), i18n("&Speak Selected Articles"), actionCollection(), "akr_texttospeech");
    //connect(d->speakSelectedArticlesAction, SIGNAL(triggered(bool) ), d->mainWidget, SLOT(slotTextToSpeechRequest()));

    //KAction *abortTTS = new KAction(KIcon("media-playback-stop"), i18n( "&Stop Speaking" ), actionCollection(), "akr_aborttexttospeech");
    //connect(abortTTS, SIGNAL(triggered(bool)), SpeechClient::self(), SLOT(slotAbortJobs()));
    //abortTTS->setShortcut(QKeySequence(Qt::Key_Escape));
    //abortTTS->setEnabled(false);

    //connect(SpeechClient::self(), SIGNAL(signalActivated(bool)),
    //abortTTS, SLOT(setEnabled(bool)));

    action = coll->addAction("article_set_status_read");
    action->setText(i18nc("as in: mark as read","&Read"));
    action->setToolTip(i18n("Mark selected article as read"));
    action->setShortcuts(KShortcut( "Ctrl+E" ));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotSetSelectedArticleRead()));
    statusMenu->addAction(action);

    action = coll->addAction("article_set_status_new");
    action->setText(i18n("&New"));
    action->setShortcuts(KShortcut( "Ctrl+N" ));
    action->setToolTip(i18n("Mark selected article as new"));
    connect(action, SIGNAL(triggered(bool)),  d->mainWidget, SLOT(slotSetSelectedArticleNew()));
    statusMenu->addAction(action);


    action = coll->addAction("article_set_status_unread");
    action->setText(i18n("&Unread"));
    action->setToolTip(i18n("Mark selected article as unread"));
    action->setShortcuts(KShortcut( "Ctrl+U" ));
    connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotSetSelectedArticleUnread()));
    statusMenu->addAction(action);

    KToggleAction* importantAction = coll->add<KToggleAction>("article_set_status_important");
    importantAction->setText(i18n("&Mark as Important"));
    importantAction->setIcon(KIcon("flag"));
    importantAction->setShortcuts(KShortcut("Ctrl+I"));
    importantAction->setCheckedState(KGuiItem(i18n("Remove &Important Mark")));
    connect(importantAction, SIGNAL(toggled(bool)), d->mainWidget, SLOT(slotArticleToggleKeepFlag(bool)));


    action = coll->addAction("feedstree_move_up");
    action->setText(i18n("Move Node Up"));
    connect(action, SIGNAL(triggered(bool)), mainWidget, SLOT(slotMoveCurrentNodeUp()));
    action->setShortcuts(KShortcut( "Shift+Alt+Up" ));

    action = coll->addAction("feedstree_move_down");
    action->setText(i18n("Move Node Down"));
    connect(action, SIGNAL(triggered(bool)), mainWidget, SLOT(slotMoveCurrentNodeDown()));
    action->setShortcuts(KShortcut( "Shift+Alt+Down" ));

    action = coll->addAction(i18n("Move Node Left"));
    action->setText(i18n("Move Node Left"));
    connect(action, SIGNAL(triggered(bool)), mainWidget, SLOT(slotMoveCurrentNodeLeft()));
    action->setShortcuts(KShortcut( "Shift+Alt+Left" ));

    action = coll->addAction("feedstree_move_right");
    action->setText(i18n("Move Node Right"));
    connect(action, SIGNAL(triggered(bool)), mainWidget, SLOT(slotMoveCurrentNodeRight()));
    action->setShortcuts(KShortcut( "Shift+Alt+Right" ));

    action = coll->addAction("file_sendlink");
    action->setIcon(KIcon("mail"));
    action->setText(i18n("Send &Link Address..."));
    connect(action, SIGNAL(triggered(bool)), mainWidget, SLOT(slotSendLink()));

    action = coll->addAction("file_sendfile");
    action->setIcon(KIcon("mail"));
    action->setText(i18n("Send &File..."));
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

    QAction *action = actionCollection()->addAction("go_previous_article");
    action->setText(i18n("&Previous Article"));
    connect(action, SIGNAL(triggered(bool)), articleList, SLOT(slotPreviousArticle()));
    action->setShortcuts(KShortcut( "Left" ));
    action = actionCollection()->addAction("go_next_article");
    action->setText(i18n("&Next Article"));
    connect(action, SIGNAL(triggered(bool)), articleList, SLOT(slotNextArticle()));
    action->setShortcuts(KShortcut( "Right" ));
}

void ActionManagerImpl::initListTabWidget(ListTabWidget* listTabWidget)
{
    if (d->listTabWidget)
        return;
    else
        d->listTabWidget = listTabWidget;

    KActionCollection *coll = actionCollection();

    QAction *action = coll->addAction("go_prev_feed");
    action->setText(i18n("&Previous Feed"));
    connect(action, SIGNAL(triggered(bool)), listTabWidget, SLOT(slotPrevFeed()));
    action->setShortcuts(KShortcut( "P" ));

    action = coll->addAction("go_next_feed");
    action->setText(i18n("&Next Feed"));
    connect(action, SIGNAL(triggered(bool)), listTabWidget, SLOT(slotNextFeed()));
    action->setShortcuts(KShortcut( "N" ));

    action = coll->addAction("go_next_unread_feed");
    action->setText(i18n("N&ext Unread Feed"));
    connect(action, SIGNAL(triggered(bool)), listTabWidget, SLOT(slotNextUnreadFeed()));
    action->setShortcuts(KShortcut( "Alt+Plus" ));

    action = coll->addAction("go_prev_unread_feed");
    action->setText(i18n("Prev&ious Unread Feed"));
    connect(action, SIGNAL(triggered(bool)), listTabWidget, SLOT(slotPrevUnreadFeed()));
    action->setShortcuts(KShortcut( "Alt+Minus" ));

    action = coll->addAction("feedstree_home");
    action->setText(i18n("Go to Top of Tree"));
    connect(action, SIGNAL(triggered(bool)), listTabWidget, SLOT(slotItemBegin()));
    action->setShortcuts(KShortcut( "Ctrl+Home" ));

    action = coll->addAction("feedstree_end");
    action->setText(i18n("Go to Bottom of Tree"));
    connect(action, SIGNAL(triggered(bool)), listTabWidget, SLOT(slotItemEnd()));
    action->setShortcuts(KShortcut( "Ctrl+End" ));

    action = coll->addAction("feedstree_left");
    action->setText(i18n("Go Left in Tree"));
    connect(action, SIGNAL(triggered(bool)), listTabWidget, SLOT(slotItemLeft()));
    action->setShortcuts(KShortcut( "Ctrl+Left" ));

    action = coll->addAction("feedstree_right");
    action->setText(i18n("Go Right in Tree"));
    connect(action, SIGNAL(triggered(bool)), listTabWidget, SLOT(slotItemRight()));
    action->setShortcuts(KShortcut( "Ctrl+Right" ));

    action = coll->addAction("feedstree_up");
    action->setText(i18n("Go Up in Tree"));
    connect(action, SIGNAL(triggered(bool)), listTabWidget, SLOT(slotItemUp()));
    action->setShortcuts(KShortcut( "Ctrl+Up" ));

    action = coll->addAction("feedstree_down" );
    action->setText(i18n("Go Down in Tree"));
    connect(action, SIGNAL(triggered(bool)), listTabWidget, SLOT(slotItemDown()));
    action->setShortcuts(KShortcut( "Ctrl+Down" ));
}

void ActionManagerImpl::initTabWidget(TabWidget* tabWidget)
{
    if (d->tabWidget)
        return;
    else
        d->tabWidget = tabWidget;

    KActionCollection *coll = actionCollection();

    QAction *action = coll->addAction("select_next_tab");
    action->setText(i18n("Select Next Tab"));
    connect(action, SIGNAL(triggered(bool)), d->tabWidget, SLOT(slotNextTab()));
    action->setShortcuts(KShortcut( "Ctrl+Period" ));

    action = coll->addAction("select_previous_tab");
    action->setText(i18n("Select Previous Tab"));
    connect(action, SIGNAL(triggered(bool)), d->tabWidget, SLOT(slotPreviousTab()));
    action->setShortcuts(KShortcut( "Ctrl+Comma" ));

    action = coll->addAction("tab_detach");
    action->setIcon(KIcon("tab-breakoff"));
    action->setText(i18n("Detach Tab"));
    connect(action, SIGNAL(triggered(bool)), d->tabWidget, SLOT(slotDetachTab()));
    action->setShortcut(QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_B));

    action = coll->addAction("tab_copylinkaddress");
    action->setText(i18n("Copy Link Address"));
    connect(action, SIGNAL(triggered(bool)), d->tabWidget, SLOT(slotCopyLinkAddress()));

    action = coll->addAction("tab_remove");
    action->setIcon(KIcon("tab-remove"));
    action->setText(i18n("&Close Tab"));
    connect(action, SIGNAL(triggered(bool)), d->tabWidget, SLOT(slotCloseTab()));
    action->setShortcuts(KStandardShortcut::close());
}

void ActionManagerImpl::initFrameManager(FrameManager* frameManager)
{
    if (d->frameManager)
        return;

    d->frameManager = frameManager;

    KToolBarPopupAction* forward = new KToolBarPopupAction(KIcon("go-next"), i18n( "Forward"), this);
    d->actionCollection->addAction("browser_forward", forward);
    forward->setShortcuts(KShortcut("Alt+Right"));
    connect(forward, SIGNAL(triggered()), frameManager, SLOT(slotBrowserForward()));

    connect(forward->menu(), SIGNAL(aboutToShow()), frameManager, SLOT(slotBrowserForwardAboutToShow()));
     
    KToolBarPopupAction* back = new KToolBarPopupAction(KIcon("go-previous"), i18n( "Back"), this);
    d->actionCollection->addAction("browser_back", back);
    back->setShortcuts(KShortcut("Alt+Left"));
    connect(back, SIGNAL(triggered()), frameManager, SLOT(slotBrowserBack()));

    connect(back->menu(), SIGNAL(aboutToShow()), frameManager, SLOT(slotBrowserBackAboutToShow()));
    
    QAction *action = d->actionCollection->addAction("browser_reload");
    action->setIcon(KIcon("view-refresh"));
    action->setText(i18n("Reload"));
    connect(action, SIGNAL(triggered(bool) ), frameManager, SLOT(slotBrowserReload()));

    action = d->actionCollection->addAction("browser_stop");
    action->setIcon(KIcon("process-stop"));
    action->setText(i18n("Stop"));
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

QAction* ActionManagerImpl::action(const char* name)
{
    return d->actionCollection != 0 ? d->actionCollection->action(name) : 0;
}

} // namespace Akregator

#include "actionmanagerimpl.moc"
