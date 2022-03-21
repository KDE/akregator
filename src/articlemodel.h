/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2007 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#pragma once

#include <QAbstractTableModel>

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
    enum Column { ItemTitleColumn = 0, FeedTitleColumn, AuthorColumn, DateColumn, DescriptionColumn, ContentColumn, ColumnCount };

    enum Role { SortRole = Qt::UserRole, LinkRole, GuidRole, ItemIdRole, FeedIdRole, StatusRole, IsImportantRole, IsDeletedRole };

    explicit ArticleModel(const QVector<Article> &articles, QObject *parent = nullptr);
    ~ArticleModel() override;

    Q_REQUIRED_RESULT int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Q_REQUIRED_RESULT int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    Q_REQUIRED_RESULT QVariant headerData(int section, Qt::Orientation, int role) const override;

    Q_REQUIRED_RESULT QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Q_REQUIRED_RESULT bool rowMatches(int row, const QSharedPointer<const Akregator::Filters::AbstractMatcher> &matcher) const;

    Q_REQUIRED_RESULT Article article(int row) const;

    Q_REQUIRED_RESULT QStringList mimeTypes() const override;

    QMimeData *mimeData(const QModelIndexList &indexes) const override;

    Q_REQUIRED_RESULT Qt::ItemFlags flags(const QModelIndex &idx) const override;

public Q_SLOTS:

    void articlesAdded(Akregator::TreeNode *, const QVector<Akregator::Article> &);
    void articlesUpdated(Akregator::TreeNode *, const QVector<Akregator::Article> &);
    void articlesRemoved(Akregator::TreeNode *, const QVector<Akregator::Article> &);
    void clear();

private:
    ArticleModel(const ArticleModel &);
    ArticleModel &operator=(const ArticleModel &);

    QVector<Article> m_articles;
    QVector<QString> m_titleCache;
};
} // namespace Akregator
