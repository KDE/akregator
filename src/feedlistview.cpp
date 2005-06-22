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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

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
#include "tagnode.h"
#include "tagnodeitem.h"
#include "tagnodelist.h"
#include "treenode.h"
#include "treenodeitem.h"
#include "treenodevisitor.h"

#include <kdebug.h>
#include <kiconeffect.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmultipledrag.h>
#include <kstringhandler.h>
#include <kurldrag.h>

#include <qfont.h>
#include <qheader.h>
#include <qpainter.h>
#include <qwhatsthis.h>

using namespace Akregator;

class FeedListView::ConnectNodeVisitor : public TreeNodeVisitor
{
    public:
        ConnectNodeVisitor(FeedListView* view) : m_view(view) {}

        virtual bool visitTagNode(TagNode* node)
        {
            connect(node, SIGNAL(signalDestroyed(TreeNode*)), m_view, SLOT(slotNodeDestroyed(TreeNode*) ));
            connect(node, SIGNAL(signalChanged(TreeNode*)), m_view, SLOT(slotNodeChanged(TreeNode*) ));
            return true;
        }
        
        virtual bool visitFolder(Folder* node)
        {
            connect(node, SIGNAL(signalChildAdded(TreeNode*)), m_view, SLOT(slotNodeAdded(TreeNode*) ));
            connect(node, SIGNAL(signalChildRemoved(Folder*, TreeNode*)), m_view, SLOT(slotNodeRemoved(Folder*, TreeNode*) ));
            
            connect(node, SIGNAL(signalDestroyed(TreeNode*)), m_view, SLOT(slotNodeDestroyed(TreeNode*) ));
            connect(node, SIGNAL(signalChanged(TreeNode*)), m_view, SLOT(slotNodeChanged(TreeNode*) ));
            return true;
        }
        
        virtual bool visitFeed(Feed* node)
        {
            connect(node, SIGNAL(signalDestroyed(TreeNode*)), m_view, SLOT(slotNodeDestroyed(TreeNode*) ));
            connect(node, SIGNAL(signalChanged(TreeNode*)), m_view, SLOT(slotNodeChanged(TreeNode*) ));
            connect(node, SIGNAL(fetchStarted(Feed*)), m_view, SLOT(slotFeedFetchStarted(Feed*)));
            connect(node, SIGNAL(fetchAborted(Feed*)), m_view, SLOT(slotFeedFetchAborted(Feed*)));
            connect(node, SIGNAL(fetchError(Feed*)), m_view, SLOT(slotFeedFetchError(Feed*)));
            connect(node, SIGNAL(fetched(Feed*)), m_view, SLOT(slotFeedFetchCompleted(Feed*)));
            return true;
        }
    private:

        FeedListView* m_view;
    
};

class FeedListView::DisconnectNodeVisitor : public TreeNodeVisitor
{
    public:
        DisconnectNodeVisitor(FeedListView* view) : m_view(view) {}

        virtual bool visitTagNode(TagNode* node)
        {
            disconnect(node, SIGNAL(signalDestroyed(TreeNode*)), m_view, SLOT(slotNodeDestroyed(TreeNode*) ));
            disconnect(node, SIGNAL(signalChanged(TreeNode*)), m_view, SLOT(slotNodeChanged(TreeNode*) ));
            return true;
        }
        
        virtual bool visitFolder(Folder* node)
        {
            disconnect(node, SIGNAL(signalChildAdded(TreeNode*)), m_view, SLOT(slotNodeAdded(TreeNode*) ));
            disconnect(node, SIGNAL(signalChildRemoved(Folder*, TreeNode*)), m_view, SLOT(slotNodeRemoved(Folder*, TreeNode*) ));
            
            disconnect(node, SIGNAL(signalDestroyed(TreeNode*)), m_view, SLOT(slotNodeDestroyed(TreeNode*) ));
            disconnect(node, SIGNAL(signalChanged(TreeNode*)), m_view, SLOT(slotNodeChanged(TreeNode*) ));
            return true;
        }
        
        virtual bool visitFeed(Feed* node)
        {

            disconnect(node, SIGNAL(signalDestroyed(TreeNode*)), m_view, SLOT(slotNodeDestroyed(TreeNode*) ));
            disconnect(node, SIGNAL(signalChanged(TreeNode*)), m_view, SLOT(slotNodeChanged(TreeNode*) ));
            disconnect(node, SIGNAL(fetchStarted(Feed*)), m_view, SLOT(slotFeedFetchStarted(Feed*)));
            disconnect(node, SIGNAL(fetchAborted(Feed*)), m_view, SLOT(slotFeedFetchAborted(Feed*)));
            disconnect(node, SIGNAL(fetchError(Feed*)), m_view, SLOT(slotFeedFetchError(Feed*)));
            disconnect(node, SIGNAL(fetched(Feed*)), m_view, SLOT(slotFeedFetchCompleted(Feed*)));
            return true;
        }
    private:

