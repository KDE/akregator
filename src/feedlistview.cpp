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

#include "dragobjects.h"
#include "folder.h"
#include "folderitem.h"
#include "tagfolder.h"
#include "tagfolderitem.h"
#include "feedlistview.h"
#include "feed.h"
#include "feeditem.h"
#include "feedlist.h"
#include "tag.h"
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

class NodeListView::NodeListViewPrivate
{
    public:
/** used for finding the item belonging to a node */
    QHash<TreeNode*, TreeNodeItem*> itemDict;
    NodeList* nodeList;
    bool showTagFolders;

    // Drag and Drop variables
    Q3ListViewItem *parent;
    Q3ListViewItem *afterme;
    QTimer autoopentimer;
    ConnectNodeVisitor* connectNodeVisitor;
    DisconnectNodeVisitor* disconnectNodeVisitor;
    CreateItemVisitor* createItemVisitor;
    DragAndDropVisitor* dragAndDropVisitor;
};

class NodeListView::DragAndDropVisitor : public TreeNodeVisitor
{

public:
    DragAndDropVisitor(NodeListView* view) : m_view(view) {}

    /*
    virtual bool visitTagNode(TagNode* node)
    {
        if (m_mode == ArticlesDropped)
        {
            Tag tag = node->tag();
            QValueList<ArticleDragItem>::ConstIterator end(m_items.end());
            for (QValueList<ArticleDragItem>::ConstIterator it = m_items.begin(); it != end; ++it)
            {
                Article a = Kernel::self()->feedList()->findArticle((*it).feedURL, (*it).guid);
                if (!a.isNull())
                     a.addTag(tag.id());
            }
        }
        return true;
    }
*/
    void articlesDropped(TreeNode* node, const QList<ArticleDragItem>& items)
    {
        m_items = items;
        m_mode = ArticlesDropped;
        visit(node);
    }

private:
    NodeListView* m_view;
    QList<ArticleDragItem> m_items;
     
    enum Mode { ArticlesDropped };
    Mode m_mode;
};

class NodeListView::ConnectNodeVisitor : public TreeNodeVisitor
{
    public:
        ConnectNodeVisitor(NodeListView* view) : m_view(view) {}

        virtual bool visitTreeNode(TreeNode* node)
        {
            connect(node, SIGNAL(signalDestroyed(TreeNode*)), m_view, SLOT(slotNodeDestroyed(TreeNode*) ));
            connect(node, SIGNAL(signalChanged(TreeNode*)), m_view, SLOT(slotNodeChanged(TreeNode*) ));
            return true;
        }

        virtual bool visitFolder(Folder* node)
        {
            visitTreeNode(node);
            connect(node, SIGNAL(signalChildAdded(TreeNode*)), m_view, SLOT(slotNodeAdded(TreeNode*) ));
            connect(node, SIGNAL(signalChildRemoved(Folder*, TreeNode*)), m_view, SLOT(slotNodeRemoved(Folder*, TreeNode*) ));
            return true;
        }
        
        virtual bool visitFeed(Feed* node)
        {
            visitTreeNode(node);
            
            connect(node, SIGNAL(fetchStarted(Feed*)), m_view, SLOT(slotFeedFetchStarted(Feed*)));
            connect(node, SIGNAL(fetchAborted(Feed*)), m_view, SLOT(slotFeedFetchAborted(Feed*)));
            connect(node, SIGNAL(fetchError(Feed*)), m_view, SLOT(slotFeedFetchError(Feed*)));
            connect(node, SIGNAL(fetched(Feed*)), m_view, SLOT(slotFeedFetchCompleted(Feed*)));
            return true;
        }
    private:

        NodeListView* m_view;
    
};

class NodeListView::DisconnectNodeVisitor : public TreeNodeVisitor
{
    public:
        DisconnectNodeVisitor(NodeListView* view) : m_view(view) {}

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

        NodeListView* m_view;
};

class NodeListView::CreateItemVisitor : public TreeNodeVisitor
{
    public:
        CreateItemVisitor(NodeListView* view) : m_view(view) {}

        virtual bool visitTagNode(TagNode* node)
        {
            kdDebug() << "create item for " << node->title() << endl;
            TagNodeItem* item = 0;
            TreeNode* prev = node->prevSibling();
            FolderItem* parentItem = static_cast<FolderItem*>(m_view->findNodeItem(node->parent()));
            if (parentItem)
            {
                if (prev)
                {
                    item = new TagNodeItem( parentItem, m_view->findNodeItem(prev), node);
                }
                else
                    item = new TagNodeItem( parentItem, node);
            }
            else
            {
                if (prev)
                {
                    item = new TagNodeItem(m_view, m_view->findNodeItem(prev), node);
                }
                else
                    item = new TagNodeItem(m_view, node);
            }                
            item->nodeChanged();     
            m_view->d->itemDict.insert(node, item);
            m_view->connectToNode(node);
            parentItem->sortChildItems(0, true);
            return true;
        }

