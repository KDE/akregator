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
}

DefaultNormalViewFormatter::~DefaultNormalViewFormatter()
{
    delete m_summaryVisitor;
}

QString DefaultNormalViewFormatter::formatSummary(TreeNode *node) const
{
    return m_summaryVisitor->formatSummary(node);
}

QString DefaultNormalViewFormatter::formatArticle(const Article &article, IconOption icon) const
{
    QString text;
    text = QStringLiteral("<div class=\"headerbox\" dir=\"%1\">\n").arg(QApplication::isRightToLeft() ? QStringLiteral("rtl") : QStringLiteral("ltr"));
    const QString enc = formatEnclosure(*article.enclosure());

    const QString strippedTitle = Utils::stripTags(article.title());
    if (!strippedTitle.isEmpty()) {
        text += QStringLiteral("<div class=\"headertitle\" dir=\"%1\">\n").arg(Utils::directionOf(strippedTitle));
        if (article.link().isValid()) {
            text += QLatin1String("<a href=\"") + article.link().url() + QLatin1String("\">");
        }
        text += strippedTitle;
        if (article.link().isValid()) {
            text += QLatin1String("</a>");
        }
        text += QStringLiteral("</div>\n");
    }
    if (article.pubDate().isValid()) {
        text += QStringLiteral("<span class=\"header\" dir=\"%1\">").arg(Utils::directionOf(i18n("Date")));
        text += QStringLiteral("%1:").arg(i18n("Date"));
        text += QLatin1String("</span><span class=\"headertext\">");
        text += KLocale::global()->formatDateTime(article.pubDate(), KLocale::FancyLongDate) + QLatin1String("</span>\n"); // TODO: might need RTL?
    }
    const QString author = article.authorAsHtml();
    if (!author.isEmpty()) {
        text += QStringLiteral("<br/><span class=\"header\" dir=\"%1\">").arg(Utils::directionOf(i18n("Author")));
        text += QStringLiteral("%1:").arg(i18n("Author"));
        text += QLatin1String("</span><span class=\"headertext\">");
        text += author + QLatin1String("</span>\n"); // TODO: might need RTL?
    }

    if (!enc.isEmpty()) {
        text += QStringLiteral("<br/><span class=\"header\" dir=\"%1\">").arg(Utils::directionOf(i18n("Enclosure")));
        text += QStringLiteral("%1:").arg(i18n("Enclosure"));
        text += QLatin1String("</span><span class=\"headertext\">");
        text += enc + QLatin1String("</span>\n"); // TODO: might need RTL?
    }

    text += QStringLiteral("</div>\n"); // end headerbox

    if (icon == ShowIcon && article.feed() && !article.feed()->image().isNull()) {
        const Feed *feed = article.feed();
        QString file = Utils::fileNameForUrl(feed->xmlUrl());
        QUrl u(m_imageDir);
        u = u.adjusted(QUrl::RemoveFilename);
        u.setPath(u.path() + file);
        text += QStringLiteral("<a href=\"%1\"><img class=\"headimage\" src=\"%2.png\"></a>\n").arg(feed->htmlUrl(), u.url());
    }

    const QString content = article.content(Article::DescriptionAsFallback);
    if (!content.isEmpty()) {
        text += QStringLiteral("<div dir=\"%1\">").arg(Utils::directionOf(Utils::stripTags(content)));
        text += QLatin1String("<span class=\"content\">") + content + QLatin1String("</span>");
        text += QLatin1String("</div>");
    }

    text += QLatin1String("<div class=\"body\">");

    if (article.commentsLink().isValid()) {
        text += QLatin1String("<a class=\"contentlink\" href=\"");
        text += article.commentsLink().url();
        text += QLatin1String("\">") + i18n("Comments");
        if (article.comments()) {
            text += QLatin1String(" (") + QString::number(article.comments()) + QLatin1Char(')');
        }
        text += QLatin1String("</a>");
    }

    if (!enc.isEmpty()) {
        text += QStringLiteral("<p><em>%1</em> %2</p>").arg(i18n("Enclosure:")).arg(enc);
    }

    if (article.link().isValid() || (article.guidIsPermaLink() && QUrl(article.guid()).isValid())) {
        text += QLatin1String("<p><a class=\"contentlink\" href=\"");
        // in case link isn't valid, fall back to the guid permaLink.
        if (article.link().isValid()) {
            text += article.link().url();
        } else {
            text += article.guid();
        }
        text += QLatin1String("\">") + i18n("Complete Story") + QLatin1String("</a></p>");
    }

    text += QLatin1String("</div>");

    return text;
}

