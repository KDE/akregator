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

struct Akregator::ArticleModelPrivate {
    Akregator::TreeNode* node;
    QList<Akregator::Article> articles;
};

Akregator::ArticleModel::ArticleModel(TreeNode* node, QObject* parent) : QAbstractListModel( parent ), d( new Akregator::ArticleModelPrivate )
{
    d->node = node;
    Q_ASSERT( node );
    d->articles = node->articles();
    connect( node, SIGNAL(destroyed()), this, SLOT(nodeDestroyed()) );
    connect( node, SIGNAL(signalArticlesAdded(Akregator::TreeNode*, QList<Akregator::Article>)), SLOT(articlesChanged()) );
    connect( node, SIGNAL(signalArticlesRemoved(Akregator::TreeNode*, QList<Akregator::Article>)), SLOT(articlesChanged()) );
    connect( node, SIGNAL(signalArticlesUpdated(Akregator::TreeNode*, QList<Akregator::Article>)), SLOT(articlesChanged()) );
}

Akregator::ArticleModel::~ArticleModel()
{
    delete d;
    d = 0;
}

int Akregator::ArticleModel::columnCount( const QModelIndex& ) const
{
    return 3;
}

int Akregator::ArticleModel::rowCount( const QModelIndex& parent ) const
{
    return !parent.isValid() ? d->articles.count() : 0;
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

void Akregator::ArticleModel::nodeDestroyed()
{
    d->node = 0;
    d->articles.clear();
}

void Akregator::ArticleModel::articlesChanged()
{
    d->articles = d->node->articles();
    reset();
}

#include "articlemodel.moc"
