/*
   SPDX-FileCopyrightText: 2016-2021 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "defaultcombinedviewformatter.h"
#include "akregator_debug.h"
#include "akregatorconfig.h"
#include "article.h"
#include "feed.h"
#include "folder.h"
#include "grantleeviewformatter.h"
#include "treenode.h"
#include "treenodevisitor.h"
#include "utils.h"

#include <QPaintDevice>
#include <QString>
using namespace Akregator;

DefaultCombinedViewFormatter::DefaultCombinedViewFormatter(const QString &grantleeDirectory, QPaintDevice *device)
    : ArticleFormatter()
{
    const QString combinedPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                        QStringLiteral("akregator/grantleetheme/%1/").arg(grantleeDirectory),
                                                        QStandardPaths::LocateDirectory);
    mGrantleeViewFormatter = new GrantleeViewFormatter(QStringLiteral("combinedview.html"), combinedPath, device->logicalDpiY());
}

DefaultCombinedViewFormatter::~DefaultCombinedViewFormatter()
{
    delete mGrantleeViewFormatter;
}

QString DefaultCombinedViewFormatter::formatArticles(const QVector<Article> &articles, IconOption icon) const
{
    return mGrantleeViewFormatter->formatArticles(articles, icon);
}

QString DefaultCombinedViewFormatter::formatSummary(TreeNode *) const
{
    return QString();
}
