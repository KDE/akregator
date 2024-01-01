/*
   SPDX-FileCopyrightText: 2016-2024 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "article.h"
#include "articleformatter.h"
#include <GrantleeTheme/GenericFormatter>
namespace Akregator
{
class Folder;
class GrantleeViewFormatter : public GrantleeTheme::GenericFormatter
{
public:
    explicit GrantleeViewFormatter(const QString &htmlFileName, const QString &themePath, int deviceDpiY);
    ~GrantleeViewFormatter();

    [[nodiscard]] QString formatArticles(const QList<Article> &article, ArticleFormatter::IconOption icon);
    [[nodiscard]] QString formatFolder(Akregator::Folder *node);
    [[nodiscard]] QString formatFeed(Akregator::Feed *feed);

private:
    void addStandardObject(QVariantHash &grantleeObject);
    [[nodiscard]] int pointsToPixel(int pointSize) const;
    const QString mHtmlArticleFileName;
    const QString mDirectionString;
    const QString mGrantleeThemePath;
    const int mDeviceDpiY;
};
}
