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

#include <kglobal.h>
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
    //TODO replace with grantlee
    SummaryVisitor(DefaultNormalViewFormatter *p) : parent(p) {}
    bool visitFeed(Feed *node) Q_DECL_OVERRIDE {
        text = QStringLiteral("<div class=\"headerbox\" dir=\"%1\">\n").arg(QApplication::isRightToLeft() ? QStringLiteral("rtl") : QStringLiteral("ltr"));
        const QString strippedTitle = Utils::stripTags(node->title());
        text += QStringLiteral("<div class=\"headertitle\" dir=\"%1\">").arg(Utils::directionOf(strippedTitle));
        text += strippedTitle;
        if (node->unread() == 0)
        {
            text += i18n(" (no unread articles)");
        } else {
            text += i18np(" (1 unread article)", " (%1 unread articles)", node->unread());
        }
        text += QStringLiteral("</div>\n"); // headertitle
        text += QStringLiteral("</div>\n"); // /headerbox

        if (!node->image().isNull())   // image
        {
            text += QLatin1String("<div class=\"body\">");
            QString file = Utils::fileNameForUrl(node->xmlUrl());
            QUrl u(parent->m_imageDir);
            u = u.adjusted(QUrl::RemoveFilename);
            u.setPath(u.path() + file);
            text += QStringLiteral("<a href=\"%1\"><img class=\"headimage\" src=\"%2.png\"></a>\n").arg(node->htmlUrl(), u.url());
        } else {
            text += QStringLiteral("<div class=\"body\">");
        }

        if (!node->description().isEmpty())
        {
            text += QStringLiteral("<div dir=\"%1\">").arg(Utils::stripTags(Utils::directionOf(node->description())));
            text += i18n("<b>Description:</b> %1<br /><br />", node->description());
            text += QStringLiteral("</div>\n"); // /description
        }

        if (!node->htmlUrl().isEmpty())
        {
            text += QStringLiteral("<div dir=\"%1\">").arg(Utils::directionOf(node->htmlUrl()));
            text += i18n("<b>Homepage:</b> <a href=\"%1\">%2</a>", node->htmlUrl(), node->htmlUrl());
            text += QStringLiteral("</div>\n"); // / link
        }

        //text += i18n("<b>Unread articles:</b> %1").arg(node->unread());
        text += QStringLiteral("</div>"); // /body

        return true;
    }

    bool visitFolder(Folder *node) Q_DECL_OVERRIDE {
        text = QStringLiteral("<div class=\"headerbox\" dir=\"%1\">\n").arg(QApplication::isRightToLeft() ? QStringLiteral("rtl") : QStringLiteral("ltr"));
        text += QStringLiteral("<div class=\"headertitle\" dir=\"%1\">%2").arg(Utils::directionOf(Utils::stripTags(node->title())), node->title());
        if (node->unread() == 0)
        {
            text += i18n(" (no unread articles)");
        } else {
            text += i18np(" (1 unread article)", " (%1 unread articles)", node->unread());
        }
        text += QStringLiteral("</div>\n");
        text += QStringLiteral("</div>\n"); // /headerbox

        return true;
    }

    QString formatSummary(TreeNode *node)
    {
        text.clear();
        visit(node);
        return text;
    }

    QString text;
    DefaultNormalViewFormatter *parent;
};

DefaultNormalViewFormatter::DefaultNormalViewFormatter(const QUrl &imageDir, QPaintDevice *device)
    : ArticleFormatter(device),
      m_imageDir(imageDir),
      m_summaryVisitor(new SummaryVisitor(this))
{
    m_DefaultThemePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                QStringLiteral("akregator/grantleetheme/default/"),
                                                QStandardPaths::LocateDirectory);
    mGrantleeViewFormatter = new GrantleeViewFormatter(QStringLiteral("normalview.html"), m_DefaultThemePath, m_imageDir);
}

DefaultNormalViewFormatter::~DefaultNormalViewFormatter()
{
    delete m_summaryVisitor;
}

QString DefaultNormalViewFormatter::formatSummary(TreeNode *node) const
{
    return m_summaryVisitor->formatSummary(node);
}

QString DefaultNormalViewFormatter::formatArticle(const QVector<Article> &articles, IconOption icon) const
{
    if (articles.count() != 1) {
        return {};
    }
    return mGrantleeViewFormatter->formatArticle(articles, icon);
}

