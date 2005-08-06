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

#include "listtabwidget.h"
#include "feedlistview.h"
#include "folder.h"
#include "treenode.h"

#include <ktabwidget.h>

#include <qiconset.h>
#include <qlayout.h>
#include <qmap.h>
#include <qstring.h>
#include <qvaluelist.h>

namespace Akregator {

class ListTabWidget::ListTabWidgetPrivate
{

public:
    KTabWidget* tabWidget;
    NodeListView* current;
    QValueList<NodeListView*> views;
    QGridLayout* layout;
    ViewMode viewMode;
    QMap<QWidget*, QString> captions;
};


void ListTabWidget::slotItemUp()
{
    if (d->current)
        d->current->slotItemUp();
}

void ListTabWidget::slotItemDown()
{
    if (d->current)
        d->current->slotItemDown();
}

void ListTabWidget::slotItemBegin()
{
    if (d->current)
        d->current->slotItemBegin();
}

void ListTabWidget::slotItemEnd()
{
    if (d->current)
        d->current->slotItemEnd();
}

void ListTabWidget::slotItemLeft()
{
    if (d->current)
        d->current->slotItemLeft();
}

void ListTabWidget::slotItemRight()
{
    if (d->current)
        d->current->slotItemRight();
}

void ListTabWidget::slotPrevFeed()
{
    if (d->current)
        d->current->slotPrevFeed();
}

void ListTabWidget::slotNextFeed()
{
    if (d->current)
        d->current->slotNextFeed();
}

void ListTabWidget::slotPrevUnreadFeed()
{
    if (d->current)
        d->current->slotPrevUnreadFeed();
}

void ListTabWidget::slotNextUnreadFeed()
{
    if (d->current)
        d->current->slotNextUnreadFeed();
}

void ListTabWidget::slotRootNodeChanged(NodeListView* view, TreeNode* node)
{
    int unread = node->unread();
    if (unread > 0)
    {
        //uncomment this to append unread count
        //d->tabWidget->changeTab(view,  QString("<qt>%1 (%2)").arg(d->captions[view]).arg(node->unread()));
        d->tabWidget->changeTab(view, d->captions[view]);
    }
    else
    {
        d->tabWidget->changeTab(view, d->captions[view]);
    }
}

void ListTabWidget::slotCurrentChanged(QWidget* current)
{
    d->current = dynamic_cast<NodeListView*>(current);
    emit signalNodeSelected(d->current->selectedNode());
}

ListTabWidget::ListTabWidget(QWidget* parent, const char* name) : QWidget(parent, name), d(new ListTabWidgetPrivate)
{
   d->current = 0;
   d->viewMode = horizTabs;
   d->layout = new QGridLayout(this, 1, 1);
   d->tabWidget = new KTabWidget(this); 
   d->layout->addWidget(d->tabWidget, 0, 0);
   connect(d->tabWidget, SIGNAL(currentChanged(QWidget*)), this, SLOT(slotCurrentChanged(QWidget*)));
}

ListTabWidget::~ListTabWidget()
{
    delete d;
    d = 0;
}

/*
void ListTabWidget::setViewMode(ViewMode mode)
{
    if (mode == d->viewMode)
        return;

    // TODO
}*/

ListTabWidget::ViewMode ListTabWidget::viewMode() const
{
    return d->viewMode;
}

void ListTabWidget::setViewIconSet(NodeListView* view, const QIconSet& iconSet)
{
    
    switch (d->viewMode)
    {
        case horizTabs:
            d->tabWidget->setTabIconSet(view, iconSet);
            break;
        default:
            break;
    }
}

void ListTabWidget::addView(NodeListView* view, const QString& caption)
{
    d->views.append(view);
    d->captions[view] = caption;
    connect(view, SIGNAL(signalNodeSelected(TreeNode*)), this, SIGNAL(signalNodeSelected(TreeNode*)));
    connect(view, SIGNAL(signalRootNodeChanged(NodeListView*, TreeNode*)), this, SLOT(slotRootNodeChanged(NodeListView*, TreeNode*)));

    switch (d->viewMode)
    {
        case horizTabs:
            d->tabWidget->addTab(view, caption);
            if (d->tabWidget->count() == 1)
                d->current = view;
            break;
        default:
            break;
    }
   
    
}

NodeListView* ListTabWidget::activeView() const
{
    return d->current;
}

}

#include "listtabwidget.h"

#include "listtabwidget.moc"
