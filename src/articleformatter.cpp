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
#include "articleformatter.h"
#include "utils.h"

#include <krss/enclosure.h>
#include <krss/feedvisitor.h>
#include <krss/feedlist.h>
#include <krss/item.h>
#include <krss/netfeed.h>
#include <krss/treenode.h>
#include <krss/treenodevisitor.h>
#include <krss/tag.h>

#include <KDateTime>
#include <KGlobal>
#include <KLocale>

#include <QApplication>
#include <QPaintDevice>
#include <QPalette>
#include <QString>

using namespace boost;
using namespace Akregator;
using namespace KRss;

namespace {
    static QString formatEnclosure( const KRss::Enclosure& enclosure )
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

    static QString formatEnclosures( const QList<KRss::Enclosure>& enclosures ) {
        QStringList list;
        Q_FOREACH( const KRss::Enclosure& i, enclosures )
            list.append( formatEnclosure( i ) );
        return list.join( QLatin1String("<br/>") );
    }

    class ImageLinkVisitor : public ConstFeedVisitor {
    public:
        explicit ImageLinkVisitor( const KUrl& imgDir ) : imageDir( imgDir ) {}

        void visitNetFeed( const shared_ptr<const NetFeed>& f ) {
            const QString file = Utils::fileNameForUrl( f->xmlUrl() );
            KUrl u( imageDir );
            u.setFileName(file);
            link = QString("<a href=\"%1\"><img class=\"headimage\" src=\"%2.png\"></a>\n").arg(f->htmlUrl(), u.url());
        }

        QString getImageLink( const shared_ptr<const Feed>& f ) {
            link.clear();
            if ( f )
                f->accept( this );
            return link;
        }

        QString link;
        const KUrl imageDir;
    };
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

class DefaultNormalViewFormatter::SummaryFeedVisitor : public KRss::ConstFeedVisitor {
private:
    const DefaultNormalViewFormatter* const q;

public:
    QString text;

    explicit SummaryFeedVisitor( const DefaultNormalViewFormatter* qq ) : q( qq ) {}

    void visitNetFeed( const shared_ptr<const NetFeed>& node ) {
        text.clear();
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
            KUrl u(q->m_imageDir);
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
    }
};

class DefaultNormalViewFormatter::SummaryVisitor : public KRss::TreeNodeVisitor
{
    public:
        explicit SummaryVisitor(const DefaultNormalViewFormatter* p) : parent(p), feedList() {}

        /* reimp */ void visit( const shared_ptr<KRss::RootNode>& node ) {
        }

        /* reimp */ void visit( const shared_ptr<KRss::FeedNode>& feedNode ) {
            if ( !feedList )
                return;
            const shared_ptr<const KRss::Feed> node = feedList->constFeedById( feedNode->feedId() );
            if ( !node )
                return;
            SummaryFeedVisitor v( parent );
            node->accept( &v );
            text = v.text;
        }

        /* reimp */ void visit( const shared_ptr<KRss::TagNode>& node ) {
            const KRss::Tag tag = node->tag();
            text = QString("<div class=\"headerbox\" dir=\"%1\">\n").arg(QApplication::isRightToLeft() ? "rtl" : "ltr");
            text += QString("<div class=\"headertitle\" dir=\"%1\">%2").arg(Utils::directionOf(Utils::stripTags(tag.label())), tag.label());
#ifdef KRSS_PORT_DISABLED
            if(node->unread() == 0)
                text += i18n(" (no unread articles)");
            else
                text += i18np(" (1 unread article)", " (%1 unread articles)", node->unread());
#else
            kWarning() << "Code temporarily disabled (Akonadi port)";
#endif //KRSS_PORT_DISABLED
            text += QString("</div>\n");
            text += "</div>\n"; // /headerbox
        }

        QString formatSummary( const shared_ptr<const KRss::FeedList>& fl, const shared_ptr<KRss::TreeNode>& node )
        {
            feedList = fl;
            text.clear();
            node->accept( this );
            return text;
        }

