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
#include "treenode.h"
#include "treenodeitem.h"

#include <kdebug.h>
#include <kiconeffect.h>
#include <kiconloader.h>
#include <klocale.h>
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
    setDragEnabled(true);
    setAcceptDrops(true);
    setFullWidth(true);
    setSorting(-1);
    setDragAutoScroll(true);
    setDropVisualizer(true);
    connect( this, SIGNAL(dropped(QDropEvent*,QListViewItem*)), this, SLOT(slotDropped(QDropEvent*,QListViewItem*)) );
    connect( this, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(slotSelectionChanged(QListViewItem*)) );
    connect( this, SIGNAL(itemRenamed(QListViewItem*)), this, SLOT(slotItemRenamed(QListViewItem*)) );

    QWhatsThis::add(this, i18n("<h2>Feeds tree</h2>"
        "Here you can browse tree of feeds. "
        "You can also add feeds or feed groups (folders) "
        "using right-click menu, or reorganize them using "
        "drag and drop."));
    setUpdatesEnabled(true);
}

FeedsTree::~FeedsTree()
{}

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
    FeedGroupItem* item = static_cast<FeedGroupItem*> (firstChild());
    
    return ( item ? item->node() : 0);
}

FeedGroupItem* FeedsTree::rootNodeItem()
{
    return static_cast<FeedGroupItem*> (firstChild());
}

TreeNode* FeedsTree::selectedNode()
{
    TreeNodeItem* item = static_cast<TreeNodeItem*> (selectedItem());
    
    return ( item ? item->node() : 0) ;
}

void FeedsTree::setSelectedNode(TreeNode* node)
{
    TreeNodeItem* item = m_itemDict.find(node);
    if ( node && item )
        setSelected(item, true);
}

TreeNodeItem* FeedsTree::selectedNodeItem()
{
    return static_cast<TreeNodeItem*> (selectedItem());
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
//    kdDebug() << "enter FeedsTree::clear()" << endl;
    KListView::clear();
    m_itemDict.clear();
    FeedGroup* rootNode = new FeedGroup(i18n("All Feeds"));
    FeedGroupItem *elt = new FeedGroupItem(this, rootNode );
    m_itemDict.insert(rootNode, elt);
                
    connect(rootNode, SIGNAL(signalChildAdded(FeedGroup*, TreeNode*)),
            this, SLOT(slotNodeAdded(FeedGroup*, TreeNode*)));
    connect(rootNode, SIGNAL(signalChildRemoved(FeedGroup*, TreeNode*)), this, SLOT(slotNodeRemoved(FeedGroup*, TreeNode*)));
    connect(rootNode, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotNodeDestroyed(TreeNode*) ));
    connect(rootNode, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotNodeChanged(TreeNode*) ));
    
        
}

void FeedsTree::drawContentsOffset( QPainter * p, int ox, int oy,
                                       int cx, int cy, int cw, int ch )
{
    bool oldUpdatesEnabled = isUpdatesEnabled();
    setUpdatesEnabled(false);
    KListView::drawContentsOffset( p, ox, oy, cx, cy, cw, ch );
    setUpdatesEnabled(oldUpdatesEnabled);
}

void FeedsTree::slotDropped( QDropEvent *e, QListViewItem * item )
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
            emit dropped( urls, afterme, parent);
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
    TreeNodeItem* afterme = static_cast<TreeNodeItem*> (qiafterme);
    
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
    QListViewItem* cur = selectedItem();
    if (!cur)
        setSelected( firstChild(), true );
    if (cur->isOpen())
        cur->setOpen(false);
    else
    {
        if (cur->parent())    
            setSelected( cur->parent(), true );
    }    
    ensureItemVisible( selectedItem() );
}

