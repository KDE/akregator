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

#ifndef AKREGATOR_ACTIONMANAGERIMPL_H
#define AKREGATOR_ACTIONMANAGERIMPL_H

#include "actionmanager.h"

class QStringList;
class QWidget;
class KAction;
class KActionMenu;
class KActionCollection;

namespace Akregator {

class ArticleListView;
class ArticleViewer;
class ListTabWidget;
class Part;
class TrayIcon;
class Tag;
class TagSet;
class TreeNode;
class View;
class TabWidget;

/** 
 * Akregator-specific implementation of the ActionManager interface
 */
class ActionManagerImpl : public ActionManager
{
    Q_OBJECT

    public:
        ActionManagerImpl(Part* part, QObject* parent=0, const char* name=0);
        virtual ~ActionManagerImpl();

        virtual KAction* action(const char* name, const char* classname=0);
        virtual QWidget* container(const char* name);

        void initView(View* view);
        void initTrayIcon(TrayIcon* trayIcon);
        void initArticleViewer(ArticleViewer* articleViewer);
        void initArticleListView(ArticleListView* articleList);
        void initListTabWidget(ListTabWidget* listTabWidget);
        void initTabWidget(TabWidget* tabWidget);
        void setTagSet(TagSet* tagSet);

    public slots:

        /** fills the remove tag menu with the given tags
            enables/disables tag menu action according to @c enabled */
        void slotUpdateTagActions(bool enabled, const QStringList& tagIds);
        
        void slotNodeSelected(TreeNode* node);
        
        void slotTagAdded(const Tag& tag);
        void slotTagRemoved(const Tag& tag);
        
    protected:
    
        KActionCollection* actionCollection();
        
    private:

        void initPart();

        friend class NodeSelectVisitor;
        class NodeSelectVisitor;
        
        class ActionManagerImplPrivate;
        ActionManagerImplPrivate* d;
};

} // namespace Akregator

#endif // AKREGATOR_ACTIONMANAGERIMPL_H
