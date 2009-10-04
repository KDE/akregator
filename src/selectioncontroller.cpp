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
#include "progressmanager.h"

#include <krss/feedlistmodel.h>
#include <krss/feedlist.h>
#include <krss/itemjobs.h>
#include <krss/itemlisting.h>
#include <krss/itemlistjob.h>
#include <krss/itemmodel.h>
#include <krss/tagprovider.h>
#include <krss/treenode.h>
#include <krss/treenodevisitor.h>

#include <Akonadi/ItemFetchScope>
#include <KDebug>

#include <QAbstractItemView>
#include <QMenu>

#include <cassert>

using namespace boost;
using namespace Akregator;
using KRss::FeedListModel;

namespace {
    static KRss::Item itemForIndex( const QModelIndex& index )
    {
        return index.data( KRss::ItemModel::ItemRole ).value<KRss::Item>();
    }

    static QList<KRss::Item> itemsForIndexes( const QModelIndexList& indexes )
    {
        QList<KRss::Item> items;
        Q_FOREACH ( const QModelIndex& i, indexes )
            items.append( itemForIndex( i ) );

        return items;
    }

    static shared_ptr<KRss::TreeNode> subscriptionForIndex( const QModelIndex& index )
    {
        return index.data( FeedListModel::TreeNodeRole ).value<shared_ptr<KRss::TreeNode> >();
    }
} // anon namespace

Akregator::SelectionController::SelectionController( QObject* parent )
    : AbstractSelectionController( parent ),
    m_feedList(),
    m_feedSelector(),
    m_articleLister( 0 ),
    m_singleDisplay( 0 ),
    m_feedListModel ( 0 ),
    m_folderExpansionHandler( 0 ),
    m_itemModel( 0 ),
    m_selectedSubscription()
{
}


void Akregator::SelectionController::setFeedSelector( QAbstractItemView* feedSelector )
{
    if ( m_feedSelector == feedSelector )
        return;

    if ( m_feedSelector ) {
        m_feedSelector->disconnect( this );
        m_feedSelector->selectionModel()->disconnect( this );
    }

    m_feedSelector = feedSelector;

    if ( !m_feedSelector )
        return;

    m_feedSelector->setModel( m_feedListModel );

    connect( m_feedSelector, SIGNAL(customContextMenuRequested(QPoint)),
             this, SLOT(subscriptionContextMenuRequested(QPoint)) );
    connect( m_feedSelector->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
             this, SLOT(selectedSubscriptionChanged(QModelIndex)) );

}

void Akregator::SelectionController::setArticleLister( Akregator::ArticleLister* lister )
{
    if ( m_articleLister == lister )
        return;

    if ( m_articleLister )
        m_articleLister->articleSelectionModel()->disconnect( this );
    if ( m_articleLister && m_articleLister->itemView() )
        m_articleLister->itemView()->disconnect( this );

    m_articleLister = lister;

    if ( m_articleLister && m_articleLister->itemView() )
        connect( m_articleLister->itemView(), SIGNAL(doubleClicked(QModelIndex)),
                 this, SLOT(itemIndexDoubleClicked(QModelIndex))  );
}

void Akregator::SelectionController::setSingleArticleDisplay( Akregator::SingleArticleDisplay* display )
{
    m_singleDisplay = display;
}

KRss::Item Akregator::SelectionController::currentItem() const
{
    return ::itemForIndex( m_articleLister->articleSelectionModel()->currentIndex() );
}

QList<KRss::Item> Akregator::SelectionController::selectedItems() const
{
    return ::itemsForIndexes( m_articleLister->articleSelectionModel()->selectedRows() );
}

shared_ptr<KRss::TreeNode> Akregator::SelectionController::selectedSubscription() const
{
    return ::subscriptionForIndex( m_feedSelector->selectionModel()->currentIndex() );
}

void Akregator::SelectionController::setFeedList( const shared_ptr<KRss::FeedList>& feedList )
{
    if ( m_feedList == feedList )
        return;

    m_feedList = feedList;

    if ( !m_tagProvider )
        return;

    std::auto_ptr<KRss::FeedListModel> oldModel( m_feedListModel );
    m_feedListModel = new KRss::FeedListModel( m_feedList, m_tagProvider, this );

#ifdef KRSS_PORT_DISABLED
    if ( m_folderExpansionHandler ) {
        m_folderExpansionHandler->setFeedList( m_feedList );
        m_folderExpansionHandler->setModel( m_subscriptionModel );
    }
#endif

    if ( m_feedSelector ) {
        m_feedSelector->setModel( m_feedListModel );
        disconnect( m_feedSelector->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                    this, SLOT(selectedSubscriptionChanged(QModelIndex)) );
        connect( m_feedSelector->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                 this, SLOT(selectedSubscriptionChanged(QModelIndex)) );
    }
}

