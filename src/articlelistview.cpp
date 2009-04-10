/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
                  2005-2008 Frank Osterfeld <osterfeld@kde.org>
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

#include "articlelistview.h"
#include "actionmanager.h"
#include "akregatorconfig.h"
#include "article.h"
#include "articlemodel.h"
#include "kernel.h"
#include "types.h"

#include <utils/filtercolumnsproxymodel.h>

#include <KIcon>
#include <KLocale>
#include <KUrl>
#include <KMenu>

#include <QApplication>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QKeyEvent>
#include <QList>
#include <QMenu>
#include <QPaintEvent>
#include <QPalette>
#include <QScrollBar>

#include <cassert>

using namespace boost;
using namespace Akregator;


FilterDeletedProxyModel::FilterDeletedProxyModel( QObject* parent ) : QSortFilterProxyModel( parent )
{
    setDynamicSortFilter( true );
}

bool FilterDeletedProxyModel::filterAcceptsRow( int source_row, const QModelIndex& source_parent ) const
{
    return !sourceModel()->index( source_row, 0, source_parent ).data( ArticleModel::IsDeletedRole ).toBool();
}

SortColorizeProxyModel::SortColorizeProxyModel( QObject* parent ) : QSortFilterProxyModel( parent ), m_keepFlagIcon( KIcon( "mail-mark-important" ) )
{
}

bool SortColorizeProxyModel::filterAcceptsRow ( int source_row, const QModelIndex& source_parent ) const
{
    if ( source_parent.isValid() )
        return false;

    for ( uint i = 0; i < m_matchers.size(); ++i )
    {
        if ( !static_cast<ArticleModel*>( sourceModel() )->rowMatches( source_row, m_matchers[i] ) )
            return false;
    }

    return true;
}

void SortColorizeProxyModel::setFilters( const std::vector<shared_ptr<const Filters::AbstractMatcher> >&  matchers )
{
    if ( m_matchers == matchers )
        return;
    m_matchers = matchers;
    invalidateFilter();
}

QVariant SortColorizeProxyModel::data( const QModelIndex& idx, int role ) const
{
    if ( !idx.isValid() || !sourceModel() )
        return QVariant();

    const QModelIndex sourceIdx = mapToSource( idx );

    switch ( role )
    {
        case Qt::ForegroundRole:
        {
            switch ( static_cast<ArticleStatus>( sourceIdx.data( ArticleModel::StatusRole ).toInt() ) )
            {
                case Unread:
                {
                    return Settings::useCustomColors() ?
                        Settings::colorUnreadArticles() : Qt::blue;
                }
                case New:
                {
                    return Settings::useCustomColors() ?
                        Settings::colorNewArticles() : Qt::red;
                }
                case Read:
                {
                    return QApplication::palette().color( QPalette::WindowText );
                }
            }
        }
        break;
        case Qt::DecorationRole:
        {
            if ( sourceIdx.column() == ArticleModel::ItemTitleColumn )
            {
                return sourceIdx.data( ArticleModel::IsImportantRole ).toBool() ? m_keepFlagIcon : QVariant();
            }
        }
        break;
    }
    return sourceIdx.data( role );
}

namespace {

    static bool isRead( const QModelIndex& idx )
    {
        if ( !idx.isValid() )
            return false;

        return static_cast<ArticleStatus>( idx.data( ArticleModel::StatusRole ).toInt() ) == Read;
    }
}

void ArticleListView::setArticleModel( ArticleModel* model )
{
    if ( !model ) {
        setModel( model );
        return;
    }

    m_proxy = new SortColorizeProxyModel( model );
    m_proxy->setSourceModel( model );
    m_proxy->setSortRole( ArticleModel::SortRole );
    m_proxy->setFilters( m_matchers );
    FilterDeletedProxyModel* const proxy2 = new FilterDeletedProxyModel( model );
    proxy2->setSortRole( ArticleModel::SortRole );
    proxy2->setSourceModel( m_proxy );

    FilterColumnsProxyModel* const columnsProxy = new FilterColumnsProxyModel( model );
    columnsProxy->setSortRole( ArticleModel::SortRole );
    columnsProxy->setSourceModel( proxy2 );
    columnsProxy->setColumnEnabled( ArticleModel::ItemTitleColumn );
    columnsProxy->setColumnEnabled( ArticleModel::FeedTitleColumn );
    columnsProxy->setColumnEnabled( ArticleModel::DateColumn );
    columnsProxy->setColumnEnabled( ArticleModel::AuthorColumn );

    setModel( columnsProxy );
    header()->setContextMenuPolicy( Qt::CustomContextMenu );
}

