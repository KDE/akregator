/*
    This file is part of Akregator.

    Copyright (C) 2007 Frank Osterfeld <frank.osterfeld@kdemail.net>

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
#include "articlemodel.h"

#include "article.h"
#include "articlematcher.h"
#include "akregatorconfig.h"
#include "treenode.h"
#include "feed.h"

#include <syndication/tools.h>

#include <QString>
#include <QVector>

#include <KLocale>

#include <cassert>
#include <cmath>

using namespace Akregator;

class ArticleModel::Private {
private:
    ArticleModel* const q;
public:
    Private( TreeNode* node_, ArticleModel* qq );
    Akregator::TreeNode* node;
    QList<Akregator::Article> articles;
    QVector<QString> titleCache;
    
    void nodeDestroyed();
    void articlesAdded( TreeNode*, const QList<Article>& );
    void articlesRemoved( TreeNode*, const QList<Article>& );
    void articlesUpdated( TreeNode*, const QList<Article>& );

};

ArticleModel::Private::Private( TreeNode* node_, ArticleModel* qq )
 : q( qq ), node( node_ )
{
    Q_ASSERT( node );
    articles = node->articles();
    titleCache.resize( articles.count() );
    for ( int i = 0; i < articles.count(); ++i )
        titleCache[i] = Syndication::htmlToPlainText( articles[i].title() );
    connect( node, SIGNAL( destroyed() ) , q, SLOT( nodeDestroyed() ) );
    connect( node, SIGNAL( signalArticlesAdded( Akregator::TreeNode*, QList<Akregator::Article> ) ), 
                          q, SLOT( articlesAdded( Akregator::TreeNode*, QList<Akregator::Article> ) ) );
    connect( node, SIGNAL( signalArticlesRemoved( Akregator::TreeNode*, QList<Akregator::Article> ) ), 
                           q, SLOT( articlesRemoved( Akregator::TreeNode*, QList<Akregator::Article> ) ) );
    connect( node, SIGNAL( signalArticlesUpdated( Akregator::TreeNode*, QList<Akregator::Article> ) ),
                           q, SLOT( articlesUpdated( Akregator::TreeNode*, QList<Akregator::Article> ) ) );

}

Akregator::ArticleModel::ArticleModel(TreeNode* node, QObject* parent) : QAbstractTableModel( parent ), d( new Private( node, this ) )
{
}

Akregator::ArticleModel::~ArticleModel()
{
    delete d;
}

int Akregator::ArticleModel::columnCount( const QModelIndex& parent ) const
{
    return parent.isValid() ? 0 : ColumnCount;
}

int Akregator::ArticleModel::rowCount( const QModelIndex& parent ) const
{
    return parent.isValid() ? 0 : d->articles.count();
}

QVariant Akregator::ArticleModel::headerData( int section, Qt::Orientation, int role ) const
{
    if ( role != Qt::DisplayRole )
        return QVariant();

    switch (section)
    {
        case ItemTitleColumn:
            return i18n("Title");
        case FeedTitleColumn:
            return i18n("Feed");
        case DateColumn:
            return i18n("Date");
        case AuthorColumn:
            return i18n("Author");
        case DescriptionColumn:
            return i18n("Description");
        case ContentColumn:
            return i18n("Content");
    }

    return QVariant();
}

QVariant Akregator::ArticleModel::data( const QModelIndex& index, int role ) const
{
    if ( !index.isValid() || index.row() < 0 || index.row() >= d->articles.count() )
        return QVariant();
    const int row = index.row();
    const Akregator::Article& article( d->articles[row] );

    if ( article.isNull() )
        return QVariant();

    switch ( role )
    {
        case Qt::DisplayRole:
        {
            switch ( index.column() )
            {
                case FeedTitleColumn:
                    return article.feed() ? article.feed()->title() : QVariant();
                case DateColumn:
                    return article.pubDate();
                case ItemTitleColumn:
                    return d->titleCache[row];
                case AuthorColumn:
                    return article.author();
                case DescriptionColumn:
                case ContentColumn:
                    return article.description();
            }
        }
        case LinkRole:
        {
            return article.link();
        }
        case ItemIdRole:
        case GuidRole:
        {
            return article.guid();
        }
        case FeedIdRole:
        {
            return article.feed() ? article.feed()->xmlUrl() : QVariant();
        }
        case StatusRole:
        {
            return article.status();
        }
        case IsImportantRole:
        {
            return article.keep();
        }
        case IsDeletedRole:
        {
            return article.isDeleted();
        }
    }

    return QVariant();
}

void Akregator::ArticleModel::Private::nodeDestroyed()
{
    node = 0;
    articles.clear();
    q->reset();
}

void ArticleModel::Private::articlesAdded( TreeNode* node, const QList<Article>& list )
{
    Q_UNUSED( node );
    if ( list.isEmpty() ) //assert?
        return;
    const int first = static_cast<int>( articles.count() );
    q->beginInsertRows( QModelIndex(), first, first + list.size() - 1 );
    
    const int oldSize = articles.size();
    articles << list;
    titleCache.resize( articles.count() );
    for ( int i = oldSize; i < articles.count(); ++i )
        titleCache[i] = Syndication::htmlToPlainText( articles[i].title() );
    q->endInsertRows();
}

void ArticleModel::Private::articlesRemoved( TreeNode* node, const QList<Article>& list )
{
    Q_UNUSED( node );
    //might want to avoid indexOf() in case of performance problems
    Q_FOREACH ( const Article& i, list )
    {
        const int row = articles.indexOf( i ); 
        assert( row != -1 );
        q->removeRow( row, QModelIndex() );
    }
}


void ArticleModel::Private::articlesUpdated( TreeNode* node, const QList<Article>& list )
{
    Q_UNUSED( node );
    int rmin = articles.count() - 1;
    int rmax = 0;

    //might want to avoid indexOf() in case of performance problems
    Q_FOREACH ( const Article& i, list )
    {
        const int row = articles.indexOf( i ); 
        assert( row != -1 );
        titleCache[row] = articles[row].title();
        rmin = std::min( row, rmin );
        rmax = std::max( row, rmax );
    }
    emit q->dataChanged( q->index( rmin, 0 ), q->index( rmax, ColumnCount-1 ) );
}


bool ArticleModel::rowMatches( int row, const boost::shared_ptr<const Akregator::Filters::AbstractMatcher>& matcher ) const
{
    assert( matcher );
    return matcher->matches( article( row ) );
}

Article ArticleModel::article( int row ) const
{
    if ( row < 0 || row >= d->articles.count() )
        return Article();
    return d->articles[row];      
}

#include "articlemodel.moc"
