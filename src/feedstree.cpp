/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "feedgroup.h"
#include "feedgroupitem.h"
#include "feedstree.h"
#include "feed.h"
#include "feeditem.h"
#include "feedlist.h"
#include "treenode.h"
#include "treenodeitem.h"

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

FeedsTree::FeedsTree( QWidget *parent, const char *name)
        : KListView(parent, name)
{
    setMinimumSize(150, 150);
    addColumn(i18n("Feeds"));
    setRootIsDecorated(false);
    setItemsRenameable(true);
    setItemMargin(2);
    
    setFullWidth(true);
    setSorting(-1);
    setDragAutoScroll(true);
    setDropVisualizer(true);

     // these have to be enabled from outside after loading the feed list!
    setDragEnabled(true);
    setAcceptDrops(true);
    setItemsMovable(true);
    
    connect( this, SIGNAL(dropped(QDropEvent*,QListViewItem*)), this, SLOT(slotDropped(QDropEvent*,QListViewItem*)) );
    connect( this, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(slotSelectionChanged(QListViewItem*)) );
    connect( this, SIGNAL(itemRenamed(QListViewItem*, const QString&, int)), this, SLOT(slotItemRenamed(QListViewItem*, const QString&, int)) );
    connect( this, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)), this, SLOT(slotContextMenu(KListView*, QListViewItem*, const QPoint&)) );
    
    clear();
    
    QWhatsThis::add(this, i18n("<h2>Feeds tree</h2>"
        "Here you can browse tree of feeds. "
        "You can also add feeds or feed groups (folders) "
        "using right-click menu, or reorganize them using "
        "drag and drop."));
    setUpdatesEnabled(true);
}

FeedsTree::~FeedsTree()
{}

void FeedsTree::setFeedList(FeedList* feedList)
{
    if (feedList == m_feedList)
         return;

    clear();

    disconnectFromFeedList(m_feedList);
    
    if (!feedList)
        return;

    m_feedList = feedList;
    connectToFeedList(feedList);
    
    FeedGroup* rootNode = feedList->rootNode();
    if (!rootNode)
        return;
    FeedGroupItem* ri = new FeedGroupItem(this, rootNode );
    m_itemDict.insert(rootNode, ri);
    connectToNode(rootNode);

    // add items for children recursively
    QPtrList<TreeNode> children = rootNode->children();
    for (TreeNode* i = children.first(); i; i = children.next() )
        slotNodeAdded(i);
}

void FeedsTree::takeNode(QListViewItem* item)
{
    if (item->parent())
        item->parent()->takeItem(item);
    else
        takeItem(item);
}

void FeedsTree::insertNode(QListViewItem* parent, QListViewItem* item, QListViewItem* after)
{
    if (parent)
        parent->insertItem(item);
    else
        insertItem(item);
    if (after)
        item->moveItem(after);
}

FeedGroup* FeedsTree::rootNode()
{
    return m_feedList ? m_feedList->rootNode() : 0;
}

TreeNode* FeedsTree::selectedNode()
{
    TreeNodeItem* item = static_cast<TreeNodeItem*> (selectedItem());
    
    return ( item ? item->node() : 0) ;
}

void FeedsTree::setSelectedNode(TreeNode* node)
{
    TreeNodeItem* item = findNodeItem(node);
    if ( node && item )
        setSelected(item, true);
}

TreeNode* FeedsTree::findNodeByTitle(const QString& title)
{
    TreeNodeItem* item = static_cast<TreeNodeItem*>(findItem(title, 0));
    if (!item)
        return 0;
    else 
        return item->node();
}

TreeNodeItem* FeedsTree::findNodeItem(TreeNode* node)
{
    return m_itemDict.find(node);
}

TreeNodeItem* FeedsTree::findItem (const QString& text, int column, ComparisonFlags compare) const
{ 
    return static_cast<TreeNodeItem*> (KListView::findItem(text, column, compare)); 
}

void FeedsTree::ensureNodeVisible(TreeNode* node)
{
    ensureItemVisible(findNodeItem(node));
}

void FeedsTree::clear()
{
    QPtrDictIterator<TreeNodeItem> it(m_itemDict);
    for( ; it.current(); ++it )
        disconnectFromNode( it.current()->node() );
    m_itemDict.clear();
    m_feedList = 0;
    
    KListView::clear();
}

