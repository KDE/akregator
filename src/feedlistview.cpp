/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

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

#include "folder.h"
#include "folderitem.h"
#include "feedlistview.h"
#include "feed.h"
#include "feeditem.h"
#include "feedlist.h"
#include "treenode.h"
#include "treenodeitem.h"
#include "treenodevisitor.h"

#include <kdebug.h>
#include <kiconeffect.h>
#include <kiconloader.h>
#include <klocale.h>
#include <k3multipledrag.h>
#include <kstringhandler.h>
#include <k3urldrag.h>

#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFont>
#include <QHash>
#include <QList>
#include <QPainter>
#include <QPixmap>
#include <QTimer>

namespace Akregator {

class FeedListView::FeedListViewPrivate
{
    public:
/** used for finding the item belonging to a node */
    QHash<TreeNode*, TreeNodeItem*> itemDict;
    FeedList* nodeList;

    // Drag and Drop variables
    Q3ListViewItem *parent;
    Q3ListViewItem *afterme;
    QTimer autoopentimer;
    ConnectNodeVisitor* connectNodeVisitor;
    DisconnectNodeVisitor* disconnectNodeVisitor;
    CreateItemVisitor* createItemVisitor;
    DeleteItemVisitor* deleteItemVisitor;
};

class FeedListView::ConnectNodeVisitor : public TreeNodeVisitor
{
    public:
        ConnectNodeVisitor(FeedListView* view) : m_view(view) {}

        virtual bool visitTreeNode(TreeNode* node)
        {
            connect(node, SIGNAL(signalDestroyed(Akregator::TreeNode*)), m_view, SLOT(slotNodeDestroyed(Akregator::TreeNode*) ));
            connect(node, SIGNAL(signalChanged(Akregator::TreeNode*)), m_view, SLOT(slotNodeChanged(Akregator::TreeNode*) ));
            return true;
        }

        virtual bool visitFolder(Folder* node)
        {
            visitTreeNode(node);
            connect(node, SIGNAL(signalChildAdded(Akregator::TreeNode*)), m_view, SLOT(slotNodeAdded(Akregator::TreeNode*) ));
            connect(node, SIGNAL(signalChildRemoved(Akregator::Folder*, Akregator::TreeNode*)), m_view, SLOT(slotNodeRemoved(Akregator::Folder*, Akregator::TreeNode*) ));
            return true;
        }
        
        virtual bool visitFeed(Feed* node)
        {
            visitTreeNode(node);
            
            connect(node, SIGNAL(fetchStarted(Akregator::Feed*)), m_view, SLOT(slotFeedFetchStarted(Akregator::Feed*)));
            connect(node, SIGNAL(fetchAborted(Akregator::Feed*)), m_view, SLOT(slotFeedFetchAborted(Akregator::Feed*)));
            connect(node, SIGNAL(fetchError(Akregator::Feed*)), m_view, SLOT(slotFeedFetchError(Akregator::Feed*)));
            connect(node, SIGNAL(fetched(Akregator::Feed*)), m_view, SLOT(slotFeedFetchCompleted(Akregator::Feed*)));
            return true;
        }
    private:

        FeedListView* m_view;
    
};

class FeedListView::DisconnectNodeVisitor : public TreeNodeVisitor
{
    public:
        DisconnectNodeVisitor(FeedListView* view) : m_view(view) {}

        virtual bool visitFolder(Folder* node)
        {
            disconnect(node, SIGNAL(signalChildAdded(Akregator::TreeNode*)), m_view, SLOT(slotNodeAdded(Akregator::TreeNode*) ));
            disconnect(node, SIGNAL(signalChildRemoved(Akregator::Folder*, Akregator::TreeNode*)), m_view, SLOT(slotNodeRemoved(Akregator::Folder*, Akregator::TreeNode*) ));
            
            disconnect(node, SIGNAL(signalDestroyed(Akregator::TreeNode*)), m_view, SLOT(slotNodeDestroyed(Akregator::TreeNode*) ));
            disconnect(node, SIGNAL(signalChanged(Akregator::TreeNode*)), m_view, SLOT(slotNodeChanged(Akregator::TreeNode*) ));
            return true;
        }
        
