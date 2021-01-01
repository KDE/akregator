/*
   SPDX-FileCopyrightText: 2016-2021 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DEFAULTNORMALVIEWFORMATTER_H
#define DEFAULTNORMALVIEWFORMATTER_H

#include "articleformatter.h"
#include "akregator_export.h"
class QPaintDevice;
namespace Akregator {
class GrantleeViewFormatter;
class AKREGATOR_EXPORT DefaultNormalViewFormatter : public ArticleFormatter
{
public:

    explicit DefaultNormalViewFormatter(const QString &grantleeDirectory, QPaintDevice *device = nullptr);
    ~DefaultNormalViewFormatter() override;

    Q_REQUIRED_RESULT QString formatArticles(const QVector<Article> &article, IconOption option) const override;

    Q_REQUIRED_RESULT QString formatSummary(TreeNode *node) const override;

private:
    DefaultNormalViewFormatter();
    QString m_DefaultThemePath;
    class SummaryVisitor;
    SummaryVisitor *m_summaryVisitor = nullptr;
    GrantleeViewFormatter *mGrantleeViewFormatter = nullptr;
};
}
#endif // DEFAULTNORMALVIEWFORMATTER_H