void FeedsTree::drawContentsOffset( QPainter * p, int ox, int oy,
                                       int cx, int cy, int cw, int ch )
{
    bool oldUpdatesEnabled = isUpdatesEnabled();
    setUpdatesEnabled(false);
    KListView::drawContentsOffset( p, ox, oy, cx, cy, cw, ch );
    setUpdatesEnabled(oldUpdatesEnabled);
}

void FeedsTree::slotDropped( QDropEvent *e, QListViewItem * /*item*/ )
{
    if (!acceptDrag(e))
        return;
    
    QListViewItem *qiparent;
    QListViewItem *qiafterme;
    findDrop( e->pos(), qiparent, qiafterme );
    FeedGroupItem* parent = static_cast<FeedGroupItem*> (qiparent);
                
    if (!parent)
    {
        e->accept();
        return;
    }
    
    if (e->source() != viewport())
    {
        if (KURLDrag::canDecode( e ))
        {
            findDrop( e->pos(), qiparent, qiafterme );
            FeedGroupItem* parent = static_cast<FeedGroupItem*> (qiparent);
            TreeNodeItem* afterme = static_cast<TreeNodeItem*> (qiafterme);
    
            KURL::List urls;
            KURLDrag::decode( e, urls );
            e->accept();
            emit signalDropped( urls, afterme, parent);
        }
        else
        {
            e->ignore();
            return;
        }
    }
    
}

void FeedsTree::movableDropEvent(QListViewItem* parent, QListViewItem* afterme)
{
    if (selectedItem() == afterme)
        return;
    
    if (parent)
    {    
        FeedGroup* parentNode = (static_cast<FeedGroupItem*> (parent))->node();

        TreeNode* afterMeNode = 0; 
        TreeNode* current = selectedNode();

        if (afterme)
            afterMeNode = (static_cast<TreeNodeItem*> (afterme))->node();
        
        // don't drop node into own subtree
        FeedGroup* p = parentNode;
        while (p)
            if (p == current)
                return;
            else
                p = p->parent(); 
        
        current->parent()->removeChild(current);
        parentNode->insertChild(current, afterMeNode);
        KListView::movableDropEvent(parent, afterme);
    }    
}

void FeedsTree::keyPressEvent(QKeyEvent* e)
{
    // handle return in in-place renaming, otherwise ignore events
    if (isRenaming() && e->key() == Qt::Key_Enter)
        KListView::keyPressEvent(e);
    else
        e->ignore();
}

