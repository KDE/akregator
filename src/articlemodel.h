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
#ifndef AKREGATOR_ARTICLEMODEL_H
#define AKREGATOR_ARTICLEMODEL_H

#include <QAbstractListModel>

#include "akregator_export.h"

namespace Akregator {

class TreeNode;

struct ArticleModelPrivate;

class AKREGATORPART_EXPORT ArticleModel : public QAbstractListModel
{
    Q_OBJECT

public:

    enum Column { ItemTitleColumn=0, 
                  FeedTitleColumn,
                  DateColumn,
                  AuthorColumn, 
                  DescriptionColumn,
                  ContentColumn,
                  ColumnCount };

    enum Role {
        LinkRole=Qt::UserRole,
        GuidRole,
        ItemIdRole,
        FeedIdRole,
        StatusRole,
        IsImportantRole
    };

    explicit ArticleModel( Akregator::TreeNode* node, QObject* parent = 0 );
    ~ArticleModel();

    //reimpl
    int columnCount( const QModelIndex& parent=QModelIndex() ) const;

    //impl
    int rowCount( const QModelIndex& parent=QModelIndex() ) const;

    //impl
    QVariant data( const QModelIndex& index, int role=Qt::DisplayRole ) const;

private Q_SLOTS:

    void nodeDestroyed();
    void articlesChanged();

private:
    ArticleModel( const ArticleModel& );
    ArticleModel& operator=( const ArticleModel& );

    ArticleModelPrivate* d;
};

} //namespace Akregator

#endif // AKREGATOR_ARTICLEMODEL_H
