/*
   SPDX-FileCopyrightText: 2016-2020 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef GRANTLEEVIEWFORMATTER_H
#define GRANTLEEVIEWFORMATTER_H

#include <GrantleeTheme/GenericFormatter>
#include "article.h"
#include "articleformatter.h"
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
