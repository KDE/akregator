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

#include <kmultitabbar.h>

#include <qicon.h>
#include <qlayout.h>
#include <qmap.h>
#include <q3ptrlist.h>
#include <qstring.h>
#include <q3valuelist.h>
#include <q3widgetstack.h>
//Added by qt3to4:
#include <QPixmap>
#include <QGridLayout>

#include <kdebug.h>

namespace Akregator {

class ListTabWidget::ListTabWidgetPrivate
{

public:
    int idCounter;
    KMultiTabBar* tabBar;
    Q3WidgetStack* stack;
    NodeListView* current;
    int currentID;
    Q3ValueList<NodeListView*> views;
    QMap<int, NodeListView*> idToView;
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
    Q_UNUSED(view)
    Q_UNUSED(node)
/*
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
*/
}

void ListTabWidget::slotTabClicked(int id)
{
    NodeListView* view = d->idToView[id];
    if (view)
    {
        d->stack->raiseWidget(view);
        d->current = view;

        if (d->currentID >= 0)
            d->tabBar->setTab(d->currentID, false);
        d->currentID = id;
        d->tabBar->setTab(d->currentID, true);

        emit signalNodeSelected(d->current->selectedNode());
    }
}

ListTabWidget::ListTabWidget(QWidget* parent, const char* name) : QWidget(parent, name), d(new ListTabWidgetPrivate)
{
    d->idCounter = 0;
    d->current = 0;
    d->currentID = -1;
    d->viewMode = verticalTabs;
    d->layout = new QGridLayout(this, 1, 2);
    d->tabBar = new KMultiTabBar(KMultiTabBar::Vertical, this); 
    d->tabBar->setStyle(KMultiTabBar::KDEV3ICON);
    //d->tabBar->setStyle(KMultiTabBar::KDEV3);
    d->tabBar->showActiveTabTexts(true);
    d->tabBar->setPosition(KMultiTabBar::Left);
    d->layout->addWidget(d->tabBar, 0, 0);

    d->stack = new Q3WidgetStack(this);
    d->layout->addWidget(d->stack, 0, 1);
    
//    connect(d->tabBar, SIGNAL(currentChanged(QWidget*)), this, SLOT(slotCurrentChanged(QWidget*)));
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

void ListTabWidget::addView(NodeListView* view, const QString& caption, const QPixmap& icon)
{
    d->captions[view] = caption;    

    view->reparent(d->stack, QPoint(0,0));
    d->stack->addWidget(view);
   
    int tabId = d->idCounter++;
    d->tabBar->appendTab(icon, tabId, caption);
    d->idToView[tabId] = view;
    connect(d->tabBar->tab(tabId), SIGNAL(clicked(int)), this, SLOT(slotTabClicked(int)));

    
    connect(view, SIGNAL(signalNodeSelected(TreeNode*)), this, SIGNAL(signalNodeSelected(TreeNode*)));
    connect(view, SIGNAL(signalRootNodeChanged(NodeListView*, TreeNode*)), this, SLOT(slotRootNodeChanged(NodeListView*, TreeNode*)));


    if (tabId == 0) // first widget
    {
        d->current = view;
        d->currentID = tabId;
        d->tabBar->setTab(d->currentID, true);
        d->stack->raiseWidget(view);
    }
}

NodeListView* ListTabWidget::activeView() const
{
    return d->current;
}

}

#include "listtabwidget.h"

#include "listtabwidget.moc"
