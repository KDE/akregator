/*
   Copyright (C) 2016 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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

#include <KLocalizedString>

#include <QApplication>
#include <QPaintDevice>
#include <QPalette>
#include <QString>
#include <KFormat>
using namespace Akregator;

DefaultCombinedViewFormatter::DefaultCombinedViewFormatter(const QString &grantleeDirectory, const QUrl &imageDir, QPaintDevice *device)
    : ArticleFormatter()
{
    const QString combinedPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                 QStringLiteral("akregator/grantleetheme/%1/").arg(grantleeDirectory),
                                 QStandardPaths::LocateDirectory);
    mGrantleeViewFormatter = new GrantleeViewFormatter(QStringLiteral("combinedview.html"), combinedPath, imageDir, device->logicalDpiY());
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