        virtual bool visitTagFolder(TagFolder* node)
        {
            TagFolderItem* item = 0;
            TreeNode* prev = node->prevSibling();
            FolderItem* parentItem = static_cast<FolderItem*>(m_view->findNodeItem(node->parent()));
            if (parentItem)
            {
                if (prev)
                {
                    item = new TagFolderItem( parentItem, m_view->findNodeItem(prev), node);
                }
                else
                    item = new TagFolderItem(parentItem, node);
            }
            else
            {
                if (prev)
                {
                    item = new TagFolderItem(m_view, m_view->findNodeItem(prev), node);
                }
                else
                    item = new TagFolderItem(m_view, node);

            }
            m_view->d->itemDict.insert(node, item);
            QList<TreeNode*> children = node->children();

            // add children recursively
            for (QList<TreeNode*>::ConstIterator it =  children.begin(); it != children.end(); ++it )
                visit(*it);

            m_view->connectToNode(node);
            return true;
        }
        
        virtual bool visitFolder(Folder* node)
        {
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
        NodeListView* m_view;
};

NodeListView::NodeListView( QWidget *parent, const char *name)
        : KListView(parent), d(new NodeListViewPrivate)
{
    setObjectName(name);
    d->showTagFolders = true;
    d->connectNodeVisitor = new ConnectNodeVisitor(this),
    d->disconnectNodeVisitor = new DisconnectNodeVisitor(this);
    d->createItemVisitor = new CreateItemVisitor(this);
    d->dragAndDropVisitor = new DragAndDropVisitor(this);

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
    connect( this, SIGNAL(contextMenu(KListView*, Q3ListViewItem*, const QPoint&)), this, SLOT(slotContextMenu(KListView*, Q3ListViewItem*, const QPoint&)) );
    connect( &(d->autoopentimer), SIGNAL( timeout() ), this, SLOT( openFolder() ) );

    clear();
    
    this->setWhatsThis( i18n("<h2>Feeds tree</h2>"
        "Here you can browse tree of feeds. "
        "You can also add feeds or feed groups (folders) "
        "using right-click menu, or reorganize them using "
        "drag and drop."));
    setUpdatesEnabled(true);
}

NodeListView::~NodeListView()
{
    delete d->connectNodeVisitor;
    delete d->disconnectNodeVisitor;
    delete d->createItemVisitor;
    delete d->dragAndDropVisitor;
    delete d;
    d = 0;
}

void NodeListView::setNodeList(NodeList* nodeList)
{
    if (nodeList == d->nodeList)
         return;

    clear();

    disconnectFromNodeList(d->nodeList);
    
    if (!nodeList)
        return;

    d->nodeList = nodeList;
    connectToNodeList(nodeList);
  
    
    Folder* rootNode = nodeList->rootNode();
    if (!rootNode)
        return;

    slotNodeAdded(rootNode);
    slotRootNodeChanged(rootNode);
}

void NodeListView::takeNode(Q3ListViewItem* item)
{
    if (item->parent())
        item->parent()->takeItem(item);
    else
        takeItem(item);
}

void NodeListView::insertNode(Q3ListViewItem* parent, Q3ListViewItem* item, Q3ListViewItem* after)
{
    if (parent)
        parent->insertItem(item);
    else
        insertItem(item);
    if (after)
        item->moveItem(after);
}

Folder* NodeListView::rootNode()
{
    return d->nodeList ? d->nodeList->rootNode() : 0;
}

TreeNode* NodeListView::selectedNode()
{
    TreeNodeItem* item = dynamic_cast<TreeNodeItem*> (selectedItem());
    
    return ( item ? item->node() : 0) ;
}

void NodeListView::setSelectedNode(TreeNode* node)
{
    TreeNodeItem* item = findNodeItem(node);
    if ( node && item )
        setSelected(item, true);
}

TreeNode* NodeListView::findNodeByTitle(const QString& title)
{
    TreeNodeItem* item = dynamic_cast<TreeNodeItem*>(findItemByTitle(title, 0));
    if (!item)
        return 0;
    else 
        return item->node();
}

TreeNodeItem* NodeListView::findNodeItem(TreeNode* node)
{
    return d->itemDict[node];
}

TreeNodeItem* NodeListView::findItemByTitle(const QString& text, int column, ComparisonFlags compare) const
{ 
    return dynamic_cast<TreeNodeItem*> (KListView::findItem(text, column, compare)); 
}

void NodeListView::ensureNodeVisible(TreeNode* node)
{
    ensureItemVisible(findNodeItem(node));
}

void NodeListView::startNodeRenaming(TreeNode* node)
{
    TreeNodeItem* item = findNodeItem(node);
    if (item)
    {   
        item->startRename(0);
    }
}

void NodeListView::clear()
{
    QHash<TreeNode*, TreeNodeItem*>::Iterator it = d->itemDict.begin();
    for( ; it != d->itemDict.end(); ++it )
        disconnectFromNode( (*it)->node() );
    d->itemDict.clear();
    d->nodeList = 0;
    
    KListView::clear();
}

void NodeListView::drawContentsOffset( QPainter * p, int ox, int oy,
                                       int cx, int cy, int cw, int ch )
{
    bool oldUpdatesEnabled = isUpdatesEnabled();
    setUpdatesEnabled(false);
    KListView::drawContentsOffset( p, ox, oy, cx, cy, cw, ch );
    setUpdatesEnabled(oldUpdatesEnabled);
}

void NodeListView::slotDropped( QDropEvent *e, Q3ListViewItem*
/*after*/)
{
	d->autoopentimer.stop();

    if (e->source() != viewport())
    {
        openFolder();

        FolderItem* parent = dynamic_cast<FolderItem*> (d->parent);
        TreeNodeItem* afterMe = 0;

        if(d->afterme)
            afterMe = dynamic_cast<TreeNodeItem*> (d->afterme);

        if (ArticleDrag::canDecode(e))
        {
            QPoint vp = contentsToViewport(e->pos());
            TreeNodeItem* tni = dynamic_cast<TreeNodeItem*>(itemAt(vp));
            if (tni != 0 && tni->node() != 0)
            {
                QList<ArticleDragItem> items;
                ArticleDrag::decode(e, items);
                d->dragAndDropVisitor->articlesDropped(tni->node(), items);

            }
        }
        else if (K3URLDrag::canDecode(e))
        {
            KURL::List urls;
            K3URLDrag::decode( e, urls );
            e->accept();
            emit signalDropped( urls, afterMe ? afterMe->node() : 0, parent ? parent->node() : 0);
        }
    }
    else
    {
    }
}

void NodeListView::movableDropEvent(Q3ListViewItem* /*parent*/, Q3ListViewItem* /*afterme*/)
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
        KListView::movableDropEvent(d->parent, d->afterme);
    }    
}

