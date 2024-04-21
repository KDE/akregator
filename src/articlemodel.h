/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2007 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#pragma once

#include <QAbstractTableModel>

#include "akregatorpart_export.h"
#include "article.h"

#include <QSharedPointer>

namespace Akregator
{
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

    explicit ArticleModel(const QList<Article> &articles, QObject *parent = nullptr);
    ~ArticleModel() override;

    [[nodiscard]] int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    [[nodiscard]] QVariant headerData(int section, Qt::Orientation, int role) const override;

    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    [[nodiscard]] bool rowMatches(int row, const QSharedPointer<const Akregator::Filters::AbstractMatcher> &matcher) const;

    [[nodiscard]] Article article(int row) const;

    [[nodiscard]] QStringList mimeTypes() const override;

    QMimeData *mimeData(const QModelIndexList &indexes) const override;

    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &idx) const override;

public Q_SLOTS:

    void articlesAdded(Akregator::TreeNode *, const QList<Akregator::Article> &);
    void articlesUpdated(Akregator::TreeNode *, const QList<Akregator::Article> &);
    void articlesRemoved(Akregator::TreeNode *, const QList<Akregator::Article> &);
    void clear();

private:
    ArticleModel(const ArticleModel &);
    ArticleModel &operator=(const ArticleModel &);

    QList<Article> m_articles;
    QList<QString> m_titleCache;
};
} // namespace Akregator