void FeedsTree::slotItemRight()
{
    QListViewItem* cur = selectedItem();
    if (!cur)
        setSelected( firstChild(), true );
    if (cur->isExpandable() && !cur->isOpen())
        cur->setOpen(true);
    else
    {
        if (cur->firstChild())    
            setSelected( cur->firstChild(), true );
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

void FeedsTree::slotItemRenamed(QListViewItem* item)
{
//    kdDebug() << "enter FeedsTree::slotItemRenamed item: " << item->text(0) << endl;
    TreeNodeItem* ni = static_cast<TreeNodeItem*> (item);
    if ( ni && ni->node() )
        ni->node()->setTitle( item->text(0) );
//    kdDebug() << "leave FeedsTree::slotItemRenamed item: " << item->text(0) << endl;
    
}

void FeedsTree::slotFeedFetchStarted(Feed* feed)
{
    // Disable icon to show it is fetching.
    if (!feed->favicon().isNull())
    {
        TreeNodeItem* item = m_itemDict.find(feed);
        KIconEffect iconEffect;
        QPixmap tempIcon = iconEffect.apply(feed->favicon(), KIcon::Small, KIcon::DisabledState);
        item->setPixmap(0, tempIcon);
    }

}

void FeedsTree::slotFeedFetchAborted(Feed* feed)
{
    TreeNodeItem* item = m_itemDict.find(feed);
    if (item)
        item->nodeChanged();
}

void FeedsTree::slotFeedFetchError(Feed* feed)
{
    TreeNodeItem* item = m_itemDict.find(feed);
    if (item)
        item->nodeChanged();
}

void FeedsTree::slotFeedFetchCompleted(Feed* feed)
{
    TreeNodeItem* item = m_itemDict.find(feed);
    if (item)
        item->nodeChanged();
}
      
void FeedsTree::slotNodeAdded(FeedGroup* parent, TreeNode* node)
{
//     kdDebug() << "enter FeedsTree::slotNodeAdded node: " << (node ? node->title() : 0) << endl;
         
    if (!node || !parent)
        return;
 
    QPtrList<TreeNode> children = parent->children();
    children.find(node);
    TreeNode* prev = children.prev();
    TreeNodeItem* prevItem = m_itemDict.find(prev); 
    FeedGroupItem* parentItem = static_cast<FeedGroupItem*>(findNodeItem(parent));
    
    if (node->isGroup())
    {
        FeedGroup* fg = static_cast<FeedGroup*>(node);  
                       
        FeedGroupItem* fgi = static_cast<FeedGroupItem*> (m_itemDict.find(node));
        if (!fgi)
        {
            if (prevItem)
                fgi = new FeedGroupItem( parentItem, prevItem, fg);
            else
                fgi = new FeedGroupItem( parentItem, fg);
            
            m_itemDict.insert(fg, fgi);
        }
        else
        {
            insertNode(parentItem, fgi, prevItem);    
            if (!selectedItem())
                setSelected(fgi, true);
        }
            
        connect(fg, SIGNAL(signalChildAdded(FeedGroup*, TreeNode*)), this, SLOT(slotNodeAdded(FeedGroup*, TreeNode*) ));
        connect(fg, SIGNAL(signalChildRemoved(FeedGroup*, TreeNode*)), this, SLOT(slotNodeAdded(FeedGroup*, TreeNode*) ));
        connect(fg, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotNodeDestroyed(TreeNode*) ));
        connect(fg, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotNodeChanged(TreeNode*) ));
    }
    else
    {
        Feed* f = static_cast<Feed*> (node);
        FeedGroupItem* parent = static_cast<FeedGroupItem*>(findNodeItem(node->parent()));
        FeedItem* fi = static_cast<FeedItem*> (m_itemDict.find(f));
        if (!fi)
        {   
            if (prevItem)        
                fi = new FeedItem( parent, prevItem, f );
            else
                fi = new FeedItem( parent, f );
                m_itemDict.insert(f, fi);
        }
        else
        {
            insertNode(parentItem, fi, prevItem);            
            if (!selectedItem())
                setSelected(fi, true);
        }    
        connect(f, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotNodeDestroyed(TreeNode*) ));
        connect(f, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotNodeChanged(TreeNode*) ));
        connect(f, SIGNAL(fetchStarted(Feed*)), this, SLOT(slotFeedFetchStarted(Feed*)));
        connect(f, SIGNAL(fetchAborted(Feed*)), this, SLOT(slotFeedFetchAborted(Feed*)));
        connect(f, SIGNAL(fetchError(Feed*)), this, SLOT(slotFeedFetchError(Feed*)));
        connect(f, SIGNAL(fetched(Feed*)), this, SLOT(slotFeedFetchCompleted(Feed*)));
    }        
//     kdDebug() << "leave FeedsTree::slotNodeAdded node: " << node->title() << endl;
}

void FeedsTree::slotNodeRemoved(FeedGroup* parent, TreeNode* node)
{
//     kdDebug() << "enter FeedsTree::slotNodeRemoved node: " << (node ? node->title() : "null") << endl;
    if (!node)
        return;
    
    if (node->isGroup())
    {
        FeedGroup* fg = static_cast<FeedGroup*> (node);
        disconnect(fg, SIGNAL(signalChildAdded(FeedGroup*, TreeNode*)), this, SLOT(slotNodeAdded(FeedGroup*, TreeNode*) ));
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
    
    takeNode(findNodeItem(node));
//     kdDebug() << "leave FeedsTree::slotNodeRemoved node: " << (node ? node->title() : "null") << endl;
}
    
void FeedsTree::slotNodeDestroyed(TreeNode* node)
{
    delete findNodeItem(node);
    m_itemDict.remove(node);
}

void FeedsTree::slotNodeChanged(TreeNode* node)
{
//     kdDebug() << "enter FeedsTree::slotNodeChanged node: " << (node ? node->title() : "null") << endl;
    TreeNodeItem* item = findNodeItem(node);    
    if (item)
    {    
        item->nodeChanged();
        triggerUpdate();
    }    
//     kdDebug() << "leave FeedsTree::slotNodeChanged node: " << (node ? node->title() : "null") << endl;
}

#include "feedstree.moc"
