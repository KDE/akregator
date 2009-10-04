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
#ifndef AKREGATOR_SELECTIONCONTROLLER_H
#define AKREGATOR_SELECTIONCONTROLLER_H

#include "abstractselectioncontroller.h"

#include <QtCore/QMap>
#include <QtCore/QPointer>

#include <boost/weak_ptr.hpp>

class QModelIndex;
class QPoint;

class KJob;

namespace KRss {
    class FeedList;
    class FeedListModel;
    class ItemListing;
    class ItemListJob;
    class TreeNode;
}

namespace Akregator
{

class SelectionController : public AbstractSelectionController
{
    Q_OBJECT

public:

    explicit SelectionController( QObject* parent = 0 );

    //impl
    void setFeedSelector( QAbstractItemView* feedSelector ) ;

    //impl
    void setArticleLister( Akregator::ArticleLister* lister );

    //impl
    KRss::Item currentItem() const;

    //impl
    QList<KRss::Item> selectedItems() const;

    //impl
    void setSingleArticleDisplay( Akregator::SingleArticleDisplay* display );

    //impl
    boost::shared_ptr<KRss::TreeNode> selectedSubscription() const;

    //impl
    void setFeedList( const boost::shared_ptr<KRss::FeedList>& feedList );

    //impl
    void setTagProvider( const boost::shared_ptr<const KRss::TagProvider>& tagProvider );

    //impl
    void setFolderExpansionHandler( Akregator::FolderExpansionHandler* handler );

public Q_SLOTS:

    //impl
    void setFilters( const std::vector<boost::shared_ptr<const Akregator::Filters::AbstractMatcher> >& );

    //impl
    void forceFilterUpdate();

private Q_SLOTS:

    void selectedSubscriptionChanged( const QModelIndex& index );
    void itemSelectionChanged();
    void fullItemFetched( KJob* );
    void itemIndexDoubleClicked( const QModelIndex& index );
    void subscriptionContextMenuRequested( const QPoint& point );
    void articleHeadersAvailable(KJob*);

private:

    boost::shared_ptr<KRss::FeedList> m_feedList;
    boost::shared_ptr<const KRss::TagProvider> m_tagProvider;
    QPointer<QAbstractItemView> m_feedSelector;
    Akregator::ArticleLister* m_articleLister;
    Akregator::SingleArticleDisplay* m_singleDisplay;
    KRss::FeedListModel* m_feedListModel;
    boost::shared_ptr<KRss::ItemListing> m_itemListing;
    Akregator::FolderExpansionHandler* m_folderExpansionHandler;
    KRss::ItemModel* m_itemModel;
    boost::shared_ptr<KRss::TreeNode> m_selectedSubscription;
    QMap<boost::weak_ptr<const KRss::TreeNode>, QPoint> m_scrollBarPositions;
    QPointer<KRss::ItemListJob> m_listJob;
};

} // namespace Akregator

#endif // AKREGATOR_SELECTIONCONTROLLER_H
