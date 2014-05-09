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
#include "articlemodel.h"

#include "article.h"
#include "articlematcher.h"
#include "akregatorconfig.h"
#include "feed.h"

#include <Syndication/Tools>

#include <QMimeData>
#include <QString>
#include <QVector>

#include <KLocalizedString>
#include <KGlobal>
#include <KLocale>
#include <KUrl>

#include <memory>

#include <cassert>
#include <cmath>

using namespace Akregator;

class ArticleModel::Private {
private:
    ArticleModel* const q;
public:
    Private( const QList<Article>& articles, ArticleModel* qq );
    QList<Article> articles;
    QVector<QString> titleCache;

    void articlesAdded( const QList<Article>& );
    void articlesRemoved( const QList<Article>& );
    void articlesUpdated( const QList<Article>& );

};

//like Syndication::htmlToPlainText, but without linebreaks

static QString stripHtml( const QString& html ) {
    QString str(html);
    //TODO: preserve some formatting, such as line breaks
    str.remove(QRegExp(QLatin1String("<[^>]*>"))); // remove tags
    str = Syndication::resolveEntities(str);
    return str.simplified();
}

ArticleModel::Private::Private( const QList<Article>& articles_, ArticleModel* qq )
 : q( qq ), articles( articles_ )
{
    titleCache.resize( articles.count() );
    for ( int i = 0; i < articles.count(); ++i )
        titleCache[i] = stripHtml( articles[i].title() );
}

Akregator::ArticleModel::ArticleModel(const QList<Article>& articles, QObject* parent) : QAbstractTableModel( parent ), d( new Private( articles, this ) )
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
            return i18nc("Articlelist's column header", "Title");
        case FeedTitleColumn:
            return i18nc("Articlelist's column header", "Feed");
        case DateColumn:
            return i18nc("Articlelist's column header", "Date");
        case AuthorColumn:
            return i18nc("Articlelist's column header", "Author");
        case DescriptionColumn:
            return i18nc("Articlelist's column header", "Description");
        case ContentColumn:
            return i18nc("Articlelist's column header", "Content");
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
        case SortRole:
            if ( index.column() == DateColumn )
                return article.pubDate();
            // no break
        case Qt::DisplayRole:
        {
            switch ( index.column() )
            {
                case FeedTitleColumn:
                    return article.feed() ? article.feed()->title() : QVariant();
                case DateColumn:
                    return KLocale::global()->formatDateTime(article.pubDate(),
                                                             KLocale::FancyShortDate );
                case ItemTitleColumn:
                    return d->titleCache[row];
                case AuthorColumn:
                    return article.authorShort();
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

void ArticleModel::clear()
{
    d->articles.clear();
    d->titleCache.clear();
    reset();
}

void ArticleModel::articlesAdded( TreeNode*, const QList<Article>& l ) {
    d->articlesAdded( l );
}

void ArticleModel::articlesRemoved( TreeNode*, const QList<Article>& l ) {
    d->articlesRemoved( l );
}
void ArticleModel::articlesUpdated( TreeNode*, const QList<Article>& l ) {
    d->articlesUpdated( l );
}

void ArticleModel::Private::articlesAdded( const QList<Article>& list )
{
    if ( list.isEmpty() ) //assert?
        return;
    const int first = static_cast<int>( articles.count() );
    q->beginInsertRows( QModelIndex(), first, first + list.size() - 1 );

    const int oldSize = articles.size();
    articles << list;
    titleCache.resize( articles.count() );
    for ( int i = oldSize; i < articles.count(); ++i )
        titleCache[i] = stripHtml( articles[i].title() );
    q->endInsertRows();
}

void ArticleModel::Private::articlesRemoved( const QList<Article>& list )
{
    //might want to avoid indexOf() in case of performance problems
    Q_FOREACH ( const Article& i, list )
    {
        const int row = articles.indexOf( i );
        assert( row != -1 );
        q->removeRow( row, QModelIndex() );
    }
}


void ArticleModel::Private::articlesUpdated( const QList<Article>& list )
{
    int rmin = 0;
    int rmax = 0;

    if ( articles.count() > 0 )
    {
         rmin = articles.count() - 1;
        //might want to avoid indexOf() in case of performance problems
        Q_FOREACH ( const Article& i, list )
        {
            const int row = articles.indexOf( i );
            //TODO: figure out how why the Article might not be found in
            //TODO: the articles list because we should need this conditional.
            if ( row >= 0 )
            {
                titleCache[row] = stripHtml( articles[row].title() );
                rmin = std::min( row, rmin );
                rmax = std::max( row, rmax );
            }
        }
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

QStringList ArticleModel::mimeTypes() const
{
    return QStringList() << QString::fromLatin1( "text/uri-list" );
}

QMimeData* ArticleModel::mimeData( const QModelIndexList& indexes ) const
{
    std::auto_ptr<QMimeData> md( new QMimeData );
    QList<QUrl> urls;
    Q_FOREACH( const QModelIndex& i, indexes ) {
        const QUrl url = i.data( ArticleModel::LinkRole ).value<KUrl>();
        if ( url.isValid() )
            urls.push_back( url );
        else {
            const QUrl guid( i.data( ArticleModel::GuidRole ).toString() );
            if ( guid.isValid() )
                urls.push_back( guid );
        }
    }
    md->setUrls( urls );
    return md.release();
}

Qt::ItemFlags ArticleModel::flags( const QModelIndex& idx ) const
{
    const Qt::ItemFlags f = QAbstractTableModel::flags( idx );
    if ( !idx.isValid() )
        return f;
    return f | Qt::ItemIsDragEnabled;
}

