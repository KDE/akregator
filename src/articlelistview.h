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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef AKREGATORARTICLELISTVIEW_H
#define AKREGATORARTICLELISTVIEW_H

#include <ctime>

#include <qpixmap.h>

#include "articlefilter.h"
#include "article.h"

#include <klistview.h>

class QKeyEvent;
class QDragObject;
template <class T> class QValueList;

namespace Akregator
{
    class Feed;
    class Folder;
    class Article;
    class TreeNode;
    
    class ArticleItem : public KListViewItem
    {
        friend class ArticleListView;

        public:
            ArticleItem( QListView *parent, const Article& a);
            ~ArticleItem();

            Article& article();

            void paintCell ( QPainter * p, const QColorGroup & cg, int column, int width, int align );
            virtual int compare(QListViewItem *i, int col, bool ascending) const;

            virtual ArticleItem* itemAbove() { return static_cast<ArticleItem*>(KListViewItem::itemAbove()); }
            
            virtual ArticleItem* nextSibling() { return static_cast<ArticleItem*>(KListViewItem::nextSibling()); }

        private:
            Article m_article;
            time_t m_pubDate;
            static QPixmap m_keepFlag;
    };
    
    
    class ArticleListView : public KListView
    {
        Q_OBJECT
        public:
            ArticleListView(QWidget *parent = 0, const char *name = 0);
            ~ArticleListView();
            
            /**
            Listen to notification signals of the viewed node. Disable this if you don't want this view updated when the node changes.
            It's enabled by default.
            
            @param doReceive whether listen to notification signals or not
            @param remember if @c true: if an update request occurred while receiving updates was disabled, the view is updated on re-enabling. You have to set this when reenabling, not when disabling! Ignored when @c doReceive is set to false.(Anyone got this?) */
            void setReceiveUpdates(bool doReceive, bool remember=true);

            ArticleItem* currentArticleItem() const { return dynamic_cast<ArticleItem*>(KListView::currentItem()); }
            
            QValueList<ArticleItem*> selectedArticleItems(bool includeHiddenItems) const;
            
            enum Columns { itemTitle, feedTitle, pubDate };

        public slots:

            /** show article list of tree node @c node (also connects to the notification signals of the node) */
            void slotShowNode(TreeNode* node);
            
            /** clears the list and disconnects from the observed node (if any) */
            void slotClear();
            
            void slotArticlesAdded(TreeNode* node, const QValueList<Article>& list);
            void slotArticlesUpdated(TreeNode* node, const QValueList<Article>& list);
            void slotArticlesRemoved(TreeNode* node, const QValueList<Article>& list);
            /** sets text filter and status filter
            @param textFilter filters text
            @param statusFilter filters status (read, unread, new) */
            void slotSetFilter(const ArticleFilter& textFilter, const ArticleFilter& statusFilter);

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
            void signalDoubleClicked(ArticleItem*, const QPoint&, int);
            void signalContextMenu(KListView*, ArticleItem*, const QPoint&);
             
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
            
            virtual QDragObject *dragObject();

        protected slots:
            virtual void slotSelectionChanged();
            virtual void slotCurrentChanged(QListViewItem* item);
            virtual void slotDoubleClicked(QListViewItem* item, const QPoint& p, int i);
            virtual void slotContextMenu(KListView* list, QListViewItem* item, const QPoint& p);
            
        private:
            /** maps article to article item */
            QMap<Article, ArticleItem*> m_articleMap;
            bool m_updated;
            bool m_doReceive;
            TreeNode* m_node;
            ArticleFilter m_textFilter;
            ArticleFilter m_statusFilter;
            enum ColumnMode { groupMode, feedMode };
            ColumnMode m_columnMode;
            int m_feedWidth;
            friend class ColumnLayoutVisitor;
            class ColumnLayoutVisitor;
            ColumnLayoutVisitor* m_columnLayoutVisitor;
    };
}

#endif
