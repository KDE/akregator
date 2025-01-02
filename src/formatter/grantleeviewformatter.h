/*
   SPDX-FileCopyrightText: 2016-2025 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "article.h"
#include "articleformatter.h"
#include <GrantleeTheme/GrantleeThemeEngine>

namespace Akregator
{
struct Colors {
    QString colorScheme;
    QString backgroundColor;
};
class Folder;
class GrantleeViewFormatter
{
public:
    explicit GrantleeViewFormatter(const QString &htmlFileName, int deviceDpiY);
    ~GrantleeViewFormatter();

    [[nodiscard]] QString formatArticles(const QList<Article> &article, ArticleFormatter::IconOption icon);
    [[nodiscard]] QString formatFolder(Akregator::Folder *node);
    [[nodiscard]] QString formatFeed(Akregator::Feed *feed);

private:
    Colors getAppColor() const;
    void addStandardObject(QVariantHash &grantleeObject) const;
    [[nodiscard]] QString sidebarCss(const Colors &colors) const;
    [[nodiscard]] int pointsToPixel(int pointSize) const;
    const QString mHtmlArticleFileName;
    const QString mDirectionString;
    GrantleeTheme::Engine mEngine;
    KTextTemplate::Template mTemplate;
    const int mDeviceDpiY;
};
}