        FeedListView* m_view;
};

class FeedListView::CreateItemVisitor : public TreeNodeVisitor
{
    public:
        CreateItemVisitor(FeedListView* view) : m_view(view) {}

        virtual bool visitTagNode(TagNode* node)
        {
            kdDebug() << "create item for " << node->title() << endl;
            TagNodeItem* item = 0;
            TreeNode* prev = node->prevSibling();
            FolderItem* parentItem = static_cast<FolderItem*>(m_view->findNodeItem(node->parent()));
            if (prev)
            {
                item = new TagNodeItem( parentItem, m_view->findNodeItem(prev), node);
            }
            else
                item = new TagNodeItem( parentItem, node);
                
            item->nodeChanged();     
            m_view->m_itemDict.insert(node, item);
            m_view->connectToNode(node);
            return true;
        }
        
        virtual bool visitFolder(Folder* node)
        {
            FolderItem* item = 0;
            TreeNode* prev = node->prevSibling();
            FolderItem* parentItem = static_cast<FolderItem*>(m_view->findNodeItem(node->parent()));
            if (prev)
            {
                 item = new FolderItem( parentItem, m_view->findNodeItem(prev), node);
            }
            else
                item = new FolderItem(parentItem, node);

            m_view->m_itemDict.insert(node, item);
            QValueList<TreeNode*> children = node->children();

            // add children recursively
            for (QValueList<TreeNode*>::ConstIterator it =  children.begin(); it != children.end(); ++it )
                visit(*it);

            m_view->connectToNode(node);
            return true;
        }
        
        virtual bool visitFeed(Feed* node)
        {
            FeedItem* item = 0;
            TreeNode* prev = node->prevSibling();
            FolderItem* parentItem = static_cast<FolderItem*>(m_view->findNodeItem(node->parent()));
            if (prev)
            {
                item = new FeedItem( parentItem, m_view->findNodeItem(prev), node);
            }
            else
                item = new FeedItem( parentItem, node);
                
            item->nodeChanged();     
            m_view->m_itemDict.insert(node, item);
            m_view->connectToNode(node);
            return true;
        }
        
    private:
        FeedListView* m_view;
};

FeedListView::FeedListView( QWidget *parent, const char *name)
        : KListView(parent, name), m_showTagFolders(true)
{
    m_tagNodeList = 0;
    m_connectNodeVisitor = new ConnectNodeVisitor(this),
    m_disconnectNodeVisitor = new DisconnectNodeVisitor(this);
    m_createItemVisitor = new CreateItemVisitor(this);

    setMinimumSize(150, 150);
    addColumn(i18n("Feeds"));
    setRootIsDecorated(true);
    setItemsRenameable(true);
    setItemMargin(2);

    setFullWidth(true);
    setSorting(-1);
    setDragAutoScroll(true);
    setDropVisualizer(true);
    //setDropHighlighter(false);

     // these have to be enabled from outside after loading the feed list!
    setDragEnabled(true);
    setAcceptDrops(true);
    setItemsMovable(true);
    
    connect( this, SIGNAL(dropped(QDropEvent*,QListViewItem*)), this, SLOT(slotDropped(QDropEvent*,QListViewItem*)) );
    connect( this, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(slotSelectionChanged(QListViewItem*)) );
    connect( this, SIGNAL(itemRenamed(QListViewItem*, const QString&, int)), this, SLOT(slotItemRenamed(QListViewItem*, const QString&, int)) );
    connect( this, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)), this, SLOT(slotContextMenu(KListView*, QListViewItem*, const QPoint&)) );
    connect( &m_autoopentimer, SIGNAL( timeout() ), this, SLOT( openFolder() ) );

    clear();
    
    QWhatsThis::add(this, i18n("<h2>Feeds tree</h2>"
        "Here you can browse tree of feeds. "
        "You can also add feeds or feed groups (folders) "
        "using right-click menu, or reorganize them using "
        "drag and drop."));
    setUpdatesEnabled(true);
}

FeedListView::~FeedListView()
{
    delete m_connectNodeVisitor;
    delete m_disconnectNodeVisitor;
    delete m_createItemVisitor;
}

