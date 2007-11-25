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
#include "akregatorconfig.h"
#include "treenode.h"
#include "feed.h"

#include <QString>

#include <KGlobal>
#include <KLocale>

using namespace Akregator;

class ArticleModel::Private {
private:
    ArticleModel* const q;
public:
    Private( ArticleModel* qq ) : q( qq ) {}
    Akregator::TreeNode* node;
    QList<Akregator::Article> articles;
    void nodeDestroyed();
    void articlesAdded( TreeNode*, const QList<Article>& );
    void articlesRemoved( TreeNode*, const QList<Article>& );
    void articlesUpdated( TreeNode*, const QList<Article>& );

};

Akregator::ArticleModel::ArticleModel(TreeNode* node, QObject* parent) : QAbstractTableModel( parent ), d( new Private( this ) )
{
    d->node = node;
    Q_ASSERT( node );
    d->articles = node->articles();
    connect( node, SIGNAL(destroyed()), this, SLOT(nodeDestroyed()) );
#if 0 // PENDING: reenable when slots are implemented correctly
    connect( node, SIGNAL(signalArticlesAdded(Akregator::TreeNode*, QList<Akregator::Article>)), SLOT((articlesAdded(Akregator::TreeNode*, QList<Akregator::Article>)) );
    connect( node, SIGNAL(signalArticlesRemoved(Akregator::TreeNode*, QList<Akregator::Article>)), SLOT((articlesRemoved(Akregator::TreeNode*, QList<Akregator::Article>)) );
    connect( node, SIGNAL(signalArticlesUpdated(Akregator::TreeNode*, QList<Akregator::Article>)), SLOT((articlesUpdated(Akregator::TreeNode*, QList<Akregator::Article>)) );
#endif 
}

Akregator::ArticleModel::~ArticleModel()
{
    delete d;
}

int Akregator::ArticleModel::columnCount( const QModelIndex& parent ) const
{
    return parent.isValid() ? 0 : 3;
}

int Akregator::ArticleModel::rowCount( const QModelIndex& parent ) const
{
    return parent.isValid() ? 0 : d->articles.count();
}

QVariant Akregator::ArticleModel::data( const QModelIndex& index, int role ) const
{
    if ( !index.isValid() || index.row() < 0 || index.row() >= d->articles.count() )
        return QVariant();
    const Akregator::Article article = d->articles[ index.row() ];

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
                    return article.title();
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
//TODO
    articles = node->articles();
    q->reset();
}

void ArticleModel::Private::articlesRemoved( TreeNode* node, const QList<Article>& list )
{
//TODO
    articles = node->articles();
    q->reset();
}


void ArticleModel::Private::articlesUpdated( TreeNode* node, const QList<Article>& list )
{
//TODO
    articles = node->articles();
    q->reset();
}

#include "articlemodel.moc"
