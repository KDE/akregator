/*
    This file is part of Akregator.

    Copyright (C) 2004 Frank Osterfeld <frank.osterfeld at kdemail.net>

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
{}

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

