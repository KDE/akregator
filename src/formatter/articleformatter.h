/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include <QVector>
#include <Syndication/Enclosure>

namespace Akregator
{
class Article;
class TreeNode;
class ArticleFormatterPrivate;

class ArticleFormatter
{
public:
    enum IconOption { NoIcon, ShowIcon };

    explicit ArticleFormatter();

    virtual ~ArticleFormatter();

    virtual QString formatArticles(const QVector<Article> &article, IconOption icon) const = 0;

    virtual QString formatSummary(TreeNode *node) const = 0;

    static QString formatEnclosure(const Syndication::Enclosure &enclosure);

private:
    Q_DISABLE_COPY(ArticleFormatter)
};
} // namespace Akregator