        QString text;
        const DefaultNormalViewFormatter* const parent;
        shared_ptr<const KRss::FeedList> feedList;
};

QString DefaultNormalViewFormatter::formatItem( const boost::shared_ptr<const KRss::FeedList>& fl, const KRss::Item& item, IconOption icon) const
{
    QString text;
    text = QString("<div class=\"headerbox\" dir=\"%1\">\n").arg(QApplication::isRightToLeft() ? "rtl" : "ltr");
    const QString enc = formatEnclosures( item.enclosures() );

    const KUrl link( item.link() );
    if (!item.title().isEmpty())
    {
        text += QString("<div class=\"headertitle\" dir=\"%1\">\n").arg(Utils::directionOf(Utils::stripTags(item.title())));
        if (link.isValid())
            text += "<a href=\""+link.url()+"\">";
        text += item.title().replace('<', "&lt;").replace('>', "&gt;"); // TODO: better leave things escaped in the parser
        if (link.isValid())
            text += "</a>";
        text += "</div>\n";
    }
    if (item.dateUpdated().isValid())
    {
        text += QString("<span class=\"header\" dir=\"%1\">").arg(Utils::directionOf(i18n("Date")));
        text += QString ("%1:").arg(i18n("Date"));
        text += "</span><span class=\"headertext\">";
        text += KGlobal::locale()->formatDateTime(item.dateUpdated(), KLocale::FancyLongDate) +"</span>\n"; // TODO: might need RTL?
    }

#ifdef KRSS_PORT_DISABLED
    const QString author = item.authorAsHtml();
    if (!author.isEmpty())
    {
        text += QString("<br/><span class=\"header\" dir=\"%1\">").arg(Utils::directionOf(i18n("Author")));
        text += QString ("%1:").arg(i18n("Author"));
        text += "</span><span class=\"headertext\">";
        text += author+"</span>\n"; // TODO: might need RTL?
    }
#else
    kWarning() << "Code temporarily disabled (Akonadi port)";
#endif //KRSS_PORT_DISABLED

    if (!enc.isEmpty())
    {
        text += QString("<br/><span class=\"header\" dir=\"%1\">").arg(Utils::directionOf(i18n("Enclosure")));
        text += QString ("%1:").arg(i18n("Enclosure"));
        text += "</span><span class=\"headertext\">";
        text += enc+"</span>\n"; // TODO: might need RTL?
    }

    text += "</div>\n"; // end headerbox

    if (icon == ShowIcon && fl )
    {
        const shared_ptr<const Feed> f = fl->constFeedById( item.sourceFeedId() );
        text += ImageLinkVisitor( m_imageDir ).getImageLink( f );
    }

    const QString content = item.content();

    if (!content.isEmpty())
    {
        text += QString("<div dir=\"%1\">").arg(Utils::directionOf(Utils::stripTags(content)) );
        text += "<span class=\"content\">"+content+"</span>";
        text += "</div>";
    }

    text += "<div class=\"body\">";

    if (KUrl( item.commentsLink() ).isValid())
    {
        text += "<a class=\"contentlink\" href=\"";
        text += KUrl( item.commentsLink() ).url();
        text += "\">" + i18n( "Comments");
        if ( item.commentsCount() > 0 )
            text += " ("+ QString::number(item.commentsCount()) +')';
        text += "</a>";
    }

    if (!enc.isEmpty())
        text += QString("<p><em>%1</em> %2</p>").arg(i18n("Enclosure:")).arg(enc);

    if (link.isValid())
        text += QString( "<p><a class=\"contentlink\" href=\"%1\">%2</a></p>" ).arg( link.url(), i18n( "Complete Story" ) );

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
    m_imageDir( imageDir )
{
}

DefaultNormalViewFormatter::~DefaultNormalViewFormatter()
{
}

QString DefaultCombinedViewFormatter::formatItem( const shared_ptr<const KRss::FeedList>& fl, const KRss::Item& item, IconOption icon ) const
{
    QString text;
    const QString enc = formatEnclosures( item.enclosures() );
    text = QString("<div class=\"headerbox\" dir=\"%1\">\n").arg(QApplication::isRightToLeft() ? "rtl" : "ltr");

    const KUrl link( item.link() );
    if (!item.title().isEmpty())
    {
        text += QString("<div class=\"headertitle\" dir=\"%1\">\n").arg(Utils::directionOf(Utils::stripTags(item.title())));
        if (link.isValid())
            text += "<a href=\""+link.url()+"\">";
        text += item.title().replace('<', "&lt;").replace('>', "&gt;"); // TODO: better leave things escaped in the parser
        if (link.isValid())
            text += "</a>";
        text += "</div>\n";
    }
    if (item.datePublished().isValid())
    {
        text += QString("<span class=\"header\" dir=\"%1\">").arg(Utils::directionOf(i18n("Date")));
        text += QString ("%1:").arg(i18n("Date"));
        text += "</span><span class=\"headertext\">";
        text += KGlobal::locale()->formatDateTime(item.datePublished(), KLocale::FancyLongDate) + "</span>\n"; // TODO: might need RTL?
    }

#ifdef KRSS_PORT_DISABLED
    const QString author = item.authorAsHtml();
    if (!author.isEmpty())
    {
        text += QString("<br/><span class=\"header\" dir=\"%1\">").arg(Utils::directionOf(i18n("Author")));
        text += QString ("%1:").arg(i18n("Author"));
        text += "</span><span class=\"headertext\">";
        text += author+"</span>\n"; // TODO: might need RTL?
    }
#else
    kWarning() << "Code temporarily disabled (Akonadi port)";
#endif //KRSS_PORT_DISABLED
    if (!enc.isEmpty())
    {
        text += QString("<br/><span class=\"header\" dir=\"%1\">").arg(Utils::directionOf(i18n("Enclosure")));
        text += QString ("%1:").arg(i18n("Enclosure"));
        text += "</span><span class=\"headertext\">";
        text += enc+"</span>\n"; // TODO: might need RTL?
    }

    text += "</div>\n"; // end headerbox

    if (icon == ShowIcon && fl )
    {
        const shared_ptr<const Feed> f = fl->constFeedById( item.sourceFeedId() );
        text += ImageLinkVisitor( m_imageDir ).getImageLink( f );
    }

    const QString content = item.content();
    if (!content.isEmpty())
    {
        text += QString("<div dir=\"%1\">").arg(Utils::directionOf(Utils::stripTags(content)) );
        text += "<span class=\"content\">"+content+"</span>";
        text += "</div>";
    }

    text += "<div class=\"body\">";

    if ( KUrl(item.commentsLink()).isValid())
    {
        text += "<a class=\"contentlink\" href=\"";
        text += KUrl( item.commentsLink() ).url();
        text += "\">" + i18n( "Comments");
        if ( item.commentsCount() > 0 )
            text += " ("+ QString::number(item.commentsCount()) +')';
        text += "</a>";
    }


    if (!enc.isEmpty())
        text += QString("<p><em>%1</em> %2</p>").arg(i18n("Enclosure:")).arg(enc);

    if (link.isValid())
        text += QString( "<p><a class=\"contentlink\" href=\"%1\">%2</a></p>" ).arg( link.url(), i18n( "Complete Story" ) );

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

QString DefaultNormalViewFormatter::formatSummary( const shared_ptr<const KRss::FeedList>& fl, const shared_ptr<KRss::TreeNode>& node ) const
{
    SummaryVisitor v( this );
    return v.formatSummary( fl, node );
}

QString DefaultCombinedViewFormatter::formatSummary( const shared_ptr<const KRss::FeedList>& fl, const shared_ptr<KRss::TreeNode>& node ) const
{
    return QString();
}