void Akregator::SelectionController::setTagProvider( const shared_ptr<const KRss::TagProvider>& tagProvider )
{
    if ( m_tagProvider == tagProvider )
        return;

    m_tagProvider = tagProvider;

    if ( !m_feedList )
        return;

    std::auto_ptr<KRss::FeedListModel> oldModel( m_feedListModel );
    m_feedListModel = new KRss::FeedListModel( m_feedList, m_tagProvider, this );

#ifdef KRSS_PORT_DISABLED
    if ( m_folderExpansionHandler ) {
        m_folderExpansionHandler->setFeedList( m_feedList );
        m_folderExpansionHandler->setModel( m_subscriptionModel );
    }
#endif

    if ( m_feedSelector ) {
        m_feedSelector->setModel( m_feedListModel );
        disconnect( m_feedSelector->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                    this, SLOT(selectedSubscriptionChanged(QModelIndex)) );
        connect( m_feedSelector->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                 this, SLOT(selectedSubscriptionChanged(QModelIndex)) );
    }
}

void Akregator::SelectionController::setFolderExpansionHandler( Akregator::FolderExpansionHandler* handler )
{
#ifdef KRSS_PORT_DISABLED
    if ( handler == m_folderExpansionHandler )
        return;
    m_folderExpansionHandler = handler;
    if ( !m_folderExpansionHandler )
        return;
    handler->setFeedList( m_feedList );
    handler->setModel( m_subscriptionModel );
#endif
}

void Akregator::SelectionController::articleHeadersAvailable( KJob* job )
{
    assert( job );
    assert( job == m_listJob );


    if ( job->error() ) {
        kWarning() << job->errorText();
        return;
    }

    m_itemListing.reset( new KRss::ItemListing( m_listJob->items(), m_listJob->fetchScope() ) );
    KRss::ConnectToItemListingVisitor visitor ( m_feedList, m_itemListing );
    selectedSubscription()->accept( &visitor );
    KRss::ItemModel* const newModel = new KRss::ItemModel( m_itemListing, this );

    m_articleLister->setIsAggregation( selectedSubscription()->tier() == KRss::TreeNode::TagTier );
    m_articleLister->setItemModel( newModel );
    delete m_itemModel; //order is important: do not delete the old model before the new model is set in the view
    m_itemModel = newModel;

    disconnect( m_articleLister->articleSelectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
                this, SLOT(itemSelectionChanged()) );
    connect( m_articleLister->articleSelectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
             this, SLOT(itemSelectionChanged()) );

    m_articleLister->setScrollBarPositions( m_scrollBarPositions.value( m_selectedSubscription ) );
}


void Akregator::SelectionController::selectedSubscriptionChanged( const QModelIndex& index )
{
    if ( !index.isValid() )
        return;

    if ( m_selectedSubscription && m_articleLister )
        m_scrollBarPositions.insert( m_selectedSubscription, m_articleLister->scrollBarPositions() );

    m_selectedSubscription = selectedSubscription();
    emit currentSubscriptionChanged( m_selectedSubscription );

    if ( m_listJob ) {
        m_listJob->disconnect( this );
        m_listJob->kill();
        m_listJob = 0;
    }

    if ( !m_selectedSubscription )
        return;

    Akonadi::ItemFetchScope scope;
    scope.fetchPayloadPart( KRss::Item::HeadersPart );
    scope.fetchAllAttributes();
    KRss::ItemListJob* const job = m_selectedSubscription->createItemListJob( m_feedList );
    if ( !job )
        return;
    assert( job->capabilities().testFlag( KJob::Killable ) );
    job->setFetchScope( scope );
    connect( job, SIGNAL(finished(KJob*)),
             this, SLOT(articleHeadersAvailable(KJob*)) );
    m_listJob = job;
    ProgressManager::self()->addJob( job );
    m_listJob->start();
}

void Akregator::SelectionController::subscriptionContextMenuRequested( const QPoint& point )
{
    Q_ASSERT( m_feedSelector );
    const shared_ptr<const KRss::TreeNode> treeNode = ::subscriptionForIndex( m_feedSelector->indexAt( point ) );
    if ( !treeNode )
        return;

    QWidget* w = ActionManager::getInstance()->container( treeNode->tier() == KRss::TreeNode::TagTier ?
                                                          "feedgroup_popup" : "feeds_popup" );
    QMenu* popup = qobject_cast<QMenu*>( w );
    if ( popup ) {
        const QPoint globalPos = m_feedSelector->viewport()->mapToGlobal( point );
        popup->exec( globalPos );
    }
}

void Akregator::SelectionController::itemSelectionChanged()
{
    const KRss::Item item = currentItem();
    Akonadi::ItemFetchScope scope;
    scope.fetchPayloadPart( KRss::Item::HeadersPart );
    scope.fetchPayloadPart( KRss::Item::ContentPart );
    KRss::ItemFetchJob* job = new KRss::ItemFetchJob( this );
    job->setFetchScope( scope );
    job->setItem( item );
    connect( job, SIGNAL(finished(KJob*)), this, SLOT(fullItemFetched(KJob*)) );
    job->start();
}

void Akregator::SelectionController::fullItemFetched( KJob* j )
{
    KRss::ItemFetchJob* job = qobject_cast<KRss::ItemFetchJob*>( j );
    assert( job );
    if ( job->error() ) {
        //PENDING(frank) TODO handle error
    }

    const KRss::Item item = job->item();

    if ( m_singleDisplay )
        m_singleDisplay->showItem( item );

    emit currentItemChanged( item );
}

void Akregator::SelectionController::itemIndexDoubleClicked( const QModelIndex& index )
{
    const KRss::Item item = ::itemForIndex( index );
    emit itemDoubleClicked( item );
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