void NodeListView::setShowTagFolders(bool enabled)
{
    d->showTagFolders = enabled;
}

void NodeListView::contentsDragMoveEvent(QDragMoveEvent* event)
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

    // the rest is handled by KListView.
    KListView::contentsDragMoveEvent(event);
}

bool NodeListView::acceptDrag(QDropEvent *e) const
{
    if (!acceptDrops() || !itemsMovable())
        return false;

    if (e->source() != viewport())
    {
        return K3URLDrag::canDecode(e);
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

void NodeListView::slotItemUp()
{
    if (selectedItem() && selectedItem()->itemAbove())
    {
        setSelected( selectedItem()->itemAbove(), true );
        ensureItemVisible(selectedItem());
    }   
}

void NodeListView::slotItemDown()
{
    if (selectedItem() && selectedItem()->itemBelow())
    {    
        setSelected( selectedItem()->itemBelow(), true );
        ensureItemVisible(selectedItem());
    }
}

void NodeListView::slotItemBegin()
{
    setSelected( firstChild(), true );
    ensureItemVisible(firstChild());
}

void NodeListView::slotItemEnd()
{
    Q3ListViewItem* elt = firstChild();
    if (elt)
        while (elt->itemBelow())
            elt = elt->itemBelow();
    setSelected( elt, true );
    ensureItemVisible(elt);
}

void NodeListView::slotItemLeft()
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

void NodeListView::slotItemRight()
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

void NodeListView::slotPrevFeed()
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
    
void NodeListView::slotNextFeed()
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

void NodeListView::slotPrevUnreadFeed()
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

void NodeListView::slotNextUnreadFeed()
{
    Q3ListViewItemIterator it;
    
    if ( !selectedItem() )
    {
        // if all feeds doesnt exists or is empty, return
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

void NodeListView::slotSelectionChanged(Q3ListViewItem* item)
{
 TreeNodeItem* ni = dynamic_cast<TreeNodeItem*> (item);
    if (ni)
        emit signalNodeSelected(ni->node());
}

void NodeListView::slotItemRenamed(Q3ListViewItem* item, int col, const QString& text)
{
    TreeNodeItem* ni = dynamic_cast<TreeNodeItem*> (item);
    if ( !ni || !ni->node() )
        return;
    if (col == 0)
    {
        if (text != ni->node()->title())
        {
            kdDebug() << "renamed item to \"" << text << "\"" << endl;
            ni->node()->setTitle(text);
        }
    }
}
void NodeListView::slotContextMenu(KListView* list, Q3ListViewItem* item, const QPoint& p)
{    
    TreeNodeItem* ti = dynamic_cast<TreeNodeItem*>(item);
    emit signalContextMenu(list, ti ? ti->node() : 0, p);
    if (ti)
        ti->showContextMenu(p);
}

void NodeListView::slotFeedFetchStarted(Feed* feed)
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

void NodeListView::slotFeedFetchAborted(Feed* feed)
{
    TreeNodeItem* item = findNodeItem(feed);
    if (item)
        item->nodeChanged();
}

void NodeListView::slotFeedFetchError(Feed* feed)
{
    TreeNodeItem* item = findNodeItem(feed);
    if (item)
        item->nodeChanged();
}

void NodeListView::slotFeedFetchCompleted(Feed* feed)
{
    TreeNodeItem* item = findNodeItem(feed);
    if (item)
        item->nodeChanged();
}
      
void NodeListView::slotNodeAdded(TreeNode* node)
{
    d->createItemVisitor->visit(node);
    kdDebug() << "NodeListView::slotNodeAdded: " << node->title() << endl;
}

void NodeListView::slotNodeRemoved(Folder* /*parent*/, TreeNode* node)
{
    if (!node)
        return;
    kdDebug() << "NodeListView::slotNodeRemoved: " << node->title() << endl; 
    disconnectFromNode(node);
    delete d->itemDict.take(node);
}

void NodeListView::connectToNode(TreeNode* node)
{
    if (node)
        d->connectNodeVisitor->visit(node);
}

void NodeListView::connectToNodeList(NodeList* list)
{
    if (!list)
        return;
    
    connect(list, SIGNAL(signalDestroyed(NodeList*)), this, SLOT(slotNodeListDestroyed(NodeList*)) );
    connect(list->rootNode(), SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotRootNodeChanged(TreeNode*)));
}

void NodeListView::disconnectFromNodeList(NodeList* list)
{
    if (!list)
        return;
    
    disconnect(list, SIGNAL(signalDestroyed(NodeList*)), this, SLOT(slotNodeListDestroyed(NodeList*)) );
    disconnect(list->rootNode(), SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotRootNodeChanged(TreeNode*)));
}

void NodeListView::disconnectFromNode(TreeNode* node)
{
    if (node)
        d->disconnectNodeVisitor->visit(node);
}

void NodeListView::slotNodeListDestroyed(NodeList* list)
{
    if (list != d->nodeList)
        return;

    setNodeList(0);
}

void NodeListView::slotNodeDestroyed(TreeNode* node)
{
    TreeNodeItem* item = findNodeItem(node);
    
    d->itemDict.remove(node);

    if (!item)
        return;
    
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

void NodeListView::slotRootNodeChanged(TreeNode* rootNode)
{
    emit signalRootNodeChanged(this, rootNode);
}

void NodeListView::slotNodeChanged(TreeNode* node)
{
    TreeNodeItem* item = findNodeItem(node);
    if (item)
    {    
        item->nodeChanged();
        triggerUpdate();
    }    
}

Q3DragObject *NodeListView::dragObject()
{
    K3MultipleDrag *md = new K3MultipleDrag(viewport());
    Q3DragObject *obj = KListView::dragObject();
    if (obj) {
        md->addDragObject(obj);
    }
    TreeNodeItem *i = dynamic_cast<TreeNodeItem*>(currentItem());
    if (i) {
        md->setPixmap(*(i->pixmap(0)));
        FeedItem *fi = dynamic_cast<FeedItem*>(i);
        if (fi) {
            md->addDragObject(new K3URLDrag(KURL(fi->node()->xmlUrl()), 0L));
        }
    }
    return md;
}

void NodeListView::openFolder() {
    d->autoopentimer.stop();
    if (d->parent && !d->parent->isOpen())
    {
        d->parent->setOpen(true);
    }
}

} // namespace Akregator

#include "feedlistview.moc"
