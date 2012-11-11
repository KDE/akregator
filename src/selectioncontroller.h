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

#include <QPointer>

class QModelIndex;
class QPoint;
class QTimer;

class KJob;

namespace Akregator {

class ArticleListJob;

class SelectionController : public AbstractSelectionController
{
    Q_OBJECT

public:

    explicit SelectionController( QObject* parent = 0 );
    ~SelectionController();

    //impl
    void setFeedSelector( QAbstractItemView* feedSelector ) ;

    //impl
    void setArticleLister( Akregator::ArticleLister* lister );

    //impl
    Akregator::Article currentArticle() const;

    //impl
    QModelIndex currentArticleIndex() const;

    //impl
    QList<Akregator::Article> selectedArticles() const;

    //impl
    void setSingleArticleDisplay( Akregator::SingleArticleDisplay* display );

    //impl
    Akregator::TreeNode* selectedSubscription() const;

    //impl
    void setFeedList( const boost::shared_ptr<FeedList>& list );

    //impl
    void setFolderExpansionHandler( Akregator::FolderExpansionHandler* handler );

public Q_SLOTS:

    //impl
    void setFilters( const std::vector<boost::shared_ptr<const Akregator::Filters::AbstractMatcher> >& );

    //impl
    void forceFilterUpdate();

private Q_SLOTS:

    void selectedSubscriptionChanged( const QModelIndex& index );
    void articleSelectionChanged();
    void articleIndexDoubleClicked( const QModelIndex& index );
    void subscriptionContextMenuRequested( const QPoint& point );
    void articleHeadersAvailable(KJob*);

private:

    boost::shared_ptr<FeedList> m_feedList;
    QPointer<QAbstractItemView> m_feedSelector;
    Akregator::ArticleLister* m_articleLister;
    Akregator::SingleArticleDisplay* m_singleDisplay;
    Akregator::SubscriptionListModel* m_subscriptionModel;
    Akregator::FolderExpansionHandler* m_folderExpansionHandler;
    Akregator::ArticleModel* m_articleModel;
    QPointer<TreeNode> m_selectedSubscription;
    QPointer<ArticleListJob> m_listJob;
};

} // namespace Akregator

#endif // AKREGATOR_SELECTIONCONTROLLER_H
