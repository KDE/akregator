/*
    This file is part of Akregator.

    Copyright (C) 2006 Frank Osterfeld <osterfeld@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "akregatorconfig.h"
#include "article.h"
#include "articleformatter.h"
#include "feed.h"
#include "folder.h"
#include "treenode.h"
#include "treenodevisitor.h"
#include "utils.h"

#include <kglobal.h>
#include <klocale.h>

#include <QApplication>
#include <QPaintDevice>
#include <QPalette>
#include <QString>

using namespace boost;
using namespace Syndication;
using namespace Akregator;

namespace {
    QString formatEnclosure( const Enclosure& enclosure )
    {
        if ( enclosure.isNull() )
            return QString();

        const QString title = !enclosure.title().isEmpty() ? enclosure.url() : enclosure.url();
        const uint length = enclosure.length();
        const QString type = enclosure.type();
        QString inf;
        if ( !type.isEmpty() && length > 0 )
            inf = i18n( "(%1, %2)", type, KGlobal::locale()->formatByteSize( length ) );
        else if ( !type.isNull() )
            inf = type;
        else if ( length > 0 )
            inf = KGlobal::locale()->formatByteSize( length );
        QString str = i18n( "<a href=\"%1\">%2</a> %3", enclosure.url(), title, inf );
        return str;
    }
}
class ArticleFormatter::Private
{
    public:
        explicit Private( QPaintDevice* device_ );
        QPaintDevice* device;
        class SummaryVisitor;
};

ArticleFormatter::Private::Private( QPaintDevice* device_ ) : device( device_ )
{
}

ArticleFormatter::ArticleFormatter( QPaintDevice* device ) : d( new Private( device ) )
{
}

ArticleFormatter::~ArticleFormatter()
{
    delete d;
}

void ArticleFormatter::setPaintDevice(QPaintDevice* device)
{
    d->device = device;
}

int ArticleFormatter::pointsToPixel(int pointSize) const
{
    return ( pointSize * d->device->logicalDpiY() + 36 ) / 72 ;
}

class DefaultNormalViewFormatter::SummaryVisitor : public TreeNodeVisitor
{
    public:
        SummaryVisitor(DefaultNormalViewFormatter* p) : parent(p) {}
        virtual bool visitFeed(Feed* node)
        {
            text = QString("<div class=\"headerbox\" dir=\"%1\">\n").arg(QApplication::isRightToLeft() ? "rtl" : "ltr");

            text += QString("<div class=\"headertitle\" dir=\"%1\">").arg(Utils::directionOf(Utils::stripTags(node->title())));
            text += node->title();
            if(node->unread() == 0)
                text += i18n(" (no unread articles)");
            else
                text += i18np(" (1 unread article)", " (%1 unread articles)", node->unread());
            text += "</div>\n"; // headertitle
            text += "</div>\n"; // /headerbox

            if (!node->image().isNull()) // image
            {
                text += QString("<div class=\"body\">");
                QString file = Utils::fileNameForUrl(node->xmlUrl());
                KUrl u(parent->m_imageDir);
                u.setFileName(file);
                text += QString("<a href=\"%1\"><img class=\"headimage\" src=\"%2.png\"></a>\n").arg(node->htmlUrl(), u.url());
            }
            else text += "<div class=\"body\">";


            if( !node->description().isEmpty() )
            {
                text += QString("<div dir=\"%1\">").arg(Utils::stripTags(Utils::directionOf(node->description())));
                text += i18n("<b>Description:</b> %1<br /><br />", node->description());
                text += "</div>\n"; // /description
            }

            if ( !node->htmlUrl().isEmpty() )
            {
                text += QString("<div dir=\"%1\">").arg(Utils::directionOf(node->htmlUrl()));
                text += i18n("<b>Homepage:</b> <a href=\"%1\">%2</a>", node->htmlUrl(), node->htmlUrl());
                text += "</div>\n"; // / link
            }

        //text += i18n("<b>Unread articles:</b> %1").arg(node->unread());
            text += "</div>"; // /body

            return true;
        }

        virtual bool visitFolder(Folder* node)
        {
            text = QString("<div class=\"headerbox\" dir=\"%1\">\n").arg(QApplication::isRightToLeft() ? "rtl" : "ltr");
            text += QString("<div class=\"headertitle\" dir=\"%1\">%2").arg(Utils::directionOf(Utils::stripTags(node->title())), node->title());
            if(node->unread() == 0)
                text += i18n(" (no unread articles)");
            else
                text += i18np(" (1 unread article)", " (%1 unread articles)", node->unread());
            text += QString("</div>\n");
            text += "</div>\n"; // /headerbox

            return true;
        }

        QString formatSummary(TreeNode* node)
        {
            text.clear();
            visit(node);
            return text;
        }

        QString text;
        DefaultNormalViewFormatter* parent;
};

QString DefaultNormalViewFormatter::formatArticle(const Article& article, IconOption icon) const
{
    QString text;
    text = QString("<div class=\"headerbox\" dir=\"%1\">\n").arg(QApplication::isRightToLeft() ? "rtl" : "ltr");
    const QString enc = formatEnclosure( *article.enclosure() );

    if (!article.title().isEmpty())
    {
        text += QString("<div class=\"headertitle\" dir=\"%1\">\n").arg(Utils::directionOf(Utils::stripTags(article.title())));
        if (article.link().isValid())
            text += "<a href=\""+article.link().url()+"\">";
        text += article.title().replace('<', "&lt;").replace('>', "&gt;"); // TODO: better leave things escaped in the parser
        if (article.link().isValid())
            text += "</a>";
        text += "</div>\n";
    }
    if (article.pubDate().isValid())
    {
        text += QString("<span class=\"header\" dir=\"%1\">").arg(Utils::directionOf(i18n("Date")));
        text += QString ("%1:").arg(i18n("Date"));
        text += "</span><span class=\"headertext\">";
        text += KGlobal::locale()->formatDateTime(article.pubDate(), KLocale::FancyLongDate) +"</span>\n"; // TODO: might need RTL?
    }
    const QString author = article.authorAsHtml();
    if (!author.isEmpty())
    {
        text += QString("<br/><span class=\"header\" dir=\"%1\">").arg(Utils::directionOf(i18n("Author")));
        text += QString ("%1:").arg(i18n("Author"));
        text += "</span><span class=\"headertext\">";
        text += author+"</span>\n"; // TODO: might need RTL?
    }

    if (!enc.isEmpty())
    {
        text += QString("<br/><span class=\"header\" dir=\"%1\">").arg(Utils::directionOf(i18n("Enclosure")));
        text += QString ("%1:").arg(i18n("Enclosure"));
        text += "</span><span class=\"headertext\">";
        text += enc+"</span>\n"; // TODO: might need RTL?
    }

    text += "</div>\n"; // end headerbox


    if (icon == ShowIcon && article.feed() && !article.feed()->image().isNull())
    {
        const Feed* feed = article.feed();
        QString file = Utils::fileNameForUrl(feed->xmlUrl());
        KUrl u(m_imageDir);
        u.setFileName(file);
        text += QString("<a href=\"%1\"><img class=\"headimage\" src=\"%2.png\"></a>\n").arg(feed->htmlUrl(), u.url());
    }

    const QString content = article.content( Article::DescriptionAsFallback );
    if (!content.isEmpty())
    {
        text += QString("<div dir=\"%1\">").arg(Utils::directionOf(Utils::stripTags(content)) );
        text += "<span class=\"content\">"+content+"</span>";
        text += "</div>";
    }

    text += "<div class=\"body\">";

    if (article.commentsLink().isValid())
    {
        text += "<a class=\"contentlink\" href=\"";
        text += article.commentsLink().url();
        text += "\">" + i18n( "Comments");
        if (article.comments())
        {
            text += " ("+ QString::number(article.comments()) +')';
        }
        text += "</a>";
    }

    if (!enc.isEmpty())
        text += QString("<p><em>%1</em> %2</p>").arg(i18n("Enclosure:")).arg(enc);

    if (article.link().isValid() || (article.guidIsPermaLink() && KUrl(article.guid()).isValid()))
    {
        text += "<p><a class=\"contentlink\" href=\"";
        // in case link isn't valid, fall back to the guid permaLink.
        if (article.link().isValid())
        {
            text += article.link().url();
        }
        else
        {
            text += article.guid();
        }
        text += "\">" + i18n( "Complete Story" ) + "</a></p>";
    }

    text += "</div>";

    return text;
}

QString DefaultNormalViewFormatter::getCss() const
{
    const QPalette & pal = QApplication::palette();

    // from kmail::headerstyle.cpp
    QString css = QString (
            "<style type=\"text/css\">\n"
            "@media screen, print {"
            "body {\n"
            "  font-family: \"%1\" ! important;\n"
            "  font-size: %2 ! important;\n"
            "  color: %3 ! important;\n"
            "  background: %4 ! important;\n"
            "}\n\n")
            .arg( Settings::standardFont(),
                  QString::number(pointsToPixel(Settings::mediumFontSize()))+"px",
                  pal.color( QPalette::Text ).name(),
                  pal.color( QPalette::Base ).name() );
    css += (
            "a {\n"
            + QString("  color: %1 ! important;\n")
            + QString(!Settings::underlineLinks() ? " text-decoration: none ! important;\n" : "")
            +       "}\n\n"
            +".headerbox {\n"
            +"  background: %2 ! important;\n"
            +"  color: %3 ! important;\n"
            +"  border:1px solid #000;\n"
            +"  margin-bottom: 10pt;\n"
            +        "}\n\n")
            .arg( pal.color( QPalette::Link ).name(),
                  pal.color( QPalette::Background ).name(),
                  pal.color( QPalette::Text ).name() );
    css += QString(".headertitle a:link { color: %1 ! important;\n text-decoration: none ! important;\n }\n"
            ".headertitle a:visited { color: %1 ! important;\n text-decoration: none ! important;\n }\n"
            ".headertitle a:hover{ color: %1 ! important;\n text-decoration: none ! important;\n }\n"
            ".headertitle a:active { color: %1 ! important;\n  text-decoration: none ! important;\n }\n" )
            .arg( pal.color( QPalette::HighlightedText ).name() );
    css += QString(
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
                    pal.color( QPalette::Highlight ).name(),
                    pal.color( QPalette::HighlightedText ).name() );

    css += QString(
            "body { clear: none; }\n\n"
            ".content {\n"
            "  display: block;\n"
            "  margin-bottom: 6px;\n"
            "}\n\n"
    // these rules make sure that there is no leading space between the header and the first of the text
            ".content > P:first-child {\n margin-top: 1px; }\n"
            ".content > DIV:first-child {\n margin-top: 1px; }\n"
            ".content > BR:first-child {\n display: none;  }\n"
    //".contentlink {\n display: block; }\n"
            "}\n\n" // @media screen, print
    // Why did we need that, bug #108187?
    //"@media screen { body { overflow: auto; } }\n"
            "\n\n");

    return css;
}

DefaultCombinedViewFormatter::DefaultCombinedViewFormatter(const KUrl& imageDir, QPaintDevice* device ) : ArticleFormatter( device ), m_imageDir(imageDir)
{
}

DefaultNormalViewFormatter::DefaultNormalViewFormatter(const KUrl& imageDir, QPaintDevice* device )
    : ArticleFormatter( device ),
    m_imageDir( imageDir ),
    m_summaryVisitor( new SummaryVisitor( this ) )
{
}

DefaultNormalViewFormatter::~DefaultNormalViewFormatter()
{
    delete m_summaryVisitor;
}

QString DefaultCombinedViewFormatter::formatArticle(const Article& article, IconOption icon) const
{
    QString text;
    const QString enc = formatEnclosure( *article.enclosure() );
    text = QString("<div class=\"headerbox\" dir=\"%1\">\n").arg(QApplication::isRightToLeft() ? "rtl" : "ltr");

    if (!article.title().isEmpty())
    {
        text += QString("<div class=\"headertitle\" dir=\"%1\">\n").arg(Utils::directionOf(Utils::stripTags(article.title())));
        if (article.link().isValid())
            text += "<a href=\""+article.link().url()+"\">";
        text += article.title().replace('<', "&lt;").replace('>', "&gt;"); // TODO: better leave things escaped in the parser
        if (article.link().isValid())
            text += "</a>";
        text += "</div>\n";
    }
    if (article.pubDate().isValid())
    {
        text += QString("<span class=\"header\" dir=\"%1\">").arg(Utils::directionOf(i18n("Date")));
        text += QString ("%1:").arg(i18n("Date"));
        text += "</span><span class=\"headertext\">";
        text += KGlobal::locale()->formatDateTime(article.pubDate(), KLocale::FancyLongDate) + "</span>\n"; // TODO: might need RTL?
    }

    const QString author = article.authorAsHtml();
    if (!author.isEmpty())
    {
        text += QString("<br/><span class=\"header\" dir=\"%1\">").arg(Utils::directionOf(i18n("Author")));
        text += QString ("%1:").arg(i18n("Author"));
        text += "</span><span class=\"headertext\">";
        text += author+"</span>\n"; // TODO: might need RTL?
    }

    if (!enc.isEmpty())
    {
        text += QString("<br/><span class=\"header\" dir=\"%1\">").arg(Utils::directionOf(i18n("Enclosure")));
        text += QString ("%1:").arg(i18n("Enclosure"));
        text += "</span><span class=\"headertext\">";
        text += enc+"</span>\n"; // TODO: might need RTL?
    }

    text += "</div>\n"; // end headerbox

    if (icon == ShowIcon && article.feed() && !article.feed()->image().isNull())
    {
        const Feed* feed = article.feed();
        QString file = Utils::fileNameForUrl(feed->xmlUrl());
        KUrl u(m_imageDir);
        u.setFileName(file);
        text += QString("<a href=\"%1\"><img class=\"headimage\" src=\"%2.png\"></a>\n").arg(feed->htmlUrl(), u.url());
    }


    const QString content = article.content( Article::DescriptionAsFallback );
    if (!content.isEmpty())
    {
        text += QString("<div dir=\"%1\">").arg(Utils::directionOf(Utils::stripTags(content)) );
        text += "<span class=\"content\">"+content+"</span>";
        text += "</div>";
    }

    text += "<div class=\"body\">";

    if (article.commentsLink().isValid())
    {
        text += "<a class=\"contentlink\" href=\"";
        text += article.commentsLink().url();
        text += "\">" + i18n( "Comments");
        if (article.comments())
        {
            text += " ("+ QString::number(article.comments()) +')';
        }
        text += "</a>";
    }


    if (!enc.isEmpty())
        text += QString("<p><em>%1</em> %2</p>").arg(i18n("Enclosure:")).arg(enc);

    if (article.link().isValid() || (article.guidIsPermaLink() && KUrl(article.guid()).isValid()))
    {
        text += "<p><a class=\"contentlink\" href=\"";
        // in case link isn't valid, fall back to the guid permaLink.
        if (article.link().isValid())
        {
            text += article.link().url();
        }
        else
        {
            text += article.guid();
        }
        text += "\">" + i18n( "Complete Story" ) + "</a></p>";
    }

    text += "</div>";
    //kDebug() << text;
    return text;
}

QString DefaultCombinedViewFormatter::getCss() const
{
    const QPalette &pal = QApplication::palette();

    // from kmail::headerstyle.cpp
    QString css = QString (
            "<style type=\"text/css\">\n"
            "@media screen, print {"
            "body {\n"
            "  font-family: \"%1\" ! important;\n"
            "  font-size: %2 ! important;\n"
            "  color: %3 ! important;\n"
            "  background: %4 ! important;\n"
            "}\n\n").arg(Settings::standardFont(),
                         QString::number(pointsToPixel(Settings::mediumFontSize()))+"px",
                         pal.color( QPalette::Text ).name(),
                         pal.color( QPalette::Base ).name() );
    css += (
            "a {\n"
            + QString("  color: %1 ! important;\n")
            + QString(!Settings::underlineLinks() ? " text-decoration: none ! important;\n" : "")
            +       "}\n\n"
            +".headerbox {\n"
            +"  background: %2 ! important;\n"
            +"  color: %3 ! important;\n"
            +"  border:1px solid #000;\n"
            +"  margin-bottom: 10pt;\n"
//    +"  width: 99%;\n"
            +        "}\n\n")
            .arg( pal.color( QPalette::Link ).name(),
                  pal.color( QPalette::Background ).name(),
                  pal.color( QPalette::Text ).name() );

    css += QString(".headertitle a:link { color: %1  ! important; text-decoration: none ! important;\n }\n"
            ".headertitle a:visited { color: %1 ! important; text-decoration: none ! important;\n }\n"
            ".headertitle a:hover{ color: %1 ! important; text-decoration: none ! important;\n }\n"
            ".headertitle a:active { color: %1 ! important; text-decoration: none ! important;\n }\n")
            .arg( pal.color( QPalette::HighlightedText ).name() );
    css += QString(
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
            ".headertext {\n"
            "  text-decoration: none ! important;\n"
            "}\n\n"
            ".headimage {\n"
            "  float: right;\n"
            "  margin-left: 5px;\n"
            "}\n\n").arg( pal.color( QPalette::Highlight ).name(),
                          pal.color( QPalette::HighlightedText ).name() );

    css += QString(
            "body { clear: none; }\n\n"
            ".content {\n"
            "  display: block;\n"
            "  margin-bottom: 6px;\n"
            "}\n\n"
    // these rules make sure that there is no leading space between the header and the first of the text
            ".content > P:first-child {\n margin-top: 1px; }\n"
            ".content > DIV:first-child {\n margin-top: 1px; }\n"
            ".content > BR:first-child {\n display: none;  }\n"
    //".contentlink {\n display: block; }\n"
            "}\n\n" // @media screen, print
    // Why did we need that, bug #108187?
    //"@media screen { body { overflow: auto; } }\n"
            "\n\n");

    return css;
}

QString DefaultNormalViewFormatter::formatSummary(TreeNode* node) const
{
    return m_summaryVisitor->formatSummary(node);
}

QString DefaultCombinedViewFormatter::formatSummary(TreeNode*) const
{
    return QString();
}
