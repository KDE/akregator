/*
    This file is part of Akregator.

    Copyright (C) 2007 Frank Osterfeld <osterfeld@kde.org>

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

#include "selectioncontroller.h"

#include "actionmanager.h"
#include "article.h"
#include "articlemodel.h"
#include "feedlist.h"
#include "subscriptionlistmodel.h"
#include "treenode.h"

#include <KRandom>
#include <KDebug>

#include <QAbstractItemView>
#include <QMenu>
#include <QTimer>

using namespace Akregator;

namespace {
    static Akregator::Article articleForIndex( const QModelIndex& index, Akregator::FeedList* feedList )
    {
        if ( !index.isValid() )
            return Akregator::Article();

        const QString guid = index.data( Akregator::ArticleModel::GuidRole ).toString();
        const QString feedId = index.data( Akregator::ArticleModel::FeedIdRole ).toString();
        return feedList->findArticle( feedId, guid );
    }

    static QList<Akregator::Article> articlesForIndexes( const QModelIndexList& indexes, Akregator::FeedList* feedList )
    {
        QList<Akregator::Article> articles;
        Q_FOREACH ( const QModelIndex& i, indexes )
        {
            articles.append( articleForIndex( i, feedList ) );
        }

        return articles;
    }

    static Akregator::TreeNode* subscriptionForIndex( const QModelIndex& index, Akregator::FeedList* feedList )
    {
        if ( !index.isValid() )
            return 0L;

         return feedList->findByID( index.data( Akregator::SubscriptionListModel::SubscriptionIdRole ).toInt() );
    }
} // anon namespace

Akregator::SelectionController::SelectionController( QObject* parent )
    : AbstractSelectionController( parent ),
    m_feedList( 0 ),
    m_feedSelector( 0 ),
    m_articleLister( 0 ),
    m_singleDisplay( 0 ),
    m_subscriptionModel ( 0 ),
    m_folderExpansionHandler( 0 ),
    m_articleModel( 0 ),
    m_selectedSubscription( 0 )
{
    m_articleFetchTimer = new QTimer( this );
    connect( m_articleFetchTimer, SIGNAL( timeout() ),
             this, SLOT( articleHeadersAvailable() ) );

}


void Akregator::SelectionController::setFeedSelector( QAbstractItemView* feedSelector )
{
    if ( m_feedSelector )
    {
        m_feedSelector->disconnect( this );

        if ( m_feedSelector->selectionModel() )
            m_feedSelector->selectionModel()->disconnect( this );
    }
    m_feedSelector = feedSelector;
    setUp();
}

void Akregator::SelectionController::setArticleLister( Akregator::ArticleLister* lister )
{
    if ( m_articleLister )
        m_articleLister->articleSelectionModel()->disconnect( this );
    m_articleLister = lister;
    setUp();
}

void Akregator::SelectionController::setSingleArticleDisplay( Akregator::SingleArticleDisplay* display )
{
    m_singleDisplay = display;
}

Akregator::Article Akregator::SelectionController::currentArticle() const
{
    return ::articleForIndex( m_articleLister->articleSelectionModel()->currentIndex(), m_feedList );
}

QList<Akregator::Article> Akregator::SelectionController::selectedArticles() const
{
    return ::articlesForIndexes( m_articleLister->articleSelectionModel()->selectedRows(), m_feedList );
}

Akregator::TreeNode* Akregator::SelectionController::selectedSubscription() const
{
    return ::subscriptionForIndex( m_feedSelector->selectionModel()->currentIndex(), m_feedList );
}

void Akregator::SelectionController::setFeedList( Akregator::FeedList* list )
{
    m_feedList = list;
    setUp();
}

void Akregator::SelectionController::setFolderExpansionHandler( Akregator::FolderExpansionHandler* handler )
{
    m_folderExpansionHandler = handler;
    if ( !handler )
        return;
    handler->setFeedList( m_feedList );
    handler->setModel( m_subscriptionModel );
}

void Akregator::SelectionController::setUp()
{
    if ( !m_feedList || !m_feedSelector || !m_articleLister )
        return;

    m_subscriptionModel = new SubscriptionListModel( m_feedList, this );
    if ( m_folderExpansionHandler )
    {
        m_folderExpansionHandler->setFeedList( m_feedList );
        m_folderExpansionHandler->setModel( m_subscriptionModel );
    }
    m_feedSelector->setModel( m_subscriptionModel );

    // setUp might be called more than once, so disconnect first
    //connect exactly once:
    disconnect( m_feedSelector->selectionModel(), SIGNAL( currentChanged( QModelIndex, QModelIndex ) ),
             this, SLOT( selectedSubscriptionChanged( QModelIndex ) ) );
    connect( m_feedSelector->selectionModel(), SIGNAL( currentChanged( QModelIndex, QModelIndex ) ),
             this, SLOT( selectedSubscriptionChanged( QModelIndex ) ) );

    //connect exactly once:
    disconnect( m_feedSelector, SIGNAL( customContextMenuRequested( QPoint ) ),
             this, SLOT( subscriptionContextMenuRequested( QPoint ) ) );
    connect( m_feedSelector, SIGNAL( customContextMenuRequested( QPoint ) ),
             this, SLOT( subscriptionContextMenuRequested( QPoint ) ) );

    if ( m_articleLister->itemView() )
    {
        //connect exactly once:
        disconnect( m_articleLister->itemView(), SIGNAL( doubleClicked( QModelIndex ) ), this, SLOT( articleIndexDoubleClicked( QModelIndex ) )  );
        connect( m_articleLister->itemView(), SIGNAL( doubleClicked( QModelIndex ) ), this, SLOT( articleIndexDoubleClicked( QModelIndex ) )  );
    }
}

void Akregator::SelectionController::articleHeadersAvailable()
{
    if(m_articleModel)
        delete m_articleModel;

    m_articleModel = new Akregator::ArticleModel( m_selectedSubscription );
    m_articleLister->setArticleModel( m_articleModel );

    m_articleLister->setIsAggregation( m_selectedSubscription->isAggregation() );

    connect( m_articleLister->articleSelectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ),
             this, SLOT( articleSelectionChanged() ) );

    if ( m_selectedSubscription )
        m_articleLister->setScrollBarPositions( m_selectedSubscription->listViewScrollBarPositions() );
}


void Akregator::SelectionController::selectedSubscriptionChanged( const QModelIndex& index )
{
    if ( !index.isValid() )
        return;

    if ( m_selectedSubscription && m_articleLister )
        m_selectedSubscription->setListViewScrollBarPositions( m_articleLister->scrollBarPositions() );

    m_selectedSubscription = selectedSubscription();
    emit currentSubscriptionChanged( m_selectedSubscription );

    // using a timer here internally to simulate async data fetching (which is still synchronous),
    // to ensure the UI copes with async behavior later on

    if ( m_articleFetchTimer->isActive() )
        m_articleFetchTimer->stop(); // to come: kill running list job

    m_articleFetchTimer->setInterval( KRandom::random() % 400 );
    m_articleFetchTimer->setSingleShot( true );
    m_articleFetchTimer->start();

}

void Akregator::SelectionController::subscriptionContextMenuRequested( const QPoint& point )
{
    Q_ASSERT( m_feedSelector );
    const TreeNode* const node = ::subscriptionForIndex( m_feedSelector->indexAt( point ), m_feedList );
    if ( !node )
        return;

    QWidget* w = ActionManager::getInstance()->container( node->isGroup() ? "feedgroup_popup" : "feeds_popup" );
    QMenu* popup = qobject_cast<QMenu*>( w );
    if ( popup )
    {
        const QPoint globalPos = m_feedSelector->viewport()->mapToGlobal( point );
        popup->exec( globalPos );
    }
}

void Akregator::SelectionController::articleSelectionChanged()
{
    const Akregator::Article article = currentArticle();
    if ( m_singleDisplay )
        m_singleDisplay->showArticle( article );
    emit currentArticleChanged( article );
}

void Akregator::SelectionController::articleIndexDoubleClicked( const QModelIndex& index )
{
    const Akregator::Article article = ::articleForIndex( index, m_feedList );
    emit articleDoubleClicked( article );
}

void SelectionController::setFilters( const std::vector<boost::shared_ptr<const Filters::AbstractMatcher> >& matchers )
{
    Q_ASSERT( m_articleLister );
    m_articleLister->setFilters( matchers );
}

void SelectionController::forceFilterUpdate()
{
    Q_ASSERT( m_articleLister );
    m_articleLister->forceFilterUpdate();
}

#include "selectioncontroller.moc"
