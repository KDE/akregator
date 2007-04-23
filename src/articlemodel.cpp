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
#include "akregatorconfig.h"
#include "treenode.h"
#include "feed.h"

#include <QString>

#include <KGlobal>
#include <KLocale>


Akregator::ArticleModel::ArticleModel(TreeNode* node, QObject* parent) : QAbstractListModel( parent ), m_node(node)
{
    Q_ASSERT(m_node);
    m_articles = m_node->articles();
    connect( m_node, SIGNAL(destroyed()), this, SLOT(nodeDestroyed()) );
    connect( m_node, SIGNAL(signalArticlesAdded(Akregator::TreeNode*, QList<Akregator::Article>)), SLOT(articlesChanged()) );
    connect( m_node, SIGNAL(signalArticlesRemoved(Akregator::TreeNode*, QList<Akregator::Article>)), SLOT(articlesChanged()) );
    connect( m_node, SIGNAL(signalArticlesUpdated(Akregator::TreeNode*, QList<Akregator::Article>)), SLOT(articlesChanged()) );
}

int Akregator::ArticleModel::columnCount( const QModelIndex& ) const
{
    return 3;
}

int Akregator::ArticleModel::rowCount( const QModelIndex& parent ) const
{
    return !parent.isValid() ? m_articles.count() : 0;
}

QVariant Akregator::ArticleModel::data( const QModelIndex& index, int role ) const
{
    if ( !index.isValid() || index.row() < 0 || index.row() >= m_articles.count() )
        return QVariant();
    const Akregator::Article article = m_articles[ index.row() ];

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
                default:
                    return article.title();
            }
        }
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
    m_node = 0;
    m_articles.clear();
}

void Akregator::ArticleModel::articlesChanged()
{
    m_articles = m_node->articles();
}

#include "articlemodel.moc"