void ArticleListView::showHeaderMenu(const QPoint& pos)
{
    if ( !model() )
        return;

    QPointer<KMenu> menu = new KMenu( this );
    menu->addTitle( i18n( "Columns" ) );
    menu->setAttribute( Qt::WA_DeleteOnClose );

    const int colCount = model()->columnCount();
    for ( int i = 0; i < colCount; ++i )
    {
        QAction* act = menu->addAction( model()->headerData( i, Qt::Horizontal ).toString() );
        act->setCheckable( true );
        act->setData( i );
        act->setChecked( !header()->isSectionHidden( i ) );
    }

    QPointer<QObject> that( this );
    QAction * const action = menu->exec( header()->mapToGlobal( pos ) );
    if ( that && action ) {
        const int col = action->data().toInt();
        if ( action->isChecked() )
            header()->showSection( col );
        else
            header()->hideSection( col );
    }
    delete menu;
}

void ArticleListView::saveHeaderSettings()
{
    if ( model() ) {
        const QByteArray state = header()->saveState();
        if ( m_columnMode == FeedMode )
            m_feedHeaderState = state;
        else
            m_groupHeaderState = state;
    }

    KConfigGroup conf( Settings::self()->config(), "General" );
    conf.writeEntry( "ArticleListFeedHeaders", m_feedHeaderState.toBase64() );
    conf.writeEntry( "ArticleListGroupHeaders", m_groupHeaderState.toBase64() );
}

void ArticleListView::loadHeaderSettings()
{
    KConfigGroup conf( Settings::self()->config(), "General" );
    m_feedHeaderState = QByteArray::fromBase64( conf.readEntry( "ArticleListFeedHeaders" ).toAscii() );
    m_groupHeaderState = QByteArray::fromBase64( conf.readEntry( "ArticleListGroupHeaders" ).toAscii() );
}

QItemSelectionModel* ArticleListView::articleSelectionModel() const
{
    return selectionModel();
}

const QAbstractItemView* ArticleListView::itemView() const
{
    return this;
}

QAbstractItemView* ArticleListView::itemView()
{
    return this;
}

QPoint ArticleListView::scrollBarPositions() const
{
    return QPoint( horizontalScrollBar()->value(), verticalScrollBar()->value() );
}

void ArticleListView::setScrollBarPositions( const QPoint& p )
{
    horizontalScrollBar()->setValue( p.x() );
    verticalScrollBar()->setValue( p.y() );
}


void ArticleListView::setGroupMode()
{
    if ( m_columnMode == GroupMode )
        return;

    // The next line (used three times in this file) is a workaround for a
    // possible Qt 4.4.3 bug that causes the last column to expand beyond
    // the viewport width.  QHeaderViewPrivate::lastSectionSize may not be
    // initialised when QHeaderViewPrivate::resizeSections() is called,
    // doing the resizeSection() here ensures that it has a sensible value.
    // This may not be necessary with Qt 4.5.
    header()->resizeSection( header()->count() - 1, 1 );

    if ( model() )
        m_feedHeaderState = header()->saveState();

    header()->restoreState( m_groupHeaderState );
    m_columnMode = GroupMode;
}

void ArticleListView::setFeedMode()
{
    if ( m_columnMode == FeedMode )
        return;

    header()->resizeSection( header()->count() - 1, 1 );
    if ( model() )
        m_groupHeaderState = header()->saveState();
    header()->restoreState( m_feedHeaderState );
    m_columnMode = FeedMode;
}

ArticleListView::~ArticleListView()
{
    saveHeaderSettings();
}

void ArticleListView::setIsAggregation( bool aggregation )
{
    if ( aggregation )
        setGroupMode();
    else
        setFeedMode();
}

ArticleListView::ArticleListView( QWidget* parent )
    : QTreeView(parent),
    m_columnMode( FeedMode )
{
    setSortingEnabled( true );
    setAlternatingRowColors( true );
    setSelectionMode( QAbstractItemView::ExtendedSelection );
    setUniformRowHeights( true );
    setRootIsDecorated( false );
    setAllColumnsShowFocus( true );
    setDragDropMode( QAbstractItemView::DragOnly );

    setMinimumSize( 250, 150 );
    setWhatsThis( i18n("<h2>Article list</h2>"
        "Here you can browse articles from the currently selected feed. "
        "You can also manage articles, as marking them as persistent (\"Keep Article\") or delete them, using the right mouse button menu."
        "To view the web page of the article, you can open the article internally in a tab or in an external browser window."));

    //connect exactly once
    disconnect( header(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showHeaderMenu(QPoint)) );
    connect( header(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showHeaderMenu(QPoint)) );
    loadHeaderSettings();
}

void ArticleListView::mousePressEvent( QMouseEvent *ev )
{
    // let's push the event, so we can use currentIndex() to get the newly selected article..
    QTreeView::mousePressEvent( ev );

    if( ev->button() == Qt::MidButton ) {
        QModelIndex idx( currentIndex() );
        const KUrl url = currentIndex().data( ArticleModel::LinkRole ).value<KUrl>();

        emit signalMouseButtonPressed( ev->button(), url );
    }
}


#if 0 // unused
namespace {
    static QString itemIdForIndex( const QModelIndex& index )
    {
        return index.isValid() ? index.data( ArticleModel::ItemIdRole ).toString() : QString();
    }

