/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
#include "feedstree.h"

#include <kdebug.h>
#include <klocale.h>
#include <kstringhandler.h>
#include <kurldrag.h>

#include <qfont.h>
#include <qheader.h>
#include <qpainter.h>
#include <qwhatsthis.h>

using namespace Akregator;

FeedsTreeItem::FeedsTreeItem( bool isF, QListView *parent, QString label)
    : KListViewItem( parent, label ), m_unread(0), m_folder(isF)
{
}

FeedsTreeItem::FeedsTreeItem( bool isF, QListViewItem *parent, QString label)
    : KListViewItem( parent, label ), m_unread(0), m_folder(isF)

{
}

FeedsTreeItem::FeedsTreeItem(bool isF, QListView *parent, QListViewItem *after, QString label)
    : KListViewItem( parent, after, label ), m_unread(0), m_folder(isF)
{
}

FeedsTreeItem::FeedsTreeItem(bool isF, QListViewItem *parent, QListViewItem *after, QString label)
    : KListViewItem( parent, after, label ), m_unread(0), m_folder(isF)
{
}

FeedsTreeItem::~FeedsTreeItem()
{
}

void FeedsTreeItem::setUnread(int u)
{
    if (m_unread==u)
        return;

    m_unread=u;
    updateParentsRecursive();
}

bool FeedsTreeItem::isFolder()
{
    return m_folder;
}

void FeedsTreeItem::setFolder(bool f)
{
    m_folder=f;
}

void FeedsTreeItem::updateParentsRecursive()
{
    repaint();
    if (parent())
    {
        static_cast<FeedsTreeItem*>(parent())->updateParentsRecursive();
    }
}

// TODO: reverse for reverse layout
void FeedsTreeItem::paintCell( QPainter * p, const QColorGroup & cg,
                                    int column, int width, int align )

{
    int u=countUnreadRecursive();

    if (u <= 0)
    {
        KListViewItem::paintCell(p,cg,column,width,align);
        return;
    }

    // from kfoldertree
    QString oldText = text(column);
    setText( column, " " );

    // draw bg
    KListViewItem::paintCell(p,cg,column,width,align);

    setText( column, oldText);

    // draw fg
    QFont f = p->font();
    f.setWeight(QFont::Bold);
    p->setFont(f);

    QFontMetrics fm( p->fontMetrics() );
    QListView *lv = listView();
    int x = lv ? lv->itemMargin() : 1;
    int m=x;
    const QPixmap *icon = pixmap( column );
    QRect br;

    if (icon)
        x += icon->width() + m;

    QString txt = " (" + QString::number(u) + ")";
    int txtW=fm.width( txt );

    if (fm.width( oldText ) + txtW + x > width)
        oldText=KStringHandler::rPixelSqueeze(oldText,fm, width - txtW - x);

    p->drawText( x, 0, width-m-x, height(), align | AlignVCenter, oldText, -1, &br );

    if ( !isSelected() )
        p->setPen( Qt::blue ); // TODO: configurable

    p->drawText( br.right(), 0, width-m-br.right(), height(),
                            align | AlignVCenter, txt );

    /*if ( isSelected() )
        p->setPen( cg.highlightedText() );
    else
        p->setPen( cg.text() );*/


}

int FeedsTreeItem::countUnreadRecursive()
{
    int count = (m_unread > 0) ? m_unread : 0;

    for ( QListViewItem *item = firstChild() ;
            item ; item = item->nextSibling() ) {
        count += static_cast<FeedsTreeItem*>(item)->countUnreadRecursive();
    }

    return count;
}



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

void FeedsTree::drawContentsOffset( QPainter * p, int ox, int oy,
                                       int cx, int cy, int cw, int ch )
{
    bool oldUpdatesEnabled = isUpdatesEnabled();
    setUpdatesEnabled(false);
    KListView::drawContentsOffset( p, ox, oy, cx, cy, cw, ch );
    setUpdatesEnabled(oldUpdatesEnabled);
}

void FeedsTree::slotDropped( QDropEvent *e, QListViewItem * )
{
    if (!acceptDrag(e))
        return;

    // disable any drops where the result would be top level nodes
    QListViewItem *afterme;
    QListViewItem *parent;
    findDrop( e->pos(), parent, afterme );
        
    if (!parent)
    {
        e->ignore();
        return;
    }

    if (e->source() != viewport())
    {
        if (KURLDrag::canDecode( e ))
        {
            QListViewItem *afterme;
            QListViewItem *parent;
            findDrop( e->pos(), parent, afterme );
            KURL::List urls;
            KURLDrag::decode( e, urls );
            e->accept();
            emit dropped( urls, afterme, parent);
        }
        else
        {
            e->ignore();
        }
    }
}

void FeedsTree::movableDropEvent(QListViewItem* parent, QListViewItem* afterme)
{
    if (parent)
        KListView::movableDropEvent(parent, afterme);
}

void FeedsTree::contentsDragMoveEvent(QDragMoveEvent* event)
{
    // if we are dragging over All feeds, enable
    QPoint vp = contentsToViewport(event->pos());
    QListViewItem *i= itemAt(vp);
    if (i==firstChild())
    {
        event->accept();
        return;
    }

    // disable any drops where the result would be top level nodes
    QListViewItem *afterme;
    QListViewItem *parent;
    findDrop( event->pos(), parent, afterme );
    
    if (!parent)
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
    if (currentItem())
        currentItem()->setOpen(false);
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
    if (currentItem())
        currentItem()->setOpen(true);
}

void FeedsTree::slotItemUp()
{
    if (currentItem())
        setCurrentItem(currentItem()->itemAbove());
}

void FeedsTree::slotItemDown()
{
    if (currentItem())
        setCurrentItem(currentItem()->itemBelow());
}

void FeedsTree::slotItemBegin()
{
    setCurrentItem(firstChild());
}

void FeedsTree::slotItemEnd()
{
    QListViewItem* elt = firstChild();
    if (elt)
        while (elt->itemBelow())
            elt = elt->itemBelow();
    setCurrentItem(elt);
}

void FeedsTree::slotItemLeft()
{
    if (currentItem())
        setCurrentItem(currentItem()->parent());
}

void FeedsTree::slotItemRight()
{
    if (currentItem())
        setCurrentItem(currentItem()->firstChild());
}

#include "feedstree.moc"
