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
#ifndef AKREGATORARTICLELIST_H
#define AKREGATORARTICLELIST_H

#include "articlefilter.h"
#include "myarticle.h"

#include <klistview.h>

class QKeyEvent;

namespace Akregator
{
    class Feed;
    class FeedGroup;
    class MyArticle;
    class TreeNode;
    
    class ArticleListItem : public KListViewItem
    {
        public:
            ArticleListItem( QListView *parent, QListViewItem *after, const MyArticle& a, Feed *parent );
            ~ArticleListItem();

            MyArticle& article();
            Feed *feed();
            void paintCell ( QPainter * p, const QColorGroup & cg, int column, int width, int align );
            virtual int compare(QListViewItem *i, int col, bool ascending) const;

            virtual ArticleListItem* itemAbove() { return static_cast<ArticleListItem*>(KListViewItem::itemAbove()); }
            
            virtual ArticleListItem* nextSibling() { return static_cast<ArticleListItem*>(KListViewItem::nextSibling()); }
            
        private:
            MyArticle m_article;
            Feed *m_feed;
    };
    
    
    class ArticleList : public KListView
    {
        Q_OBJECT
        public:
            ArticleList(QWidget *parent = 0, const char *name = 0);
            ~ArticleList();
            
            /**
            Listen to notification signals of the viewed node. Disable this if you don't want this view updated when the node changes.
            It's enabled by default.
            
            @param doReceive whether listen to notification signals or not
            @param remember if @c true: if an update request occured while receiving updates was disabled, the view is updated on re-enabling. You have to set this when reenabling, not when disabling! Ignored when @c doReceive is set to false.(Anyone got this?) */
            void setReceiveUpdates(bool doReceive, bool remember=true);

            ArticleListItem* currentItem() const { return static_cast<ArticleListItem*>(KListView::currentItem()); }
            
            ArticleListItem* selectedItem() const { return static_cast<ArticleListItem*>(KListView::selectedItem()); }
            
        public slots:

            /** show article list of tree node @c node (also connects to the notification signals of the node) */
            void slotShowNode(TreeNode* node);
            
            /** clears the list and disconnects from the observed node (if any) */
            void slotClear();
            
            /** Update view */
            void slotUpdate();
            
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
             /** emitted when an article was selected
                 @param article the article selected*/
            void signalArticleSelected(MyArticle article);
            void signalDoubleClicked(ArticleListItem*, const QPoint&, int);
            void signalContextMenu(KListView*, ArticleListItem*, const QPoint&);
             
        protected:
            /** reimplemented for kmail-like behaviour */            
            virtual void keyPressEvent(QKeyEvent* e);
            
            /** applies text filter and status filter by setting visibility
            of items accordingly */
            virtual void applyFilters();

        protected slots:
            virtual void slotSelectionChanged(QListViewItem* item);
            virtual void slotDoubleClicked(QListViewItem* item, const QPoint& p, int i);
            virtual void slotContextMenu(KListView* list, QListViewItem* item, const QPoint& p);
            
        private:
            bool m_updated;
            bool m_doReceive;
            TreeNode* m_node;
            ArticleFilter m_textFilter;
            ArticleFilter m_statusFilter;
            enum ColumnMode { groupMode, feedMode };
            ColumnMode m_columnMode;
            int m_feedWidth;
    };
}

#endif
