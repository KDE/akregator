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

#ifndef AKREGATOR_LISTTABWIDGET_H
#define AKREGATOR_LISTTABWIDGET_H

#include <qwidget.h>
//Added by qt3to4:
#include <QPixmap>

class QIcon;
class QPixmap;
class QString;

namespace Akregator {

class NodeListView;
class TreeNode;

/** A widget containing multiple list views, e.g. feed list, tag list etc.
    It also forwards slot calls triggered by keyboard shortcuts to the currently active view.
    Lists are added to a tab widget

    @author Frank Osterfeld
 */
class ListTabWidget : public QWidget
{

Q_OBJECT

public:

    ListTabWidget(QWidget* parent=0, const char* name=0);
    virtual ~ListTabWidget();

    enum ViewMode { /*single, horizontalTabs, */  verticalTabs };

    // void setViewMode(ViewMode mode); // TODO
    ViewMode viewMode() const;

    void addView(NodeListView* view, const QString& caption, const QPixmap& icon);

    NodeListView* activeView() const;

public slots:
    
    /** go one item up */
    void slotItemUp();
    /** go one item down */
    void slotItemDown();
    /** select the first item in the list */
    void slotItemBegin();
    /** select last item in the list */
    void slotItemEnd();
    /** go to parent item */
    void slotItemLeft();
    /** go to first child */
    void slotItemRight();

    void slotPrevFeed();
    void slotNextFeed();
    void slotPrevUnreadFeed();
    void slotNextUnreadFeed();

signals:
    void signalNodeSelected(TreeNode*);

protected slots:

    void slotRootNodeChanged(NodeListView*, TreeNode*);
    void slotTabClicked(int id);
 
private:
    class ListTabWidgetPrivate;
    ListTabWidgetPrivate* d;
};

} // namespace Akregator

#endif // AKREGATOR_LISTTABWIDGET_H
