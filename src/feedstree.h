/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
#ifndef AKREGATORFEEDSTREE_H
#define AKREGATORFEEDSTREE_H

#include <klistview.h>

namespace Akregator
{
    class FeedsTree : public KListView
    {
        Q_OBJECT
        public:
            FeedsTree( QWidget *parent = 0, const char *name = 0 );
            ~FeedsTree();

            void takeNode(QListViewItem* item);
            void insertNode(QListViewItem* parent, QListViewItem* item, QListViewItem* after);

        protected:
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
    };

}

#endif
