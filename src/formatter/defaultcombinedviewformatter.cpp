/*
  Copyright (c) 2016 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "defaultcombinedviewformatter.h"
#include "grantleeviewformatter.h"
#include "akregatorconfig.h"
#include "akregator_debug.h"
#include "article.h"
#include "feed.h"
#include "folder.h"
#include "treenode.h"
#include "treenodevisitor.h"
#include "utils.h"

#include <kglobal.h>
#include <KLocalizedString>

#include <QApplication>
#include <QPaintDevice>
#include <QPalette>
#include <QString>
#include <KFormat>
using namespace Akregator;

DefaultCombinedViewFormatter::DefaultCombinedViewFormatter(const QUrl &imageDir, QPaintDevice *device)
    : ArticleFormatter(device)
{
    const QString combinedPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                        QStringLiteral("akregator/grantleetheme/default/"),
                                                        QStandardPaths::LocateDirectory);
    mGrantleeViewFormatter = new GrantleeViewFormatter(QStringLiteral("combinedview.html"), combinedPath, imageDir);
}

DefaultCombinedViewFormatter::~DefaultCombinedViewFormatter()
{
    delete mGrantleeViewFormatter;
}

QString DefaultCombinedViewFormatter::formatArticle(const QVector<Article> &articles, IconOption icon) const
{
    return mGrantleeViewFormatter->formatArticle(articles, icon);
}

QString DefaultCombinedViewFormatter::formatSummary(TreeNode *) const
{
    return QString();
}
