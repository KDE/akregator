/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
#ifndef AKREGATORFEEDSTREE_H
#define AKREGATORFEEDSTREE_H

#include <klistview.h>
#include <kurl.h>

namespace Akregator
{

    class FeedsTreeItem : public KListViewItem
    {
        public:
            FeedsTreeItem( QListView *parent, QString label=QString::null);
            FeedsTreeItem( QListViewItem *parent, QString label=QString::null);
            FeedsTreeItem( QListView *parent, QListViewItem *after, QString
        label=QString::null);
            FeedsTreeItem(QListViewItem *parent, QListViewItem *after,
                       QString label=QString::null);
        virtual void paintCell( QPainter * p, const QColorGroup & cg,
                            int column, int width, int align );
   
        int unread(){return m_unread;}
        void setUnread(int u);
        
        private:
            int countUnreadRecursive();
            int m_unread;
    };
              
    
    class FeedsTree : public KListView
    {
        Q_OBJECT
        public:
            FeedsTree( QWidget *parent = 0, const char *name = 0 );
            ~FeedsTree();

            void takeNode(QListViewItem* item);
            void insertNode(QListViewItem* parent, QListViewItem* item, QListViewItem* after);

        protected:
            virtual void drawContentsOffset( QPainter * p, int ox, int oy,
                                       int cx, int cy, int cw, int ch );
            virtual void contentsDragEnterEvent (QDragEnterEvent *e);
            virtual void contentsDropEvent( QDropEvent *e );
            virtual void contentsDragMoveEvent(QDragMoveEvent* event);

        public slots:
            /** Shows only first-level feeds */
            void slotCollapseAll();
            /** Shows only root of selected tree */
            void slotCollapse();
            /** Shows all feeds */
            void slotExpandAll();
            /** Shows all feeds of selected tree */
            void slotExpand();
            /** Move feed up */
            void slotItemUp();
            /** Move feed down */
            void slotItemDown();
            /** Move feed at the beginning of current list */
            void slotItemBegin();
            /** Move feed at the end of current list */
            void slotItemEnd();
            /** Move feed level up (to enclosing group) */
            void slotItemLeft();
            /** Move feed level down */
            void slotItemRight();
        signals:
            void dropped (KURL::List &, QListViewItem *, QListViewItem *);
    };

}

#endif