        virtual bool visitFeed(Feed* node)
        {

            disconnect(node, SIGNAL(signalDestroyed(Akregator::TreeNode*)), m_view, SLOT(slotNodeDestroyed(Akregator::TreeNode*) ));
            disconnect(node, SIGNAL(signalChanged(Akregator::TreeNode*)), m_view, SLOT(slotNodeChanged(Akregator::TreeNode*) ));
            disconnect(node, SIGNAL(fetchStarted(Akregator::Feed*)), m_view, SLOT(slotFeedFetchStarted(Akregator::Feed*)));
            disconnect(node, SIGNAL(fetchAborted(Akregator::Feed*)), m_view, SLOT(slotFeedFetchAborted(Akregator::Feed*)));
            disconnect(node, SIGNAL(fetchError(Akregator::Feed*)), m_view, SLOT(slotFeedFetchError(Akregator::Feed*)));
            disconnect(node, SIGNAL(fetched(Akregator::Feed*)), m_view, SLOT(slotFeedFetchCompleted(Akregator::Feed*)));
            return true;
        }
    private:

        FeedListView* m_view;
};

class FeedListView::DeleteItemVisitor : public TreeNodeVisitor
{
    public:
        
        DeleteItemVisitor(FeedListView* view) : m_view(view) {}
        
        virtual bool visitTreeNode(TreeNode* node)
        {
            TreeNodeItem* item = m_view->d->itemDict.take(node);
    
            if (!item)
                return true;
    
            if ( m_selectNeighbour && item->isSelected() )
            {
                if (item->itemBelow())
                    m_view->setSelected(item->itemBelow(), true);
                else if (item->itemAbove())
                    m_view->setSelected(item->itemAbove(), true);
                else
                    m_view->setSelected(item, false);
            }
            
            m_view->disconnectFromNode(node);
            delete item;
            return true;
        
        }
        
        virtual bool visitFolder(Folder* node)
        {
            // delete child items recursively before deleting parent
            QList<TreeNode*> children = node->children();
            for (QList<TreeNode*>::ConstIterator it =  children.begin(); it != children.end(); ++it )
                visit(*it);
            
            visitTreeNode(node);
            
            return true;
        }
        
        void deleteItem(TreeNode* node, bool selectNeighbour)
        {
            m_selectNeighbour = selectNeighbour;
            visit(node);
        }
        
    private:
        FeedListView* m_view;
        bool m_selectNeighbour;
};


class FeedListView::CreateItemVisitor : public TreeNodeVisitor
{
    public:
        CreateItemVisitor(FeedListView* view) : m_view(view) {}

        virtual bool visitFolder(Folder* node)
        {
            if (m_view->findNodeItem(node))
                return true;

            FolderItem* item = 0;
            TreeNode* prev = node->prevSibling();
            FolderItem* parentItem = static_cast<FolderItem*>(m_view->findNodeItem(node->parent()));
            if (parentItem)
            {
                if (prev)
                {
                    item = new FolderItem( parentItem, m_view->findNodeItem(prev), node);
                }
                else
                    item = new FolderItem(parentItem, node);
            }
            else
            {
                if (prev)
                {
                    item = new FolderItem(m_view, m_view->findNodeItem(prev), node);
                }
                else
                    item = new FolderItem(m_view, node);
            }
            m_view->d->itemDict.insert(node, item);
            QList<TreeNode*> children = node->children();

            // add children recursively
            for (QList<TreeNode*>::ConstIterator it =  children.begin(); it != children.end(); ++it )
                visit(*it);

            m_view->connectToNode(node);
            return true;
        }
        