void FeedListView::setFeedList(FeedList* feedList, TagNodeList* tagNodeList)
{
    if (feedList == m_feedList)
         return;

    clear();

    disconnectFromFeedList(m_feedList);
    
    if (!feedList)
        return;

    m_feedList = feedList;
    connectToFeedList(feedList);
    
    Folder* rootNode = feedList->rootNode();
    if (!rootNode)
        return;
    FolderItem* ri = new FolderItem(this, rootNode );
    m_itemDict.insert(rootNode, ri);
    connectToNode(rootNode);

    // add items for children recursively
    QValueList<TreeNode*> children = rootNode->children();
    
    for (QValueList<TreeNode*>::ConstIterator it = children.begin(); it != children.end(); ++it)
        slotNodeAdded(*it);

    // handle tagNodeLsit

    rootNode = tagNodeList->rootNode();
    ri = new FolderItem(this, ri, rootNode);
    m_itemDict.insert(rootNode, ri);

    children = rootNode->children();
    
    for (QValueList<TreeNode*>::ConstIterator it = children.begin(); it != children.end(); ++it)
        slotNodeAdded(*it);
}

void FeedListView::takeNode(QListViewItem* item)
{
    if (item->parent())
        item->parent()->takeItem(item);
    else
        takeItem(item);
}

void FeedListView::insertNode(QListViewItem* parent, QListViewItem* item, QListViewItem* after)
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
    return m_feedList ? m_feedList->rootNode() : 0;
}

TreeNode* FeedListView::selectedNode()
{
    TreeNodeItem* item = static_cast<TreeNodeItem*> (selectedItem());
    
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
    TreeNodeItem* item = static_cast<TreeNodeItem*>(findItem(title, 0));
    if (!item)
        return 0;
    else 
        return item->node();
}

TreeNodeItem* FeedListView::findNodeItem(TreeNode* node)
{
    return m_itemDict.find(node);
}

TreeNodeItem* FeedListView::findItem (const QString& text, int column, ComparisonFlags compare) const
{ 
    return static_cast<TreeNodeItem*> (KListView::findItem(text, column, compare)); 
}

void FeedListView::ensureNodeVisible(TreeNode* node)
{
    ensureItemVisible(findNodeItem(node));
}

void FeedListView::clear()
{
    QPtrDictIterator<TreeNodeItem> it(m_itemDict);
    for( ; it.current(); ++it )
        disconnectFromNode( it.current()->node() );
    m_itemDict.clear();
    m_feedList = 0;
    
    KListView::clear();
}

void FeedListView::drawContentsOffset( QPainter * p, int ox, int oy,
                                       int cx, int cy, int cw, int ch )
{
    bool oldUpdatesEnabled = isUpdatesEnabled();
    setUpdatesEnabled(false);
    KListView::drawContentsOffset( p, ox, oy, cx, cy, cw, ch );
    setUpdatesEnabled(oldUpdatesEnabled);
}

void FeedListView::slotDropped( QDropEvent *e, QListViewItem * /*item*/ )
{
	m_autoopentimer.stop();
    if (e->source() != viewport())
    {
        openFolder();

        FolderItem* parent = static_cast<FolderItem*> (m_parent);
        TreeNodeItem* afterMe = 0;

        if(m_afterme)
            afterMe = static_cast<TreeNodeItem*> (m_afterme);

        KURL::List urls;
        KURLDrag::decode( e, urls );
        e->accept();
        emit signalDropped( urls, afterMe, parent);
    }
}

void FeedListView::movableDropEvent(QListViewItem* parent, QListViewItem* afterme)
{
	m_autoopentimer.stop();
    if (m_parent)
    {    
        openFolder();

        Folder* parentNode = (static_cast<FolderItem*> (m_parent))->node();
        TreeNode* afterMeNode = 0; 
        TreeNode* current = selectedNode();

        if (m_afterme)
            afterMeNode = (static_cast<TreeNodeItem*> (m_afterme))->node();

        current->parent()->removeChild(current);
        parentNode->insertChild(current, afterMeNode);
        KListView::movableDropEvent(m_parent, m_afterme);
    }    
}

void FeedListView::setShowTagFolders(bool enabled)
{
    m_showTagFolders = enabled;
}

