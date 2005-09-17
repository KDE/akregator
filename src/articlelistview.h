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
#ifndef AKREGATORARTICLELISTVIEW_H
#define AKREGATORARTICLELISTVIEW_H

#include <klistview.h>
#include <QList>
#include <QKeyEvent>
#include <QPaintEvent>

class QKeyEvent;
class Q3DragObject;
template <class T> class QList;

namespace Akregator
{
    class Article;
    class TreeNode;

    namespace Filters
    {
        class ArticleMatcher;
    }

    class ArticleListView : public KListView
    {
        Q_OBJECT
        public:
            ArticleListView(QWidget *parent = 0, const char *name = 0);
            virtual ~ArticleListView();
            
            /** returns the current article, or a null article if there is none */
            Article currentArticle() const;
            
            /** returns a list of currently selected articles */
            QList<Article> selectedArticles() const;
            
            enum Columns { itemTitle, feedTitle, pubDate };

        public slots:

            /** show article list of tree node @c node (also connects to the notification signals of the node) */
            void slotShowNode(TreeNode* node);
            
            /** clears the list and disconnects from the observed node (if any) */
            void slotClear();
            
            /** sets text filter and status filter
            @param textFilter filters text
            @param statusFilter filters status (read, unread, new) */
            void slotSetFilter(const Akregator::Filters::ArticleMatcher& textFilter, const Akregator::Filters::ArticleMatcher& statusFilter);

            /** selects previous article in list view, first article if no article was selected */
            void slotPreviousArticle();

            /** selects next article in list view, first article if no article was selected */
            void slotNextArticle();

            /** selects previous unread article in list view, first unread article if no article was selected */
            void slotPreviousUnreadArticle();

            /** selects next unread article in list view, first unread article if no article was selected */
            void slotNextUnreadArticle();

        signals:
            void signalArticleChosen(const Article& article);
            void signalDoubleClicked(const Article&, const QPoint&, int);
            //void signalContextMenu(KListView*, ArticleItem*, const QPoint&);
            void signalMouseButtonPressed(int, const Article&, const QPoint &, int);
             
        protected:
            /** reimplemented for kmail-like behaviour */            
            virtual void keyPressEvent(QKeyEvent* e);
            
            /** applies text filter and status filter by setting visibility
            of items accordingly */
            virtual void applyFilters();
            
            /**
             * @return count of visible articles, used for info boxes
             */
            int visibleArticles();
            
            /** Paints infobox for filtering and stuff
             */
            void paintInfoBox(const QString &message);
            
            virtual void viewportPaintEvent(QPaintEvent *e);
            
            void connectToNode(TreeNode* node);
            void disconnectFromNode(TreeNode* node);
            
            virtual Q3DragObject *dragObject();

        protected slots:

            void slotArticlesAdded(TreeNode* node, const QList<Article>& list);
            void slotArticlesUpdated(TreeNode* node, const QList<Article>& list);
            void slotArticlesRemoved(TreeNode* node, const QList<Article>& list);

            virtual void slotCurrentChanged(Q3ListViewItem* item);
            virtual void slotSelectionChanged();
            virtual void slotDoubleClicked(Q3ListViewItem* item, const QPoint& p, int i);
            virtual void slotContextMenu(KListView* list, Q3ListViewItem* item, const QPoint& p);
            virtual void slotMouseButtonPressed(int, Q3ListViewItem *, const QPoint &, int);
            
        private:
            class ArticleListViewPrivate;
            ArticleListViewPrivate* d;
            
            friend class ColumnLayoutVisitor;
            class ColumnLayoutVisitor;
            
            class ArticleItem;
    };
}

#endif
