/*
    This file is part of Akregator.

    Copyright (C) 2004 Frank Osterfeld <osterfeld@kde.org>

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

#include "folderitem.h"
#include "treenode.h"
#include "treenodeitem.h"

#include <kdebug.h>

#include <QFont>
#include <QPainter>
#include <QPixmap>
#include <QString>


namespace Akregator {

TreeNodeItem::TreeNodeItem(FolderItem* parent, TreeNode* node)
    : K3ListViewItem(parent), m_node(node)
{
    initialize(node);
}

TreeNodeItem::TreeNodeItem(K3ListView* parent, TreeNode* node)
    : K3ListViewItem(parent), m_node(node)
{
    initialize(node);
}

TreeNodeItem::TreeNodeItem(K3ListView* parent, TreeNodeItem* after, TreeNode* node) : K3ListViewItem(parent, after), m_node(node)
{
    initialize(node);
}

TreeNodeItem::TreeNodeItem(FolderItem* parent, TreeNodeItem* after, TreeNode* node)
    : K3ListViewItem(parent, after), m_node(node)
{
    initialize(node);
}

void TreeNodeItem::initialize(TreeNode* node)
{
    setRenameEnabled(0, true);
    if (node)
        setText(0, node->title() );
}

TreeNodeItem::~TreeNodeItem()
{}

QString TreeNodeItem::toolTip() const
{
    return QString();
}

TreeNode* TreeNodeItem::node()
{
    return m_node;
}

void TreeNodeItem::nodeChanged()
{
//    kDebug() <<"enter TreeNodeItem::nodeChanged item" << text(0);
    if (!node())
        return;
    if (text(0) != node()->title())
        setText(0, node()->title());
//    kDebug() <<"leave TreeNodeItem::nodeChanged item" << text(0);
}

TreeNodeItem* TreeNodeItem::firstChild() const
{
    return static_cast<TreeNodeItem*>(K3ListViewItem::firstChild());
}

TreeNodeItem* TreeNodeItem::nextSibling() const
{
    return static_cast<TreeNodeItem*>(K3ListViewItem::nextSibling());
}

FolderItem* TreeNodeItem::parent() const
{
    return static_cast<FolderItem*>(K3ListViewItem::parent());
}


// TODO: reverse for reverse layout
void TreeNodeItem::paintCell( QPainter * p, const QColorGroup & cg,
                               int column, int width, int align )

{
    int u = node() ? node()->unread() : 0;

    if (u <= 0)
    {
        K3ListViewItem::paintCell(p,cg,column,width,align);
        return;
    }

    // from kfoldertree
    QString oldText = text(column);
    setText( column, " " );

    // draw bg
    K3ListViewItem::paintCell(p,cg,column,width,align);

    setText( column, oldText);

    // draw fg
    QFont f = p->font();
    f.setWeight(QFont::Bold);
    p->setFont(f);

    QFontMetrics fm( p->fontMetrics() );
    Q3ListView *lv = listView();
    int x = lv ? lv->itemMargin() : 1;
    int m=x;
    const QPixmap *icon = pixmap( column );
    QRect br;

    if (icon)
        x += icon->width() + m;

    QString txt = " (" + QString::number(u) + ')';
    int txtW=fm.width( txt );

    if (fm.width( oldText ) + txtW + x > width)
        oldText = fm.elidedText(oldText, Qt::ElideRight, width - txtW - x);

    p->drawText( x, 0, width-m-x, height(), align | Qt::AlignVCenter, oldText, &br );

    if ( !isSelected() )
        p->setPen( Qt::blue ); // TODO: configurable

    p->drawText( br.right(), 0, width-m-br.right(), height(),
                 align | Qt::AlignVCenter, txt );

    /*if ( isSelected() )
    p->setPen( cg.highlightedText() );
    else
    p->setPen( cg.text() );*/
}

} // namespace Akregator
