/*
   SPDX-FileCopyrightText: 2016-2025 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "defaultnormalviewformatter.h"
#include "akregatorconfig.h"
#include "article.h"
#include "feed.h"
#include "folder.h"
#include "grantleeviewformatter.h"
#include "treenode.h"
#include "treenodevisitor.h"

#include <QPaintDevice>
#include <QString>

using namespace Syndication;

using namespace Akregator;

class DefaultNormalViewFormatter::SummaryVisitor : public TreeNodeVisitor
{
public:
    explicit SummaryVisitor(DefaultNormalViewFormatter *p)
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

DefaultNormalViewFormatter::DefaultNormalViewFormatter(QPaintDevice *device)
    : ArticleFormatter()
    , m_summaryVisitor(std::make_unique<SummaryVisitor>(this))
    , mGrantleeViewFormatter(std::make_unique<GrantleeViewFormatter>(QStringLiteral(":/formatter/html/normalview.html"), device->logicalDpiY()))
{
}

DefaultNormalViewFormatter::~DefaultNormalViewFormatter() = default;

QString DefaultNormalViewFormatter::formatSummary(TreeNode *node) const
{
    return m_summaryVisitor->formatSummary(node);
}

QString DefaultNormalViewFormatter::formatArticles(const QList<Article> &articles, IconOption icon) const
{
    if (articles.count() != 1) {
        return {};
    }
    return mGrantleeViewFormatter->formatArticles(articles, icon);
}
