/*
   SPDX-FileCopyrightText: 2016-2026 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "akregator_export.h"
#include "articleformatter.h"
class QPaintDevice;
namespace Akregator
{
class GrantleeViewFormatter;
class AKREGATOR_EXPORT DefaultNormalViewFormatter : public ArticleFormatter
{
public:
    explicit DefaultNormalViewFormatter(QPaintDevice *device);
    ~DefaultNormalViewFormatter() override;

    [[nodiscard]] QString formatArticles(const QList<Article> &article, IconOption option) const override;

    [[nodiscard]] QString formatSummary(TreeNode *node) const override;

private:
    QString m_DefaultThemePath;
    class SummaryVisitor;
    std::unique_ptr<SummaryVisitor> m_summaryVisitor;
    std::unique_ptr<GrantleeViewFormatter> mGrantleeViewFormatter;
};
}
