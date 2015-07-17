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
#ifndef AKREGATOR_ARTICLEMODEL_H
#define AKREGATOR_ARTICLEMODEL_H

#include <QAbstractTableModel>
#include <QList>

#include "akregatorpart_export.h"

#include <QSharedPointer>
#include <vector>

namespace Akregator
{

class Article;
class TreeNode;

namespace Filters
{
class AbstractMatcher;
}

class AKREGATORPART_EXPORT ArticleModel : public QAbstractTableModel
{
    Q_OBJECT

public:

    enum Column {
        ItemTitleColumn = 0,
        FeedTitleColumn,
        AuthorColumn,
        DateColumn,
        DescriptionColumn,
        ContentColumn,
        ColumnCount
    };

    enum Role {
        SortRole = Qt::UserRole,
        LinkRole,
        GuidRole,
        ItemIdRole,
        FeedIdRole,
        StatusRole,
        IsImportantRole,
        IsDeletedRole
    };

    explicit ArticleModel(const QVector<Article> &articles, QObject *parent = Q_NULLPTR);
    ~ArticleModel();

    //reimpl
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    //impl
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    QVariant headerData(int section, Qt::Orientation, int role) const Q_DECL_OVERRIDE;
    //impl
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    bool rowMatches(int row, const QSharedPointer<const Akregator::Filters::AbstractMatcher> &matcher) const;

    Article article(int row) const;

    QStringList mimeTypes() const Q_DECL_OVERRIDE;

    QMimeData *mimeData(const QModelIndexList &indexes) const Q_DECL_OVERRIDE;

    Qt::ItemFlags flags(const QModelIndex &idx) const Q_DECL_OVERRIDE;

public Q_SLOTS:

    void articlesAdded(Akregator::TreeNode *, const QVector<Akregator::Article> &);
    void articlesUpdated(Akregator::TreeNode *, const QVector<Akregator::Article> &);
    void articlesRemoved(Akregator::TreeNode *, const QVector<Akregator::Article> &);
    void clear();

private:
    ArticleModel(const ArticleModel &);
    ArticleModel &operator=(const ArticleModel &);

    class Private;
    Private *const d;
};

} //namespace Akregator

#endif // AKREGATOR_ARTICLEMODEL_H
