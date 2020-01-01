/*
   Copyright (C) 2016-2020 Laurent Montel <montel@kde.org>

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

#ifndef GRANTLEEVIEWFORMATTER_H
#define GRANTLEEVIEWFORMATTER_H

#include <GrantleeTheme/GenericFormatter>
#include "article.h"
#include "articleformatter.h"
#include <QUrl>
namespace Akregator {
class Folder;
class GrantleeViewFormatter : public GrantleeTheme::GenericFormatter
{
public:
    explicit GrantleeViewFormatter(const QString &htmlFileName, const QString &themePath, int deviceDpiY);
    ~GrantleeViewFormatter();

    Q_REQUIRED_RESULT QString formatArticles(const QVector<Article> &article, ArticleFormatter::IconOption icon);
    Q_REQUIRED_RESULT QString formatFolder(Akregator::Folder *node);
    Q_REQUIRED_RESULT QString formatFeed(Akregator::Feed *feed);
private:
    void addStandardObject(QVariantHash &grantleeObject);
    int pointsToPixel(int pointSize) const;
    QString mHtmlArticleFileName;
    QString mDirectionString;
    QString mGrantleeThemePath;
    int mDeviceDpiY;
};
}

#endif // GRANTLEEVIEWFORMATTER_H