    static QStringList itemIdsForIndexes( const QModelIndexList& indexes )
    {
        QStringList articles;
        Q_FOREACH ( const QModelIndex i, indexes )
        {
            articles.append( itemIdForIndex( i ) );
        }

        return articles;
    }
}
#endif

void ArticleListView::contextMenuEvent( QContextMenuEvent* event )
{
    QWidget* w = ActionManager::getInstance()->container( "article_popup" );
    QMenu* popup = qobject_cast<QMenu*>( w );
    if ( popup )
        popup->exec( event->globalPos() );
}

void ArticleListView::paintEvent( QPaintEvent* e )
{
    QTreeView::paintEvent( e );

#ifdef __GNUC__
#warning The distinction between empty node and 0 items after filtering is hard to port to interview
#endif
#if 0
    QString message;

    if ( !model() || model()->rowCount() > 0 ) // article list is not empty
    {
        if (visibleArticles() == 0)
        {
            message = i18n("<div align=center>"
                            "<h3>No matches</h3>"
                            "Filter does not match any articles, "
                            "please change your criteria and try again."
                            "</div>");
        }
    }
    else if ( !model() ) // article list is empty
    {
        if (!d->node) // no node selected
        {
            message = i18n("<div align=center>"
                       "<h3>No feed selected</h3>"
                       "This area is article list. "
                       "Select a feed from the feed list "
                       "and you will see its articles here."
                       "</div>");
        }
    }

    if (!message.isNull())
        paintInfoBox( message, viewport(), palette() );
#endif
}


void ArticleListView::setModel( QAbstractItemModel* m )
{
    const bool groupMode = m_columnMode == GroupMode;

    QAbstractItemModel* const oldModel = model();
    if ( oldModel ) {
        const QByteArray state = header()->saveState();
        if ( groupMode )
            m_groupHeaderState = state;
        else
            m_feedHeaderState = state;
    }

    QTreeView::setModel( m );

    if ( m )
    {
        header()->resizeSection( header()->count() - 1, 1 );
        header()->restoreState( groupMode ? m_groupHeaderState : m_feedHeaderState );
    }
}

void ArticleListView::slotClear()
{
    setModel( 0L );
}

void ArticleListView::slotPreviousArticle()
{
    if ( !model() )
        return;

    const QModelIndex idx = currentIndex();
    const int newRow = qMax( 0, ( idx.isValid() ? idx.row() : model()->rowCount() ) - 1 );
    const QModelIndex newIdx = idx.isValid() ? idx.sibling( newRow, 0 ) : model()->index( newRow, 0 );
    selectIndex( newIdx );
}

void ArticleListView::slotNextArticle()
{
    if ( !model() )
        return;

    const QModelIndex idx = currentIndex();
    const int newRow = idx.isValid() ? ( idx.row() + 1 ) : 0;
    const QModelIndex newIdx = model()->index( qMin( newRow, model()->rowCount() - 1 ), 0 );
    selectIndex( newIdx );
}

void ArticleListView::slotNextUnreadArticle()
{
    if (!model())
        return;

    const int rowCount = model()->rowCount();
    const int startRow = qMin( rowCount - 1, ( currentIndex().isValid() ? currentIndex().row() + 1 : 0 ) );

    int i = startRow;
    bool foundUnread = false;

    do
    {
        if ( !::isRead( model()->index( i, 0 ) ) )
            foundUnread = true;
        else
            i = (i + 1) % rowCount;
    }
    while ( !foundUnread && i != startRow );

    if ( foundUnread )
    {
        selectIndex( model()->index( i, 0 ) );
    }
}

void ArticleListView::selectIndex( const QModelIndex& idx )
{
    if ( !idx.isValid() )
        return;
    setCurrentIndex( idx );
    clearSelection();
    Q_ASSERT( selectionModel() );
    selectionModel()->select( idx, QItemSelectionModel::Select | QItemSelectionModel::Rows );
    scrollTo( idx, PositionAtCenter );
}

void ArticleListView::slotPreviousUnreadArticle()
{
    if ( !model() )
        return;

    const int rowCount = model()->rowCount();
    const int startRow = qMax( 0, ( currentIndex().isValid() ? currentIndex().row() : rowCount ) - 1 );

    int i = startRow;
    bool foundUnread = false;

    do
    {
        if ( !::isRead( model()->index( i, 0 ) ) )
            foundUnread = true;
        else
            i = i > 0 ? i - 1 : rowCount - 1;
    }
    while ( !foundUnread && i != startRow );

    if ( foundUnread )
    {
        selectIndex( model()->index( i, 0 ) );
    }
}


void ArticleListView::forceFilterUpdate()
{
    if ( m_proxy )
        m_proxy->invalidate();
}

void ArticleListView::setFilters( const std::vector<shared_ptr<const Filters::AbstractMatcher> >& matchers )
{
    if ( m_matchers == matchers )
        return;
    m_matchers = matchers;
    if ( m_proxy )
        m_proxy->setFilters( matchers );
}

#include "articlelistview.moc"
