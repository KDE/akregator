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
class AKREGATOR_EXPORT DefaultNormalViewFormatter : public ArticleFormatter
{
public:
    explicit DefaultNormalViewFormatter(const QString &grantleeDirectory, QPaintDevice *device = nullptr);
    ~DefaultNormalViewFormatter() override;

    Q_REQUIRED_RESULT QString formatArticles(const QVector<Article> &article, IconOption option) const override;

    Q_REQUIRED_RESULT QString formatSummary(TreeNode *node) const override;

private:
    DefaultNormalViewFormatter() = delete;
    QString m_DefaultThemePath;
    class SummaryVisitor;
    SummaryVisitor *m_summaryVisitor = nullptr;
    GrantleeViewFormatter *mGrantleeViewFormatter = nullptr;
};
}
