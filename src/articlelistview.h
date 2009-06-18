/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
                  2005-2007 Frank Osterfeld <osterfeld@kde.org>

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

#include "akregator_export.h"
#include "abstractselectioncontroller.h"

#include <QPointer>
#include <QSortFilterProxyModel>
#include <QTreeView>

#include <boost/shared_ptr.hpp>

class KUrl;
class KMenu;

class QContextMenuEvent;
class QPaintEvent;
class QKeyEvent;
template <class T> class QList;

namespace Akregator {

class FilterColumnsProxyModel;

namespace Filters
{
    class ArticleMatcher;
}

class AKREGATORPART_EXPORT FilterDeletedProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:

    explicit FilterDeletedProxyModel( QObject* parent = 0 );

private:
    //reimpl
    bool filterAcceptsRow( int source_row, const QModelIndex& source_parent ) const;
};

class AKREGATORPART_EXPORT SortColorizeProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:

    explicit SortColorizeProxyModel( QObject* parent = 0 );

    //reimpl
    QVariant data( const QModelIndex& index, int role=Qt::DisplayRole ) const;

    void setFilters( const std::vector<boost::shared_ptr<const Akregator::Filters::AbstractMatcher> >& );

private:

    //reimpl
    bool filterAcceptsRow( int source_row, const QModelIndex& source_parent ) const;

    QIcon m_keepFlagIcon;
    std::vector<boost::shared_ptr<const Akregator::Filters::AbstractMatcher> > m_matchers;
};


class AKREGATORPART_EXPORT ArticleListView : public QTreeView, public ArticleLister
{
    Q_OBJECT

public:

    explicit ArticleListView( QWidget* parent = 0 );
    ~ArticleListView();

    //impl ArticleLister
    void setItemModel( KRss::ItemModel* model );

    //impl ArticleLister
    QItemSelectionModel* articleSelectionModel() const;

    //impl ArticleLister
    const QAbstractItemView* itemView() const;

    //impl ArticleLister
    QAbstractItemView* itemView();

    //impl ArticleLister
    QPoint scrollBarPositions() const;

    //impl ArticleLister
    void setScrollBarPositions( const QPoint& p );

    //impl ArticleLister
    void setFilters( const std::vector<boost::shared_ptr<const Akregator::Filters::AbstractMatcher> >& );

    //impl ArticleLister
    void forceFilterUpdate();

    void setIsAggregation( bool isAggregation );

    /* reimp */ void setModel( QAbstractItemModel* model );

protected:
    void mousePressEvent( QMouseEvent *ev );

signals:
    void signalMouseButtonPressed( int, const KUrl );

public Q_SLOTS:

    void slotClear();

    void slotPreviousArticle();

    void slotNextArticle();

    void slotPreviousUnreadArticle();

    void slotNextUnreadArticle();

private:
    void saveHeaderSettings();
    void loadHeaderSettings();

    //reimpl
    void paintEvent( QPaintEvent* e );

    //reimpl
    void contextMenuEvent( QContextMenuEvent* event );

    void selectIndex( const QModelIndex& index );

    void setFeedMode();
    void setGroupMode();

private Q_SLOTS:
    void showHeaderMenu( const QPoint& pos );

private:

    enum ColumnMode { GroupMode, FeedMode };
    ColumnMode m_columnMode;
    QPointer<SortColorizeProxyModel> m_proxy;
    std::vector<boost::shared_ptr<const Filters::AbstractMatcher> > m_matchers;
    QByteArray m_feedHeaderState;
    QByteArray m_groupHeaderState;
};

} // namespace Akregator


#endif // AKREGATOR_ARTICLELISTVIEW_H
