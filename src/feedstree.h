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
            FeedsTreeItem( bool f, QListView *parent, QString label=QString::null);
            FeedsTreeItem( bool f, QListViewItem *parent, QString label=QString::null);
            FeedsTreeItem( bool f, QListView *parent, QListViewItem *after, QString
        label=QString::null);
            FeedsTreeItem(bool f, QListViewItem *parent, QListViewItem *after,
                       QString label=QString::null);
            ~FeedsTreeItem();
        virtual void paintCell( QPainter * p, const QColorGroup & cg,
                            int column, int width, int align );

        int unread(){return m_unread;}
        void setUnread(int u);

        bool isFolder();
        void setFolder(bool f);
        int countUnreadRecursive();

        private:
            void updateParentsRecursive();
            int m_unread;
            bool m_folder;
    };


    class FeedsTree : public KListView
    {
        Q_OBJECT
        public:
            FeedsTree( QWidget *parent = 0, const char *name = 0 );
            ~FeedsTree();

 
        protected:
            virtual void drawContentsOffset( QPainter * p, int ox, int oy,
                                       int cx, int cy, int cw, int ch );
            virtual void contentsDragMoveEvent(QDragMoveEvent* event);
            virtual bool acceptDrag(QDropEvent *event) const;
            virtual void movableDropEvent(QListViewItem* parent, QListViewItem* afterme);
            virtual void keyPressEvent(QKeyEvent* e);
            void takeNode(QListViewItem* item);
            void insertNode(QListViewItem* parent, QListViewItem* item, QListViewItem* after);

         public slots:
           
            /** handle dropped urls */
            void slotDropped(QDropEvent *e, QListViewItem *after);
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
            
            void slotMoveItemUp();
            void slotMoveItemDown();
            void slotMoveItemLeft();
            void slotMoveItemRight();
            
        signals:
            void dropped (KURL::List &, QListViewItem *, QListViewItem *);
    };

}

#endif