QString DefaultNormalViewFormatter::getCss() const
{
    const QPalette &pal = QApplication::palette();

    // from kmail::headerstyle.cpp
    QString css = QStringLiteral(
                      "<style type=\"text/css\">\n"
                      "@media screen, print {"
                      "body {\n"
                      "  font-family: \"%1\" ! important;\n"
                      "  font-size: %2 ! important;\n"
                      "  color: %3 ! important;\n"
                      "  background: %4 ! important;\n"
                      "}\n\n")
                  .arg(Settings::standardFont(),
                       QString::number(pointsToPixel(Settings::mediumFontSize())) + QLatin1String("px"),
                       pal.color(QPalette::Text).name(),
                       pal.color(QPalette::Base).name());
    css += QString(
               QLatin1String("a {\n")
               + QLatin1String("  color: %1 ! important;\n")
               +       QLatin1String("}\n\n")
               + QLatin1String(".headerbox {\n")
               + QLatin1String("  background: %2 ! important;\n")
               + QLatin1String("  color: %3 ! important;\n")
               + QLatin1String("  border:1px solid #000;\n")
               + QLatin1String("  margin-bottom: 10pt;\n")
               +        QLatin1String("}\n\n"))
           .arg(pal.color(QPalette::Link).name(),
                pal.color(QPalette::Background).name(),
                pal.color(QPalette::Text).name());
    css += QStringLiteral(".headertitle a:link { color: %1 ! important;\n text-decoration: none ! important;\n }\n"
                          ".headertitle a:visited { color: %1 ! important;\n text-decoration: none ! important;\n }\n"
                          ".headertitle a:hover{ color: %1 ! important;\n text-decoration: none ! important;\n }\n"
                          ".headertitle a:active { color: %1 ! important;\n  text-decoration: none ! important;\n }\n")
           .arg(pal.color(QPalette::HighlightedText).name());
    css += QStringLiteral(
               ".headertitle {\n"
               "  background: %1 ! important;\n"
               "  padding:2px;\n"
               "  color: %2 ! important;\n"
               "  font-weight: bold;\n"
               "  text-decoration: none ! important;\n"
               "}\n\n"
               ".header {\n"
               "  font-weight: bold;\n"
               "  padding:2px;\n"
               "  margin-right: 5px;\n"
               "  text-decoration: none ! important;\n"
               "}\n\n"
               ".headertext a {\n"
               "  text-decoration: none ! important;\n"
               "}\n\n"
               ".headimage {\n"
               "  float: right;\n"
               "  margin-left: 5px;\n"
               "}\n\n").arg(
               pal.color(QPalette::Highlight).name(),
               pal.color(QPalette::HighlightedText).name());

    css += QLatin1String(
               "body { clear: none; }\n\n"
               ".content {\n"
               "  display: block;\n"
               "  margin-bottom: 6px;\n"
               "}\n\n"
               // these rules make sure that there is no leading space between the header and the first of the text
               ".content > P:first-child {\n margin-top: 1px; }\n"
               ".content > DIV:first-child {\n margin-top: 1px; }\n"
               // Do we really need that? See bug #144420
//            ".content > BR:first-child {\n display: none;  }\n"
               //".contentlink {\n display: block; }\n"
               "}\n\n" // @media screen, print
               // Why did we need that, bug #108187?
               //"@media screen { body { overflow: auto; } }\n"
               "\n\n");

    return css;
}