        virtual bool visitFeed(Feed* node)
        {
            if (m_view->findNodeItem(node))
                return true;

            FeedItem* item = 0;
            TreeNode* prev = node->prevSibling();
            FolderItem* parentItem = static_cast<FolderItem*>(m_view->findNodeItem(node->parent()));
            
            if (parentItem)
            {
                if (prev)
                {
                    item = new FeedItem( parentItem, m_view->findNodeItem(prev), node);
                }
                else
                    item = new FeedItem( parentItem, node);
            }
            else
            {
                if (prev)
                {
                    item = new FeedItem(m_view, m_view->findNodeItem(prev), node);
                }
                else
                    item = new FeedItem(m_view, node);
            }

            item->nodeChanged();     
            m_view->d->itemDict.insert(node, item);
            m_view->connectToNode(node);
            return true;
        }
        
    private:
        FeedListView* m_view;
};

FeedListView::FeedListView( QWidget *parent, const char *name)
        : K3ListView(parent), d(new FeedListViewPrivate)
{
    setObjectName(name);
    d->connectNodeVisitor = new ConnectNodeVisitor(this),
    d->disconnectNodeVisitor = new DisconnectNodeVisitor(this);
    d->createItemVisitor = new CreateItemVisitor(this);
    d->deleteItemVisitor = new DeleteItemVisitor(this);
    
    setMinimumSize(150, 150);
    addColumn(i18n("Feeds"));
    setRootIsDecorated(false);
    setItemsRenameable(false); // NOTE: setting this this to true collides with setRenameEnabled() in items and breaks in-place renaming in strange ways. Do not enable!
    setItemMargin(2);

    setFullWidth(true);
    setSorting(-1);
    setDragAutoScroll(true);
    setDropVisualizer(true);
    //setDropHighlighter(false);

    setDragEnabled(true);
    setAcceptDrops(true);
    setItemsMovable(true);
    
    connect( this, SIGNAL(dropped(QDropEvent*, Q3ListViewItem*)), this, SLOT(slotDropped(QDropEvent*, Q3ListViewItem*)) );
    connect( this, SIGNAL(selectionChanged(Q3ListViewItem*)), this, SLOT(slotSelectionChanged(Q3ListViewItem*)) );
    connect( this, SIGNAL(itemRenamed(Q3ListViewItem*, int, const QString&)), this, SLOT(slotItemRenamed(Q3ListViewItem*, int, const QString&)) );
    connect( this, SIGNAL(contextMenu(K3ListView*, Q3ListViewItem*, const QPoint&)), this, SLOT(slotContextMenu(K3ListView*, Q3ListViewItem*, const QPoint&)) );
    connect( &(d->autoopentimer), SIGNAL( timeout() ), this, SLOT( openFolder() ) );

    clear();
    
    this->setWhatsThis( i18n("<h2>Feeds tree</h2>"
        "Here you can browse tree of feeds. "
        "You can also add feeds or feed groups (folders) "
        "using right-click menu, or reorganize them using "
        "drag and drop."));
    setUpdatesEnabled(true);
}

FeedListView::~FeedListView()
{
    delete d->connectNodeVisitor;
    delete d->disconnectNodeVisitor;
    delete d->createItemVisitor;
    delete d->deleteItemVisitor;
    delete d;
    d = 0;
}

void FeedListView::setFeedList(FeedList* nodeList)
{
    if (nodeList == d->nodeList)
         return;

    clear();

    disconnectFromFeedList(d->nodeList);
    
    if (!nodeList)
        return;

    d->nodeList = nodeList;
    connectToFeedList(nodeList);
  
    
    Folder* rootNode = nodeList->rootNode();
    if (!rootNode)
        return;

    slotNodeAdded(rootNode);
    slotRootNodeChanged(rootNode);
}

void FeedListView::takeNode(Q3ListViewItem* item)
{
    if (item->parent())
        item->parent()->takeItem(item);
    else
        takeItem(item);
}

void FeedListView::insertNode(Q3ListViewItem* parent, Q3ListViewItem* item, Q3ListViewItem* after)
{
    if (parent)
        parent->insertItem(item);
    else
        insertItem(item);
    if (after)
        item->moveItem(after);
}

Folder* FeedListView::rootNode()
{
    return d->nodeList ? d->nodeList->rootNode() : 0;
}

TreeNode* FeedListView::selectedNode()
{
    TreeNodeItem* item = dynamic_cast<TreeNodeItem*> (selectedItem());
    
    return ( item ? item->node() : 0) ;
}

void FeedListView::setSelectedNode(TreeNode* node)
{
    TreeNodeItem* item = findNodeItem(node);
    if ( node && item )
        setSelected(item, true);
}

TreeNode* FeedListView::findNodeByTitle(const QString& title)
{
    TreeNodeItem* item = dynamic_cast<TreeNodeItem*>(findItemByTitle(title, 0));
    if (!item)
        return 0;
    else 
        return item->node();
}

TreeNodeItem* FeedListView::findNodeItem(TreeNode* node)
{
    return d->itemDict[node];
}

TreeNodeItem* FeedListView::findItemByTitle(const QString& text, int column, ComparisonFlags compare) const
{ 
    return dynamic_cast<TreeNodeItem*> (K3ListView::findItem(text, column, compare)); 
}

void FeedListView::ensureNodeVisible(TreeNode* node)
{
    ensureItemVisible(findNodeItem(node));
}

void FeedListView::startNodeRenaming(TreeNode* node)
{
    TreeNodeItem* item = findNodeItem(node);
    if (item)
    {   
        item->startRename(0);
    }
}

void FeedListView::clear()
{
    foreach(TreeNode *node, d->itemDict.keys())
        disconnectFromNode(node);
    d->itemDict.clear();
    d->nodeList = 0;
    
    K3ListView::clear();
}

void FeedListView::drawContentsOffset( QPainter * p, int ox, int oy,
                                       int cx, int cy, int cw, int ch )
{
    bool oldUpdatesEnabled = updatesEnabled();
    setUpdatesEnabled(false);
    K3ListView::drawContentsOffset( p, ox, oy, cx, cy, cw, ch );
    setUpdatesEnabled(oldUpdatesEnabled);
}

void FeedListView::slotDropped( QDropEvent *e, Q3ListViewItem*
/*after*/)
{
	d->autoopentimer.stop();

    if (e->source() != viewport())
    {
        openFolder();
        const QMimeData *md = e->mimeData();
        if (KUrl::List::canDecode(md))
        {
            FolderItem* parent = dynamic_cast<FolderItem*> (d->parent);
            TreeNodeItem* afterMe = 0;

            if(d->afterme)
                afterMe = dynamic_cast<TreeNodeItem*> (d->afterme);
        
            KUrl::List urls = KUrl::List::fromMimeData( md );
            e->accept();
            emit signalDropped( urls, afterMe ? afterMe->node() : 0, parent ? parent->node() : 0);
        }
    }
    else
    {
    }
}

void FeedListView::movableDropEvent(Q3ListViewItem* /*parent*/, Q3ListViewItem* /*afterme*/)
{
	d->autoopentimer.stop();
    if (d->parent)
    {    
        openFolder();

        Folder* parentNode = (dynamic_cast<FolderItem*> (d->parent))->node();
        TreeNode* afterMeNode = 0; 
        TreeNode* current = selectedNode();

        if (d->afterme)
            afterMeNode = (dynamic_cast<TreeNodeItem*> (d->afterme))->node();

        current->parent()->removeChild(current);
        parentNode->insertChild(current, afterMeNode);
        K3ListView::movableDropEvent(d->parent, d->afterme);
    }    
}


void FeedListView::contentsDragMoveEvent(QDragMoveEvent* event)
{
    QPoint vp = contentsToViewport(event->pos());
    Q3ListViewItem *i = itemAt(vp);

    Q3ListViewItem *qiparent;
    Q3ListViewItem *qiafterme;
    findDrop( event->pos(), qiparent, qiafterme );

    if (event->source() == viewport()) {
        // disable any drops where the result would be top level nodes 
        if (i && !i->parent())
        {
            event->ignore();
            d->autoopentimer.stop();
            return;
        }

        // prevent dragging nodes from All Feeds to My Tags or vice versa
        Q3ListViewItem* root1 = i;
        while (root1 && root1->parent())
            root1 = root1->parent();

        Q3ListViewItem* root2 = selectedItem();
        while (root2 && root2->parent())
            root2 = root2->parent();

        if (root1 != root2)
        {
            event->ignore();
            d->autoopentimer.stop();
            return;
        }

        // don't drop node into own subtree
        Q3ListViewItem* p = qiparent;
        while (p)
            if (p == selectedItem())
            {
                event->ignore();
                d->autoopentimer.stop();
                return;
            }
            else
            {
                p = p->parent();
            }

        // disable drags onto the item itself
        if (selectedItem() == i)
        {
            event->ignore();
            d->autoopentimer.stop();
            return;
        }
    }

    // what the hell was this good for? -fo
    //    if (!i || event->pos().x() > header()->cellPos(header()->mapToIndex(0)) +
    //            treeStepSize() * (i->depth() + 1) + itemMargin() ||
    //            event->pos().x() < header()->cellPos(header()->mapToIndex(0)))
    //   {} else
 
    // do we want to move inside the old parent or do we want to move to a new parent
    if (i && (itemAt(vp - QPoint(0,5)) == i && itemAt(vp + QPoint(0,5)) == i))
    {
        setDropVisualizer(false);
        setDropHighlighter(true);
        cleanDropVisualizer();

        TreeNode *iNode = (dynamic_cast<TreeNodeItem*> (i))->node();
        if (iNode->isGroup())
        {
            if (i != d->parent)
                d->autoopentimer.start(750);

            d->parent = i;
            d->afterme = 0;
        }
        else
        {
            event->ignore();
            d->autoopentimer.stop();
            d->afterme = i;
            return;
        }
    }
    else
    {
        setDropVisualizer(true);
        setDropHighlighter(false);
        cleanItemHighlighter();
        d->parent = qiparent;
        d->afterme = qiafterme;
        d->autoopentimer.stop();
    }

    // the rest is handled by K3ListView.
    K3ListView::contentsDragMoveEvent(event);
}

bool FeedListView::acceptDrag(QDropEvent *e) const
{
    if (!acceptDrops() || !itemsMovable())
        return false;

    if (e->source() != viewport())
    {
        const QMimeData *md = e->mimeData();
        return KUrl::List::canDecode(md);
    }
    else
    {
        // disable dragging of top-level nodes (All Feeds, My Tags)
        if (selectedItem() && !selectedItem()->parent())
            return false;
        else
            return true;
    }

    return true;
}

void FeedListView::slotItemUp()
{
    if (selectedItem() && selectedItem()->itemAbove())
    {
        setSelected( selectedItem()->itemAbove(), true );
        ensureItemVisible(selectedItem());
    }   
}

void FeedListView::slotItemDown()
{
    if (selectedItem() && selectedItem()->itemBelow())
    {    
        setSelected( selectedItem()->itemBelow(), true );
        ensureItemVisible(selectedItem());
    }
}

void FeedListView::slotItemBegin()
{
    setSelected( firstChild(), true );
    ensureItemVisible(firstChild());
}

void FeedListView::slotItemEnd()
{
    Q3ListViewItem* elt = firstChild();
    if (elt)
        while (elt->itemBelow())
            elt = elt->itemBelow();
    setSelected( elt, true );
    ensureItemVisible(elt);
}

void FeedListView::slotItemLeft()
{
    Q3ListViewItem* sel = selectedItem();
    
    if (!sel || sel == findNodeItem(rootNode()))
        return;
    
    if (sel->isOpen())
        sel->setOpen(false);
    else
    {
        if (sel->parent())
            setSelected( sel->parent(), true );
    }
        
    ensureItemVisible( selectedItem() );    
}

void FeedListView::slotItemRight()
{
    Q3ListViewItem* sel = selectedItem();
    if (!sel)
    {
        setSelected( firstChild(), true );
        sel = firstChild();
    }
    if (sel->isExpandable() && !sel->isOpen())
        sel->setOpen(true);
    else
    {
        if (sel->firstChild())
            setSelected( sel->firstChild(), true );
    }
    ensureItemVisible( selectedItem() );
}

void FeedListView::slotPrevFeed()
{
    for (Q3ListViewItemIterator it( selectedItem()); it.current(); --it )
    {
        TreeNodeItem* tni = dynamic_cast<TreeNodeItem*>(*it);
        if (tni && !tni->isSelected() && !tni->node()->isGroup() )
        {
            setSelected(tni, true);
            ensureItemVisible(tni);
            return;
        }     
    }
}
    
void FeedListView::slotNextFeed()
{
    for (Q3ListViewItemIterator it( selectedItem()); it.current(); ++it )
    {
        TreeNodeItem* tni = dynamic_cast<TreeNodeItem*>(*it);
        if ( tni && !tni->isSelected() && !tni->node()->isGroup() )
        {
            setSelected(tni, true);
            ensureItemVisible(tni);
            return;
        }     
    }
}

void FeedListView::slotPrevUnreadFeed()
{
    if (!firstChild() || !firstChild()->firstChild())
        return;
    if ( !selectedItem() )
        slotNextUnreadFeed(); 

    Q3ListViewItemIterator it( selectedItem() );
    
    for ( ; it.current(); --it )
    {
        TreeNodeItem* tni = dynamic_cast<TreeNodeItem*> (it.current());
        if (!tni)
            break;
        if ( !tni->isSelected() && !tni->node()->isGroup() && tni->node()->unread() > 0)
        {
            setSelected(tni, true);
            ensureItemVisible(tni);
            return;
        }
    }
    // reached when there is no unread feed above the selected one
    // => cycle: go to end of list...
    if (rootNode()->unread() > 0)
    {

        it = Q3ListViewItemIterator(lastItem());
    
        for ( ; it.current(); --it)
        {

            TreeNodeItem* tni = dynamic_cast<TreeNodeItem*> (it.current());

            if (!tni)
                break;

            if (!tni->isSelected() && !tni->node()->isGroup() && tni->node()->unread() > 0)
            {
                setSelected(tni, true);
                ensureItemVisible(tni);
                return;
            }
        }
    }
}

void FeedListView::slotNextUnreadFeed()
{
    Q3ListViewItemIterator it;
    
    if ( !selectedItem() )
    {
        // if all feeds doesn't exists or is empty, return
        if (!firstChild() || !firstChild()->firstChild())
            return;    
        else 
            it = Q3ListViewItemIterator( firstChild()->firstChild());
    }
    else
        it = Q3ListViewItemIterator( selectedItem() );
    
    for ( ; it.current(); ++it )
    {
        TreeNodeItem* tni = dynamic_cast<TreeNodeItem*> (it.current());
        if (!tni)
            break;
        if ( !tni->isSelected() && !tni->node()->isGroup() && tni->node()->unread() > 0)
        {
            setSelected(tni, true);
            ensureItemVisible(tni);
            return;
        }
    }
    // if reached, we are at the end of the list++
    if (rootNode()->unread() > 0)
    {
        clearSelection();
        slotNextUnreadFeed();
    }
}

void FeedListView::slotSelectionChanged(Q3ListViewItem* item)
{
 TreeNodeItem* ni = dynamic_cast<TreeNodeItem*> (item);
    if (ni)
        emit signalNodeSelected(ni->node());
}

void FeedListView::slotItemRenamed(Q3ListViewItem* item, int col, const QString& text)
{
    TreeNodeItem* ni = dynamic_cast<TreeNodeItem*> (item);
    if ( !ni || !ni->node() )
        return;
    if (col == 0)
    {
        if (text != ni->node()->title())
        {
            kDebug() << "renamed item to \"" << text << "\"" << endl;
            ni->node()->setTitle(text);
        }
    }
}
void FeedListView::slotContextMenu(K3ListView* list, Q3ListViewItem* item, const QPoint& p)
{    
    TreeNodeItem* ti = dynamic_cast<TreeNodeItem*>(item);
    emit signalContextMenu(list, ti ? ti->node() : 0, p);
    if (ti)
        ti->showContextMenu(p);
}

void FeedListView::slotFeedFetchStarted(Feed* feed)
{
#if 0
    // Disable icon to show it is fetching.
    if (!feed->favicon().isNull())
    {
        TreeNodeItem* item = findNodeItem(feed);
        KIconEffect iconEffect;
        QPixmap tempIcon = iconEffect.apply(feed->favicon(), K3Icon::Small, K3Icon::DisabledState);
        item->setPixmap(0, tempIcon);
    }
#endif
}

void FeedListView::slotFeedFetchAborted(Feed* feed)
{
    TreeNodeItem* item = findNodeItem(feed);
    if (item)
        item->nodeChanged();
}

void FeedListView::slotFeedFetchError(Feed* feed)
{
    TreeNodeItem* item = findNodeItem(feed);
    if (item)
        item->nodeChanged();
}

void FeedListView::slotFeedFetchCompleted(Feed* feed)
{
    TreeNodeItem* item = findNodeItem(feed);
    if (item)
        item->nodeChanged();
}
      
void FeedListView::slotNodeAdded(TreeNode* node)
{
    if (node)
        d->createItemVisitor->visit(node);
}

void FeedListView::slotNodeRemoved(Folder* /*parent*/, TreeNode* node)
{
    if (node)
        d->deleteItemVisitor->deleteItem(node, false); 
}

void FeedListView::connectToNode(TreeNode* node)
{
    if (node)
        d->connectNodeVisitor->visit(node);
}

void FeedListView::connectToFeedList(FeedList* list)
{
    if (!list)
        return;
    
    connect(list, SIGNAL(signalDestroyed(Akregator::FeedList*)), this, SLOT(slotFeedListDestroyed(Akregator::FeedList*)) );
    connect(list->rootNode(), SIGNAL(signalChanged(Akregator::TreeNode*)), this, SLOT(slotRootNodeChanged(Akregator::TreeNode*)));
}

void FeedListView::disconnectFromFeedList(FeedList* list)
{
    if (!list)
        return;
    
    disconnect(list, SIGNAL(signalDestroyed(Akregator::FeedList*)), this, SLOT(slotFeedListDestroyed(Akregator::FeedList*)) );
    disconnect(list->rootNode(), SIGNAL(signalChanged(Akregator::TreeNode*)), this, SLOT(slotRootNodeChanged(Akregator::TreeNode*)));
}

void FeedListView::disconnectFromNode(TreeNode* node)
{
    if (node)
        d->disconnectNodeVisitor->visit(node);
}

void FeedListView::slotFeedListDestroyed(FeedList* list)
{
    if (list != d->nodeList)
        return;

    setFeedList(0);
}

void FeedListView::slotNodeDestroyed(TreeNode* node)
{
    if (node)
        d->deleteItemVisitor->deleteItem(node, true);
}

void FeedListView::slotRootNodeChanged(TreeNode* rootNode)
{
    emit signalRootNodeChanged(this, rootNode);
}

void FeedListView::slotNodeChanged(TreeNode* node)
{
    TreeNodeItem* item = findNodeItem(node);
    if (item)
    {    
        item->nodeChanged();
        triggerUpdate();
    }    
}

#ifdef __GNUC__
#warning Port to new drag'n'drop way of using QMimeData and QDrop instead of drop objects
#endif
Q3DragObject *FeedListView::dragObject()
{
    K3MultipleDrag *md = new K3MultipleDrag(viewport());
    Q3DragObject *obj = K3ListView::dragObject();
    if (obj) {
        md->addDragObject(obj);
    }
    TreeNodeItem *i = dynamic_cast<TreeNodeItem*>(currentItem());
    if (i) {
        md->setPixmap(*(i->pixmap(0)));
        FeedItem *fi = dynamic_cast<FeedItem*>(i);
        if (fi) {
            md->addDragObject(new K3URLDrag(KUrl(fi->node()->xmlUrl()), 0L));
        }
    }
    return md;
}

void FeedListView::openFolder() {
    d->autoopentimer.stop();
    if (d->parent && !d->parent->isOpen())
    {
        d->parent->setOpen(true);
    }
}

} // namespace Akregator

#include "feedlistview.moc"