void FeedsTree::contentsDragMoveEvent(QDragMoveEvent* event)
{
    // if we are dragging over All feeds, enable
    QPoint vp = contentsToViewport(event->pos());
    QListViewItem *i = itemAt(vp);
    if (i == firstChild())
    {
        event->accept();
        return;
    }

    QListViewItem *qiparent;
    QListViewItem *qiafterme;
    findDrop( event->pos(), qiparent, qiafterme );
    FeedGroupItem* parent = static_cast<FeedGroupItem*> (qiparent);
    //TreeNodeItem* afterme = static_cast<TreeNodeItem*> (qiafterme);
    
    // disable any drops where the result would be top level nodes 
     if (!parent )
    {
        event->ignore();
        return;
    }
        
    if (!i || event->pos().x() > header()->cellPos(header()->mapToIndex(0)) +
            treeStepSize() * (i->depth() + 1) + itemMargin() ||
            event->pos().x() < header()->cellPos(header()->mapToIndex(0)))
    {}
    else if (i && i->childCount() && !i->isOpen())
            i->setOpen(true); // open folders under drag

    // the rest is handled by KListView.
    KListView::contentsDragMoveEvent(event);
}
/*
void FeedsTree::keyPressEvent(QKeyEvent* e)
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

bool FeedsTree::acceptDrag(QDropEvent *e) const
{
    if (!acceptDrops() || !itemsMovable())
        return false;

    if (e->source() != viewport())
    {
        return KURLDrag::canDecode( e );
    }
    else
    {
        // disable dragging "All Feeds"
        if (firstChild()->isSelected())
            return false;
        else
            return true;
    }

    return true;
}

void FeedsTree::slotCollapseAll()
{
    QListViewItem* it = firstChild();
    while(it)
    {
        it->setOpen(false);
        it = it->nextSibling();
    }
}

void FeedsTree::slotCollapse()
{
    if (selectedItem())
        selectedItem()->setOpen(false);
}


void FeedsTree::slotExpandAll()
{
    QListViewItem* it = firstChild();
    while(it)
    {
        it->setOpen(true);
        it = it->nextSibling();
    }
}

void FeedsTree::slotExpand()
{
    if (selectedItem())
        selectedItem()->setOpen(true);
}


void FeedsTree::slotItemUp()
{
    if (selectedItem() && selectedItem()->itemAbove())
    {
        setSelected( selectedItem()->itemAbove(), true );
        ensureItemVisible(selectedItem());
    }   
}

void FeedsTree::slotItemDown()
{
    if (selectedItem() && selectedItem()->itemBelow())
    {    
        setSelected( selectedItem()->itemBelow(), true );
        ensureItemVisible(selectedItem());
    }
}

void FeedsTree::slotItemBegin()
{
    setSelected( firstChild(), true );
    ensureItemVisible(firstChild());
}

void FeedsTree::slotItemEnd()
{
    QListViewItem* elt = firstChild();
    if (elt)
        while (elt->itemBelow())
            elt = elt->itemBelow();
    setSelected( elt, true );
    ensureItemVisible(elt);
}

void FeedsTree::slotItemLeft()
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

void FeedsTree::slotItemRight()
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

void FeedsTree::slotPrevFeed()
{
    for (QListViewItemIterator it( selectedItem()); it.current(); --it )
        if ( !(*it)->isSelected() && !(*it)->isExpandable() )
        {
            setSelected(*it, true);
            ensureItemVisible(*it);
            return;
        }     
}
    
void FeedsTree::slotNextFeed()
{
    for (QListViewItemIterator it( selectedItem()); it.current(); ++it )
        if ( !(*it)->isSelected() && !(*it)->isExpandable() )
        {
            setSelected(*it, true);
            ensureItemVisible(*it);
            return;
        }     
}

void FeedsTree::slotPrevUnreadFeed()
{
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
}

void FeedsTree::slotNextUnreadFeed()
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
}

void FeedsTree::slotSelectionChanged(QListViewItem* item)
{
 TreeNodeItem* ni = static_cast<TreeNodeItem*> (item);
    if (ni)
        emit signalNodeSelected(ni->node());
}

void FeedsTree::slotItemRenamed(QListViewItem* item, const QString& text, int)
{
    kdDebug() << "FeedsTree::slotItemRenamed(): enter" << endl;
    TreeNodeItem* ni = static_cast<TreeNodeItem*> (item);
    if ( ni && ni->node() )
    {
        kdDebug() << "renamed item to \"" << text << "\"" << endl;
        ni->node()->setTitle(text);
    }
}

void FeedsTree::slotContextMenu(KListView* list, QListViewItem* item, const QPoint& p)
{
    emit signalContextMenu(list, static_cast<TreeNodeItem*>(item), p);
}

void FeedsTree::slotFeedFetchStarted(Feed* feed)
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

void FeedsTree::slotFeedFetchAborted(Feed* feed)
{
    TreeNodeItem* item = findNodeItem(feed);
    if (item)
        item->nodeChanged();
}

void FeedsTree::slotFeedFetchError(Feed* feed)
{
    TreeNodeItem* item = findNodeItem(feed);
    if (item)
        item->nodeChanged();
}

void FeedsTree::slotFeedFetchCompleted(Feed* feed)
{
    TreeNodeItem* item = findNodeItem(feed);
    if (item)
        item->nodeChanged();
}
      
void FeedsTree::slotNodeAdded(TreeNode* node)
{
    if (!node)
        return;

    FeedGroup* parent = node->parent();
    FeedGroupItem* parentItem = static_cast<FeedGroupItem*>(findNodeItem(parent));
        
    // we aren't interested in nodes whose parents we don't know 
    if (!parentItem)
        return;
    
    TreeNodeItem* item = findNodeItem(node);
    
    QPtrList<TreeNode> children = parent->children();
    children.find(node);
    TreeNode* prev = children.prev();
    
    if (!item)
    {
        if (node->isGroup())
        {
            FeedGroup* fg = static_cast<FeedGroup*>(node);
            if (prev)
                item = new FeedGroupItem( parentItem, findNodeItem(prev), fg);
            else
                item = new FeedGroupItem( parentItem, fg);

            m_itemDict.insert(node, item);
            QPtrList<TreeNode> children = fg->children();

            // add children recursively
            for (TreeNode* i = children.first(); i; i = children.next() )
                slotNodeAdded(i);
        }
        else
        {
            Feed* f = static_cast<Feed*> (node);
            if (prev)
                item = new FeedItem( parentItem, findNodeItem(prev), f );
            else
                item = new FeedItem( parentItem, f );
            m_itemDict.insert(node, item);
        }
    }
    else
    {
        insertNode(parentItem, item, findNodeItem(prev));
        if (!selectedItem())
            setSelected(item, true);
    }

    connectToNode(node);
}

void FeedsTree::slotNodeRemoved(FeedGroup* /*parent*/, TreeNode* node)
{
    if (!node)
        return;
    
    disconnectFromNode(node);    
    takeNode(findNodeItem(node));
}

