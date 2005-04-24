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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef AKREGATOR_ACTIONMANAGER_H
#define AKREGATOR_ACTIONMANAGER_H

#include <qobject.h>

class KAction;
class KActionCollection;
class KXMLGUIClient;

namespace Akregator {

class ArticleListView;
class ArticleViewer;
class FeedListView;
class Part;
class TrayIcon;
class View;

class ActionManager : public QObject
{
    Q_OBJECT

    public:

        static ActionManager* getInstance();
        static void setInstance(ActionManager* manager);

        ActionManager(KXMLGUIClient* client, QObject* parent=0, const char* name=0);
        virtual ~ActionManager();

        void initPart(Part* part);
        void initView(View* view);
        void initTrayIcon(TrayIcon* trayIcon);
        void initArticleViewer(ArticleViewer* articleViewer);
        void initArticleListView(ArticleListView* articleList);
        void initFeedListView(FeedListView* feedListView);
        
        KActionCollection* actionCollection();
        KAction* action(const char* name, const char* classname=0);

    private:

        static ActionManager* m_self;
        KXMLGUIClient* m_client;
        ArticleListView* m_articleList;
        FeedListView* m_feedListView;
        View* m_view;
        ArticleViewer* m_articleViewer;
        Part* m_part;
        TrayIcon* m_trayIcon;
        
        KActionCollection* m_actionCollection;
};

} // namespace Akregator

#endif // AKREGATOR_ACTIONMANAGER_H
