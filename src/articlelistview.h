/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
                  2005-2007 Frank Osterfeld <frank.osterfeld@kdemail.net>

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
#ifndef AKREGATOR_ARTICLELISTVIEW_H
#define AKREGATOR_ARTICLELISTVIEW_H

#include <kdepim_export.h>

#include <QSortFilterProxyModel>
#include <QTreeView>

class QPaintEvent;
class QKeyEvent;
template <class T> class QList;

namespace Akregator {

class Article;
class TreeNode;

namespace Filters
{
    class ArticleMatcher;
}

class AKREGATOR_EXPORT SortColorizeProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:

    explicit SortColorizeProxyModel( QObject* parent = 0 );

    //reimpl
    int columnCount( const QModelIndex& index=QModelIndex() ) const;

    //reimpl
    QVariant data( const QModelIndex& index, int role=Qt::DisplayRole ) const;

    //reimpl
    QVariant headerData( int section, Qt::Orientation orientation, int role=Qt::DisplayRole ) const;

private:
    enum Column { ItemTitleColumn=0, FeedTitleColumn, DateColumn, ColumnCount };

    QIcon m_keepFlagIcon;
};


class AKREGATOR_EXPORT ArticleListView : public QTreeView
{
    Q_OBJECT

public:

    explicit ArticleListView( QWidget* parent = 0 );
    ~ArticleListView();

    Akregator::Article currentArticle() const;

    QList<Akregator::Article> selectedArticles() const;

public Q_SLOTS:

    void slotShowNode( Akregator::TreeNode* node );

    void slotClear();

    void slotPreviousArticle();

    void slotNextArticle();

    void slotPreviousUnreadArticle();

    void slotNextUnreadArticle();

signals:

    void signalArticleChosen( const Akregator::Article& );

    void signalDoubleClicked( const Akregator::Article& );

    void articleSelected( const QString& );
    void articleDoubleClicked( const QString& );

protected:

    //reimpl
    void paintEvent( QPaintEvent* e );

    //reimpl
    void keyPressEvent( QKeyEvent* e );

protected Q_SLOTS:

    //reimpl
    void currentChanged( const QModelIndex& current, const QModelIndex& previous );

private Q_SLOTS:

    void slotDoubleClicked( const QModelIndex& index );

private:

    void selectIndex( const QModelIndex& index );

    void setupHeader();

    void setFeedMode();
    void setGroupMode();

private:

    enum Column { ItemTitleColumn=0, FeedTitleColumn, DateColumn, ColumnCount };
    enum ColumnMode { Unspecified, GroupMode, FeedMode };
    ColumnMode m_columnMode;
};

#if 0
class AKREGATOR_EXPORT ArticleListViewOld : public K3ListView
{
    Q_OBJECT
    public:
        explicit ArticleListViewOld(QWidget *parent = 0, const char *name = 0);
        ~ArticleListViewOld();
        
        /** returns the current article, or a null article if there is none */
        Article currentArticle() const;
        
        /** returns a list of currently selected articles */
        QList<Article> selectedArticles() const;
        
        enum Columns { itemTitle, feedTitle, pubDate };

    public slots:

        /** show article list of tree node @c node (also connects to the notification signals of the node) */
        void slotShowNode(Akregator::TreeNode* node);
        
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

        void slotPaletteOrFontChanged();
        
    signals:
        void signalArticleChosen(const Akregator::Article& article);
        void signalDoubleClicked(const Akregator::Article&, const QPoint&, int);
        //void signalContextMenu(K3ListView*, ArticleItem*, const QPoint&);
        void signalMouseButtonPressed(int, const Akregator::Article&, const QPoint &, int);
            
    protected:
        /** reimplemented for kmail-like behaviour */            
        void keyPressEvent(QKeyEvent* e);
        
        /** applies text filter and status filter by setting visibility
        of items accordingly */
        void applyFilters();
        
        /**
        * @return count of visible articles, used for info boxes
        */
        int visibleArticles();
        
        void viewportPaintEvent(QPaintEvent *e);
        
        void connectToNode(TreeNode* node);
        void disconnectFromNode(TreeNode* node);
        
        Q3DragObject *dragObject();

    protected slots:

        void slotArticlesAdded(Akregator::TreeNode* node, const QList<Akregator::Article>& list);
        void slotArticlesUpdated(Akregator::TreeNode* node, const QList<Akregator::Article>& list);
        void slotArticlesRemoved(Akregator::TreeNode* node, const QList<Akregator::Article>& list);

        void slotCurrentChanged(Q3ListViewItem* item);
        void slotSelectionChanged();
        void slotDoubleClicked(Q3ListViewItem* item, const QPoint& p, int i);
        void slotContextMenu(K3ListView* list, Q3ListViewItem* item, const QPoint& p);
        void slotMouseButtonPressed(int, Q3ListViewItem *, const QPoint &, int);
        
    private:
        class ArticleListViewOldPrivate;
        ArticleListViewOldPrivate* d;
        
        friend class ColumnLayoutVisitor;
        class ColumnLayoutVisitor;
        
        class ArticleItem;
};

#endif // if 0

} // namespace Akregator


#endif // AKREGATOR_ARTICLELISTVIEW_H
