/*
  Copyright (c) 2016 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "defaultnormalviewformatter.h"
#include "akregatorconfig.h"
#include "grantleeviewformatter.h"
#include "article.h"
#include "feed.h"
#include "folder.h"
#include "treenode.h"
#include "treenodevisitor.h"
#include "utils.h"

#include <KLocalizedString>

#include <QApplication>
#include <QPaintDevice>
#include <QPalette>
#include <QString>
#include <KFormat>

using namespace Syndication;

using namespace Akregator;

class DefaultNormalViewFormatter::SummaryVisitor : public TreeNodeVisitor
{
public:
    SummaryVisitor(DefaultNormalViewFormatter *p)
        : parent(p)
    {
    }
    ~SummaryVisitor()
    {
    }

    bool visitFeed(Feed *node) Q_DECL_OVERRIDE {
        text = parent->mGrantleeViewFormatter->formatFeed(node);
        return true;
    }

    bool visitFolder(Folder *node) Q_DECL_OVERRIDE {
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
    DefaultNormalViewFormatter *parent;
};

DefaultNormalViewFormatter::DefaultNormalViewFormatter(const QString &grantleeDirectory, const QUrl &imageDir, QPaintDevice *device)
    : ArticleFormatter(),
      m_imageDir(imageDir),
      m_summaryVisitor(new SummaryVisitor(this))
{
    m_DefaultThemePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                         QStringLiteral("akregator/grantleetheme/%1/").arg(grantleeDirectory),
                         QStandardPaths::LocateDirectory);
    mGrantleeViewFormatter = new GrantleeViewFormatter(QStringLiteral("normalview.html"), m_DefaultThemePath, m_imageDir, device->logicalDpiY());
}

DefaultNormalViewFormatter::~DefaultNormalViewFormatter()
{
    delete m_summaryVisitor;
}

QString DefaultNormalViewFormatter::formatSummary(TreeNode *node) const
{
    return m_summaryVisitor->formatSummary(node);
}

QString DefaultNormalViewFormatter::formatArticles(const QVector<Article> &articles, IconOption icon) const
{
    if (articles.count() != 1) {
        return {};
    }
    return mGrantleeViewFormatter->formatArticles(articles, icon);
}

