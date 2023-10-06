/*
   SPDX-FileCopyrightText: 2016-2023 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "akregator_export.h"
#include "articleformatter.h"
class QPaintDevice;
namespace Akregator
{
class GrantleeViewFormatter;
class AKREGATOR_EXPORT DefaultCombinedViewFormatter : public ArticleFormatter
{
public:
    explicit DefaultCombinedViewFormatter(const QString &grantleeDirectory, QPaintDevice *device = nullptr);
    ~DefaultCombinedViewFormatter() override;

    [[nodiscard]] QString formatArticles(const QList<Article> &articles, IconOption option) const override;

    [[nodiscard]] QString formatSummary(TreeNode *node) const override;

private:
    DefaultCombinedViewFormatter() = delete;
    GrantleeViewFormatter *mGrantleeViewFormatter = nullptr;
};
}