void FeedListView::contentsDragMoveEvent(QDragMoveEvent* event)
{
    QPoint vp = contentsToViewport(event->pos());
    QListViewItem *i = itemAt(vp);

    QListViewItem *qiparent;
    QListViewItem *qiafterme;
    findDrop( event->pos(), qiparent, qiafterme );

    if (event->source() == viewport()) {
        // disable any drops where the result would be top level nodes 
        if (i && !i->parent())
        {
            event->ignore();
            m_autoopentimer.stop();
            return;
        }

        // prevent dragging nodes from All Feeds to My Tags or vice versa
        QListViewItem* root1 = i;
        while (root1 && root1->parent())
            root1 = root1->parent();

        QListViewItem* root2 = selectedItem();
        while (root2 && root2->parent())
            root2 = root2->parent();

        if (root1 != root2)
        {
            event->ignore();
            m_autoopentimer.stop();
            return;
        }

        // don't drop node into own subtree
        QListViewItem* p = qiparent;
        while (p)
            if (p == selectedItem())
            {
                event->ignore();
                m_autoopentimer.stop();
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
            m_autoopentimer.stop();
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

        TreeNode *iNode = (static_cast<TreeNodeItem*> (i))->node();
        if (iNode->isGroup())
        {
            if (i != m_parent)
                m_autoopentimer.stop();
            m_parent = i;
        }
        else
        {
            event->ignore();
            m_autoopentimer.stop();
            return;
        }
        m_afterme = 0;
        m_autoopentimer.start(750);
    }
    else
    {
        setDropVisualizer(true);
        setDropHighlighter(false);
        cleanItemHighlighter();
        m_parent = qiparent;
        m_afterme = qiafterme;
        m_autoopentimer.stop();
    }

    // the rest is handled by KListView.
    KListView::contentsDragMoveEvent(event);
}

/*
void FeedListView::keyPressEvent(QKeyEvent* e)
{
    switch(e->key())
    {
        case Key_Alt+Key_Up:
        case Key_Alt+Key_Down:
        case Key_Alt+Key_Left:    
        case Key_Alt+Key_Right:
            KListView::keyPressEvent(e);    
            break;
        case Key_Left:
        case Key_Right:
        case Key_Up:
        case Key_Down:
            e->ignore();
            break;
        default:
            KListView::keyPressEvent(e);    
     }
}*/

bool FeedListView::acceptDrag(QDropEvent *e) const
{
    if (!acceptDrops() || !itemsMovable())
        return false;

    if (e->source() != viewport())
    {
        return KURLDrag::canDecode( e );
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

void FeedListView::slotCollapseAll()
{
    QListViewItem* it = firstChild();
    while(it)
    {
        it->setOpen(false);
        it = it->nextSibling();
    }
}

void FeedListView::slotCollapse()
{
    if (selectedItem())
        selectedItem()->setOpen(false);
}


void FeedListView::slotExpandAll()
{
    QListViewItem* it = firstChild();
    while(it)
    {
        it->setOpen(true);
        it = it->nextSibling();
    }
}

void FeedListView::slotExpand()
{
    if (selectedItem())
        selectedItem()->setOpen(true);
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
    QListViewItem* elt = firstChild();
    if (elt)
        while (elt->itemBelow())
            elt = elt->itemBelow();
    setSelected( elt, true );
    ensureItemVisible(elt);
}

void FeedListView::slotItemLeft()
{
    QListViewItem* sel = selectedItem();
    
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
    QListViewItem* sel = selectedItem();
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
    for (QListViewItemIterator it( selectedItem()); it.current(); --it )
        if ( !(*it)->isSelected() && !(*it)->isExpandable() )
        {
            setSelected(*it, true);
            ensureItemVisible(*it);
            return;
        }     
}
    
void FeedListView::slotNextFeed()
{
    for (QListViewItemIterator it( selectedItem()); it.current(); ++it )
        if ( !(*it)->isSelected() && !(*it)->isExpandable() )
        {
            setSelected(*it, true);
            ensureItemVisible(*it);
            return;
        }     
}

void FeedListView::slotPrevUnreadFeed()
{
    if (!firstChild() || !firstChild()->firstChild())
        return;
    if ( !selectedItem() )
        slotNextUnreadFeed(); 

    QListViewItemIterator it( selectedItem() );
    
    for ( ; it.current(); --it )
    {
        TreeNodeItem* tni = static_cast<TreeNodeItem*> (it.current());
        if (!tni)
            break;
        if ( !tni->isSelected() && !tni->isExpandable() && tni->node()->unread() > 0)
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

        it = QListViewItemIterator(lastItem());
    
        for ( ; it.current(); --it)
        {

            TreeNodeItem* tni = static_cast<TreeNodeItem*> (it.current());

            if (!tni)
                break;

            if (!tni->isSelected() && !tni->isExpandable() && tni->node()->unread() > 0)
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
    QListViewItemIterator it;
    
    if ( !selectedItem() )
    {
        // if all feeds doesnt exists or is empty, return
        if (!firstChild() || !firstChild()->firstChild())
            return;    
        else 
            it = QListViewItemIterator( firstChild()->firstChild());
    }
    else
        it = QListViewItemIterator( selectedItem() );
    
    for ( ; it.current(); ++it )
    {
        TreeNodeItem* tni = static_cast<TreeNodeItem*> (it.current());
        if (!tni)
            break;
        if ( !tni->isSelected() && !tni->isExpandable() && tni->node()->unread() > 0)
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

void FeedListView::slotSelectionChanged(QListViewItem* item)
{
 TreeNodeItem* ni = static_cast<TreeNodeItem*> (item);
    if (ni)
        emit signalNodeSelected(ni->node());
}

void FeedListView::slotItemRenamed(QListViewItem* item, const QString& text, int)
{
    kdDebug() << "FeedListView::slotItemRenamed(): enter" << endl;
    TreeNodeItem* ni = dynamic_cast<TreeNodeItem*> (item);
    if ( ni && ni->node() )
    {
        kdDebug() << "renamed item to \"" << text << "\"" << endl;
        ni->node()->setTitle(text);
    }
}

void FeedListView::slotContextMenu(KListView* list, QListViewItem* item, const QPoint& p)
{
    emit signalContextMenu(list, static_cast<TreeNodeItem*>(item), p);
}

void FeedListView::slotFeedFetchStarted(Feed* feed)
{
    // Disable icon to show it is fetching.
    if (!feed->favicon().isNull())
    {
        TreeNodeItem* item = findNodeItem(feed);
        KIconEffect iconEffect;
        QPixmap tempIcon = iconEffect.apply(feed->favicon(), KIcon::Small, KIcon::DisabledState);
        item->setPixmap(0, tempIcon);
    }

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
    m_createItemVisitor->visit(node);
}

void FeedListView::slotNodeRemoved(Folder* /*parent*/, TreeNode* node)
{
    if (!node)
        return;
    
    disconnectFromNode(node);    
    takeNode(findNodeItem(node));
}

void FeedListView::connectToNode(TreeNode* node)
{
    if (node)
        m_connectNodeVisitor->visit(node);
}

void FeedListView::connectToFeedList(FeedList* list)
{
    if (!list)
        return;
    
    connect(list, SIGNAL(signalDestroyed(FeedList*)), this, SLOT(slotFeedListDestroyed(FeedList*)) );
}

void FeedListView::disconnectFromFeedList(FeedList* list)
{
    if (!list)
        return;
    
    disconnect(list, SIGNAL(signalDestroyed(FeedList*)), this, SLOT(slotFeedListDestroyed(FeedList*)) );
}

void FeedListView::disconnectFromNode(TreeNode* node)
{
    if (node)
        m_disconnectNodeVisitor->visit(node);
}

void FeedListView::slotFeedListDestroyed(FeedList* list)
{
    if (list != m_feedList)
        return;

    setFeedList(0, 0);
}

void FeedListView::slotNodeDestroyed(TreeNode* node)
{
    TreeNodeItem* item = findNodeItem(node);
    
    if (!node || !item)
        return;
    
    m_itemDict.remove(node);

    if ( item->isSelected() )
    {
        if (item->itemBelow())
            setSelected(item->itemBelow(), true);
        else if (item->itemAbove())
            setSelected(item->itemAbove(), true);
        else
            setSelected(item, false);
    }
    delete item;
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

QDragObject *FeedListView::dragObject()
{
    KMultipleDrag *md = new KMultipleDrag(viewport());
    QDragObject *obj = KListView::dragObject();
    if (obj) {
        md->addDragObject(obj);
    }
    TreeNodeItem *i = static_cast<TreeNodeItem*>(currentItem());
    if (i) {
        md->setPixmap(*(i->pixmap(0)));
        FeedItem *fi = dynamic_cast<FeedItem*>(i);
        if (fi) {
            md->addDragObject(new KURLDrag(KURL(fi->node()->xmlUrl()), 0L));
        }
    }
    return md;
}

void FeedListView::openFolder() {
    m_autoopentimer.stop();
    if (m_parent && !m_parent->isOpen())
    {
        m_parent->setOpen(true);
    }
}
#include "feedlistview.moc"
