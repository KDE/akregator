/*
    This file is part of Akregator.

    Copyright (C) 2004 Sashmit Bhaduri <smt@vfemail.net>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qdatetime.h>
#include <qevent.h>
#include <qscrollview.h>
#include <qvaluelist.h>

#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <khtmlview.h>
#include <klocale.h>
#include <kprocess.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <kshell.h>

#include <libkdepim/kfileio.h>

#include "aboutdata.h"
#include "akregator_run.h"
#include "akregatorconfig.h"
#include "articleviewer.h"
#include "feed.h"
#include "feedgroup.h"
#include "myarticle.h"
#include "treenode.h"

using namespace Akregator;

// from kmail::headerstyle.cpp
static inline QString directionOf(const QString &str)
{
    return str.isRightToLeft() ? "rtl" : "ltr" ;
}

static inline QString stripTags(const QString& str)
{
    return QString(str).replace(QRegExp("<[^>]*>"), "");
}

ArticleViewer::ArticleViewer(QWidget *parent, const char *name)
    : Viewer(parent, name), m_htmlHead(), m_htmlFooter(), m_currentText(), m_node(0), m_viewMode(NormalView)
{
    setXMLFile(locate("data", "akregator/articleviewer.rc"), true);
    generateCSS();
    
    new KAction( i18n("&Scroll Up"), QString::null, "Up", this, SLOT(slotScrollUp()), actionCollection(), "articleviewer_scroll_up" );
    new KAction( i18n("&Scroll Down"), QString::null, "Down", this, SLOT(slotScrollDown()), actionCollection(), "articleviewer_scroll_down" );
    
    connect(this, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()));

    connect(kapp, SIGNAL(kdisplayPaletteChanged()), this, SLOT(slotPaletteOrFontChanged()) );
    connect(kapp, SIGNAL(kdisplayFontChanged()), this, SLOT(slotPaletteOrFontChanged()) );
    
    m_imageDir.setPath(KGlobal::dirs()->saveLocation("cache", "akregator/Media/"));
    m_htmlFooter = "</body></html>";
}

void ArticleViewer::generateCSS()
{
    const QColorGroup & cg = QApplication::palette().active();
    
    m_htmlHead=QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n"
                        "<html><head><title>.</title>");

    // from kmail::headerstyle.cpp
    m_htmlHead += QString (
            "<style type=\"text/css\">\n"
            "@media screen, print {"
            "body {\n"
            "  font-family: \"%1\" ! important;\n"
            "  font-size: %2 ! important;\n"
            "  color: %3 ! important;\n"
            "  background: %4 ! important;\n"
            "}\n\n").arg(Settings::standardFont())
            .arg(QString::number(pointsToPixel(Settings::mediumFontSize()))+"px")
            .arg(cg.text().name())
            .arg(cg.base().name());
    m_htmlHead += (
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
            .arg(cg.link().name())
            .arg(cg.background().name())
            .arg(cg.text().name());

    m_htmlHead += QString(".headertitle a:link { color: %1  ! important; }\n"
    ".headertitle a:visited { color: %2 ! important; }\n"
    ".headertitle a:hover{ color: %3 ! important; }\n"
            ".headertitle a:active { color: %4 ! important; }\n")
            .arg(cg.highlightedText().name())
            .arg(cg.highlightedText().name())
            .arg(cg.highlightedText().name())
            .arg(cg.highlightedText().name());
    m_htmlHead += QString(
    ".headertitle {\n"
    "  background: %1 ! important;\n"
    "  padding:2px;\n"
    "  color: %2 ! important;\n"
    "  font-weight: bold;\n"
    "}\n\n"
    ".header {\n"
    "  font-weight: bold;\n"
    "  padding:2px;\n"
    "  margin-right: 5px;\n"
    "}\n\n"
    ".headertext {\n"
    "}\n\n"
    ".headimage {\n"
    "  float: right;\n"
    "  margin-left: 5px;\n"
            "}\n\n").arg(cg.highlight().name())
            .arg(cg.highlightedText().name());
    
    m_htmlHead += QString(
    "body { clear: none; }\n\n"
    ".content {\n"
    "  display: block;\n"
    "  margin-bottom: 6px;\n"
            "}\n\n"
    // these rules make sure that there is no leading space between the header and the first of the text
    ".content > P:first-child {\n margin-top: 1px; }\n"
    ".content > DIV:first-child {\n margin-top: 1px; }\n"
    ".content > BR:first-child {\n display: none;  }\n"
    ".contentlink {\n display: block; }\n"
    "}\n\n" // @media screen, print
    "@media screen { body { overflow: auto; } }\n"
    "\n\n");
     
   
    m_htmlHead += "</style></head><body>";
    //kdDebug() << m_htmlHead << endl;
}

void ArticleViewer::reload()
{
    beginWriting();
    write(m_currentText);
    endWriting();
}

void ArticleViewer::displayAboutPage()
{
    QString location = locate("data", "akregator/about/main.html");
    QString content = KPIM::kFileToString(location);
    content = content.arg( locate( "data", "libkdepim/about/kde_infopage.css" ) );
    if ( kapp->reverseLayout() )
        content = content.arg( "@import \"%1\";" ).arg( locate( "data", "libkdepim/about/kde_infopage_rtl.css" ) );
    else
        content = content.arg( "" );

    begin(KURL( location ));
    QString info =
            i18n("%1: Akregator version; %2: help:// URL; %3: homepage URL; "
            "--- end of comment ---",
    "<h2 style='margin-top: 0px;'>Welcome to Akregator %1</h2>"
            "<p>Akregator is an RSS feed aggregator for the K Desktop Environment. "
            "Feed aggregators provide a convenient way to browse different kinds of "
            "content, including news, blogs, and other content from online sites. "
            "Instead of checking all your favorite web sites manually for updates, "
            "Akregator collects the content for you.</p>"
            "<p>For more information about using Akregator, check the "
            "<a href=\"%3\">Akregator website</a> as well as the <a href=\"%2\">Akregator handbook</a>.</p>"
            "<p>We hope that you will enjoy Akregator.</p>\n"
            "<p>Thank you,</p>\n"
            "<p style='margin-bottom: 0px'>&nbsp; &nbsp; The Akregator Team</p>")
            .arg(AKREGATOR_VERSION) // Akregator version
            .arg("help:/akregator/index.html") // Akregator help:// URL
            .arg("http://akregator.sourceforge.net/"); // Akregator homepage URL

    QString fontSize = QString::number( pointsToPixel( Settings::mediumFontSize() ));
    QString appTitle = i18n("Akregator");
    QString catchPhrase = ""; //not enough space for a catch phrase at default window size i18n("Part of the Kontact Suite");
    QString quickDescription = i18n("An RSS feed reader for the K Desktop Environment.");
    write(content.arg(fontSize).arg(appTitle).arg(catchPhrase).arg(quickDescription).arg(info));
    end();
}

QString ArticleViewer::formatArticle(Feed* feed, const MyArticle& article)
{
    QString text;
    text = QString("<div class=\"headerbox\" dir=\"%1\">\n").arg(QApplication::reverseLayout() ? "rtl" : "ltr");

    if (!article.title().isEmpty())
    {
        text += QString("<div class=\"headertitle\" dir=\"%1\">\n").arg(directionOf(stripTags(article.title())));
        if (article.link().isValid())
            text += "<a href=\""+article.link().url()+"\">";
        text += article.title().replace("<", "&lt;").replace(">", "&gt;"); // TODO: better leave things escaped in the parser
        if (article.link().isValid())
            text += "</a>";
        text += "</div>\n";
    }
    if (article.pubDate().isValid())
    {
        text += QString("<span class=\"header\" dir=\"%1\">").arg(directionOf(i18n("Date")));
        text += QString ("%1:").arg(i18n("Date"));
        text += "</span><span class=\"headertext\">";
        text += KGlobal::locale()->formatDateTime(article.pubDate(), false, false)+"</span>\n"; // TODO: might need RTL?
    }
    text += "</div>\n"; // end headerbox

    if (feed && !feed->image().isNull())
    {
        QString url=feed->xmlUrl();
        QString file = url.replace("/", "_").replace(":", "_");
        KURL u(m_imageDir);
        u.setFileName(file);
        text += QString("<a href=\"%1\"><img class=\"headimage\" src=\"%2.png\"></a>\n").arg(feed->htmlUrl()).arg(u.url());
    }

    

    if (!article.description().isEmpty())
    {
        text += QString("<div dir=\"%1\">").arg(directionOf(stripTags(article.description())) );
        text += "<span class=\"content\">"+article.description()+"</span>";
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
            text += " ("+ QString::number(article.comments()) +")";
        }
        text += "</a>";
    }

    if (article.link().isValid() || (article.guidIsPermaLink() && KURL(article.guid()).isValid()))
    {
        text += "<a class=\"contentlink\" href=\"";
        // in case link isn't valid, fall back to the guid permaLink.
        if (article.link().isValid())
        {
            text += article.link().url();
        }
        else
         {
            text += article.guid();
        }
        text += "\">" + i18n( "Complete Story" ) + "</a>";
    }
    text += "</div>";
    //kdDebug() << text << endl;
    return text;

}

void ArticleViewer::renderContent(const QString& text)
{
    m_currentText = text;
    beginWriting();
    //kdDebug() << text << endl;
    write(text);
    endWriting();
}

void ArticleViewer::beginWriting()
{
    view()->setContentsPos(0,0);
    begin();
    write(m_htmlHead);
    //kdDebug() << m_htmlHead << endl;
}

void ArticleViewer::endWriting()
{
    write(m_htmlFooter);
    //kdDebug() << m_htmlFooter << endl;
    end();
}

void ArticleViewer::slotShowSummary(TreeNode* node)
{
    m_viewMode = SummaryView;

    if (!node)
    {
        slotClear();
        return;
    }

    if (node != m_node)
    {
        disconnectFromNode(m_node);
        connectToNode(node);
        m_node = node;
    }
    
    if (node->isGroup())
        showSummary(static_cast<FeedGroup*>(m_node));
    else
        showSummary(static_cast<Feed*>(m_node));
}

void ArticleViewer::showSummary(FeedGroup* group)
{
    if (!group)
        return;
    QString text;
    text = QString("<div class=\"headerbox\" dir=\"%1\">\n").arg(QApplication::reverseLayout() ? "rtl" : "ltr");
    text += QString("<div class=\"headertitle\" dir=\"%1\">%2").arg(directionOf(stripTags(group->title()))).arg(group->title());
    if(group->unread() == 0)
        text += i18n(" (no unread articles)");
    else
        text += i18n(" (1 unread article)", " (%n unread articles)", group->unread());
    text += QString("</div>\n");
    text += "</div>\n"; // /headerbox
    
    renderContent(text);
}

void ArticleViewer::showSummary(Feed *f)
{
    if(!f)
        return;

    QString text;
    text = QString("<div class=\"headerbox\" dir=\"%1\">\n").arg(QApplication::reverseLayout() ? "rtl" : "ltr");

    text += QString("<div class=\"headertitle\" dir=\"%1\">").arg(directionOf(stripTags(f->title())));
    text += f->title();
    if(f->unread() == 0)
        text += i18n(" (no unread articles)");
    else
        text += i18n(" (1 unread article)", " (%n unread articles)", f->unread());
    text += "</div>\n"; // headertitle
    text += "</div>\n"; // /headerbox
    
    if (!f->image().isNull()) // image
    {
        text += QString("<div class=\"body\">");
        QString url=f->xmlUrl();
        QString file = url.replace("/", "_").replace(":", "_");
        KURL u(m_imageDir);
        u.setFileName(file);
        text += QString("<a href=\"%1\"><img class=\"headimage\" src=\"%2.png\"></a>\n").arg(f->htmlUrl()).arg(u.url());
    }
    else text += "<div class=\"body\">";

    
    if( !f->description().isEmpty() )
    {
        text += QString("<div dir=\"%1\">").arg(stripTags(directionOf(f->description())));
        text += i18n("<b>Description:</b> %1<br><br>").arg(f->description());
        text += "</div>\n"; // /description
    }

    if ( !f->htmlUrl().isEmpty() )
    {
        text += QString("<div dir=\"%1\">").arg(directionOf(f->htmlUrl()));
        text += i18n("<b>Homepage:</b> <a href=\"%1\">%2</a>").arg(f->htmlUrl()).arg(f->htmlUrl());
        text += "</div>\n"; // / link
    }   
    
    //text += i18n("<b>Unread articles:</b> %1").arg(f->unread());
    text += "</div>"; // /body

    renderContent(text);
}

void ArticleViewer::slotShowArticle(const MyArticle& article)
{
    m_viewMode = NormalView;
    disconnectFromNode(m_node);
    m_node = 0;
    renderContent( formatArticle(article.feed(), article) );
}

void ArticleViewer::slotSetFilter(const ArticleFilter& textFilter, const ArticleFilter& statusFilter)
{
    if (m_statusFilter == statusFilter && m_textFilter == textFilter)
        return;

    m_textFilter = textFilter;
    m_statusFilter = statusFilter;

    slotUpdateCombinedView();
}

void ArticleViewer::slotUpdateCombinedView()
{
    if (m_viewMode != CombinedView)
        return;

    if (!m_node)
        return slotClear();

    ArticleSequence articles = m_node->articles();
    ArticleSequence::ConstIterator end = articles.end();
    ArticleSequence::ConstIterator it = articles.begin();

    QString text;

    for ( ; it != end; ++it)
        if ( !(*it).isDeleted() && m_textFilter.matches(*it) && m_statusFilter.matches(*it) )
            text += "<p><div class=\"article\">"+formatArticle(0, *it)+"</div><p>";

    renderContent(text);
}

void ArticleViewer::slotClear()
{
    disconnectFromNode(m_node);
    m_node = 0;

    renderContent(QString());
}

void ArticleViewer::slotShowNode(TreeNode* node)
{
    m_viewMode = CombinedView;

    if (node != m_node)
        disconnectFromNode(m_node);

    connectToNode(node);
    
    m_node = node;

    slotUpdateCombinedView();
}

void ArticleViewer::keyPressEvent(QKeyEvent* e)
{
    e->ignore();
}

void ArticleViewer::slotPaletteOrFontChanged()
{
    generateCSS();
    reload();
}

void ArticleViewer::connectToNode(TreeNode* node)
{
    if (node)
    {
        if (m_viewMode == CombinedView)
            connect( node, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotUpdateCombinedView() ) );
        if (m_viewMode == SummaryView)
            connect( node, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotShowSummary(TreeNode*) ) );

        connect( node, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotClear() ) );
    }   
}

void ArticleViewer::disconnectFromNode(TreeNode* node)
{
    if (node)
    {
        disconnect( node, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotUpdateCombinedView() ) );
        disconnect( node, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotClear() ) );
        disconnect( node, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotShowSummary(TreeNode*) ) );
    }
}
            
#include "articleviewer.moc"
// vim: set et ts=4 sts=4 sw=4:
