/*
   SPDX-FileCopyrightText: 2016-2024 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "akregator_export.h"
#include "articleformatter.h"
#include "grantleeviewformatter.h"
#include "treenodevisitor.h"
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
    DefaultNormalViewFormatter() = delete;
    class SummaryVisitor : public TreeNodeVisitor
    {
    public:
        SummaryVisitor(DefaultNormalViewFormatter *p)
            : parent(p)
        {
        }

        ~SummaryVisitor() override = default;

        bool visitFeed(Feed *node) override
        {
            text = parent->mGrantleeViewFormatter->formatFeed(node);
            return true;
        }

        bool visitFolder(Folder *node) override
        {
            text = parent->mGrantleeViewFormatter->formatFolder(node);
            return true;
        }

        QString formatSummary(TreeNode *node)
        {
            text.clear();
            visit(node);
            return text;
        }

        QString text;

    private:
        DefaultNormalViewFormatter *const parent;
    };
    std::unique_ptr<SummaryVisitor> m_summaryVisitor;
    std::unique_ptr<GrantleeViewFormatter> mGrantleeViewFormatter;
};
}