void FeedsTree::connectToNode(TreeNode* node)
{
    if (node->isGroup())
    {
        FeedGroup* fg = static_cast<FeedGroup*>(node);
                       
        connect(fg, SIGNAL(signalChildAdded(TreeNode*)), this, SLOT(slotNodeAdded(TreeNode*) ));
        connect(fg, SIGNAL(signalChildRemoved(FeedGroup*, TreeNode*)), this, SLOT(slotNodeRemoved(FeedGroup*, TreeNode*) ));
        connect(fg, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotNodeDestroyed(TreeNode*) ));
        connect(fg, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotNodeChanged(TreeNode*) ));
    }
    else
    {
        Feed* f = static_cast<Feed*> (node);
        
        connect(f, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotNodeDestroyed(TreeNode*) ));
        connect(f, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotNodeChanged(TreeNode*) ));
        connect(f, SIGNAL(fetchStarted(Feed*)), this, SLOT(slotFeedFetchStarted(Feed*)));
        connect(f, SIGNAL(fetchAborted(Feed*)), this, SLOT(slotFeedFetchAborted(Feed*)));
        connect(f, SIGNAL(fetchError(Feed*)), this, SLOT(slotFeedFetchError(Feed*)));
        connect(f, SIGNAL(fetched(Feed*)), this, SLOT(slotFeedFetchCompleted(Feed*)));
    }        
}

void FeedsTree::connectToFeedList(FeedList* list)
{
    if (!list)
        return;
    
    connect(list, SIGNAL(signalDestroyed(FeedList*)), this, SLOT(slotFeedListDestroyed(FeedList*)) );
}

void FeedsTree::disconnectFromFeedList(FeedList* list)
{
    if (!list)
        return;
    
    disconnect(list, SIGNAL(signalDestroyed(FeedList*)), this, SLOT(slotFeedListDestroyed(FeedList*)) );
}

void FeedsTree::disconnectFromNode(TreeNode* node)
{
    if (node->isGroup())
    {
        FeedGroup* fg = static_cast<FeedGroup*> (node);
        disconnect(fg, SIGNAL(signalChildAdded(TreeNode*)), this, SLOT(slotNodeAdded(TreeNode*) ));
        disconnect(fg, SIGNAL(signalChildRemoved(FeedGroup*, TreeNode*)), this, SLOT(slotNodeRemoved(FeedGroup*, TreeNode*) ));
        
        disconnect(fg, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotNodeDestroyed(TreeNode*) ));
        disconnect(fg, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotNodeChanged(TreeNode*) ));
    }
    else
    {
        Feed* f = static_cast<Feed*> (node);
        disconnect(f, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotNodeDestroyed(TreeNode*) ));
        disconnect(f, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotNodeChanged(TreeNode*) ));
        disconnect(f, SIGNAL(fetchStarted(Feed*)), this, SLOT(slotFeedFetchStarted(Feed*)));
        disconnect(f, SIGNAL(fetchAborted(Feed*)), this, SLOT(slotFeedFetchAborted(Feed*)));
        disconnect(f, SIGNAL(fetchError(Feed*)), this, SLOT(slotFeedFetchError(Feed*)));
        disconnect(f, SIGNAL(fetched(Feed*)), this, SLOT(slotFeedFetchCompleted(Feed*)));
    }
}

void FeedsTree::slotFeedListDestroyed(FeedList* list)
{
    if (list != m_feedList)
        return;

    setFeedList(0);
}

void FeedsTree::slotNodeDestroyed(TreeNode* node)
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

void FeedsTree::slotNodeChanged(TreeNode* node)
{
    TreeNodeItem* item = findNodeItem(node);
    if (item)
    {    
        item->nodeChanged();
        triggerUpdate();
    }    
}

QDragObject *FeedsTree::dragObject()
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

#include "feedstree.moc"
// vim: ts=4 sw=4 et
