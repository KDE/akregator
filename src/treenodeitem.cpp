/***************************************************************************
 *   Copyright (C) 2004 by Frank Osterfeld                                 *
 *   frank.osterfeld AT kdemail.net                                      *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "treenode.h"
#include "treenodeitem.h"
#include "feedgroupitem.h"
#include <qfont.h>
#include <qheader.h>
#include <qpainter.h>

#include <kstringhandler.h>

#include <kdebug.h>

using namespace Akregator;

TreeNodeItem::TreeNodeItem(FeedGroupItem* parent, TreeNode* node)
    : KListViewItem(parent), m_node(node)
{
    if (node)
        setText(0, node->title() );
}

TreeNodeItem::TreeNodeItem(KListView* parent, TreeNode* node) 
    : KListViewItem(parent), m_node(node)
{
    if (node)
         setText(0, node->title() );
}

TreeNodeItem::TreeNodeItem(FeedGroupItem* parent, TreeNodeItem* after, TreeNode* node)
    : KListViewItem(parent, after), m_node(node)
{
    if (node)
        setText(0, node->title() );
}
    

TreeNodeItem::~TreeNodeItem()
{
}

TreeNode* TreeNodeItem::node()
{ 
    return m_node; 
}

void TreeNodeItem::nodeChanged()
{
//    kdDebug() << "enter TreeNodeItem::nodeChanged item" << text(0) << endl; 
    if (!node())
        return;
    if (text(0) != node()->title())
        setText(0, node()->title());
//    kdDebug() << "leave TreeNodeItem::nodeChanged item" << text(0) << endl; 
}

TreeNodeItem* TreeNodeItem::firstChild() const 
{ 
    return static_cast<TreeNodeItem*>(KListViewItem::firstChild()); 
} 

TreeNodeItem* TreeNodeItem::nextSibling() const 
{ 
    return static_cast<TreeNodeItem*>(KListViewItem::nextSibling()); 
} 

FeedGroupItem* TreeNodeItem::parent() const 
{ 
    return static_cast<FeedGroupItem*>(KListViewItem::parent()); 
} 
    
/*
void TreeNodeItem::setUnread(int u)
{
    if (m_unread==u)
        return;

    m_unread=u;
    updateParentsRecursive();
}

void TreeNodeItem::updateParentsRecursive()
{
    repaint();
    if (parent())
    {
        static_cast<TreeNodeItem*>(parent())->updateParentsRecursive();
    }
}
*/
// TODO: reverse for reverse layout
void TreeNodeItem::paintCell( QPainter * p, const QColorGroup & cg,
                               int column, int width, int align )

{
    int u = node() ? node()->unread() : 0;

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
/*
int TreeNodeItem::countUnreadRecursive()
{
    int count = (m_unread > 0) ? m_unread : 0;

    for ( QListViewItem *item = firstChild() ;
          item ; item = item->nextSibling() ) {
              count += static_cast<TreeNodeItem*>(item)->countUnreadRecursive();
          }

          return count;
}
*/
