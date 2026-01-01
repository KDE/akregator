/*
   SPDX-FileCopyrightText: 2016-2026 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "defaultcombinedviewformatter.h"
#include "article.h"
#include "grantleeviewformatter.h"
#include "treenode.h"

#include <QPaintDevice>
#include <QString>
using namespace Akregator;

DefaultCombinedViewFormatter::DefaultCombinedViewFormatter(QPaintDevice *device)
    : ArticleFormatter()
{
    mGrantleeViewFormatter = std::make_unique<GrantleeViewFormatter>(QStringLiteral(":/formatter/html/combinedview.html"), device->logicalDpiY());
}

DefaultCombinedViewFormatter::~DefaultCombinedViewFormatter() = default;

QString DefaultCombinedViewFormatter::formatArticles(const QList<Article> &articles, IconOption icon) const
{
    return mGrantleeViewFormatter->formatArticles(articles, icon);
}

QString DefaultCombinedViewFormatter::formatSummary(TreeNode *) const
{
    return {};
}
