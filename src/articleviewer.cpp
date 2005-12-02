/*
    This file is part of Akregator.

    Copyright (C) 2004 Sashmit Bhaduri <smt@vfemail.net>
                  2005 Frank Osterfeld <frank.osterfeld at kdemail.net>
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
#include <kmessagebox.h>
#include <kio/netaccess.h>

#include <libkdepim/kfileio.h>

#include "aboutdata.h"
#include "akregatorconfig.h"
#include "article.h"
#include "articleviewer.h"
#include "feed.h"
#include "folder.h"
#include "treenode.h"
#include "treenodevisitor.h"
#include "tagnode.h"
#include "utils.h"

namespace Akregator {

// from kmail::headerstyle.cpp
static inline QString directionOf(const QString &str)
{
    return str.isRightToLeft() ? "rtl" : "ltr" ;
}

class ArticleViewer::ShowSummaryVisitor : public TreeNodeVisitor
{
    public:
    
    ShowSummaryVisitor(ArticleViewer* view) : m_view(view) {}
    
    virtual bool visitFeed(Feed* node)
    {

        QString text;
        text = QString("<div class=\"headerbox\" dir=\"%1\">\n").arg(QApplication::isRightToLeft() ? "rtl" : "ltr");
        
        text += QString("<div class=\"headertitle\" dir=\"%1\">").arg(directionOf(Utils::stripTags(node->title())));
        text += node->title();
        if(node->unread() == 0)
            text += i18n(" (no unread articles)");
        else
            text += i18n(" (1 unread article)", " (%n unread articles)", node->unread());
        text += "</div>\n"; // headertitle
        text += "</div>\n"; // /headerbox
        
        if (!node->image().isNull()) // image
        {
            text += QString("<div class=\"body\">");
            QString url=node->xmlUrl();
            QString file = url.replace("/", "_").replace(":", "_");
            KURL u(m_view->m_imageDir);
            u.setFileName(file);
            text += QString("<a href=\"%1\"><img class=\"headimage\" src=\"%2.png\"></a>\n").arg(node->htmlUrl()).arg(u.url());
        }
        else text += "<div class=\"body\">";
        
        
        if( !node->description().isEmpty() )
        {
            text += QString("<div dir=\"%1\">").arg(Utils::stripTags(directionOf(node->description())));
            text += i18n("<b>Description:</b> %1<br><br>").arg(node->description());
            text += "</div>\n"; // /description
        }
        
        if ( !node->htmlUrl().isEmpty() )
        {
            text += QString("<div dir=\"%1\">").arg(directionOf(node->htmlUrl()));
            text += i18n("<b>Homepage:</b> <a href=\"%1\">%2</a>").arg(node->htmlUrl()).arg(node->htmlUrl());
            text += "</div>\n"; // / link
        }
        
        //text += i18n("<b>Unread articles:</b> %1").arg(node->unread());
        text += "</div>"; // /body
        
        m_view->renderContent(text);
        return true;
    }
    
    virtual bool visitFolder(Folder* node)
    {

        QString text;
        text = QString("<div class=\"headerbox\" dir=\"%1\">\n").arg(QApplication::isRightToLeft() ? "rtl" : "ltr");
        text += QString("<div class=\"headertitle\" dir=\"%1\">%2").arg(directionOf(Utils::stripTags(node->title()))).arg(node->title());
        if(node->unread() == 0)
            text += i18n(" (no unread articles)");
        else
            text += i18n(" (1 unread article)", " (%n unread articles)", node->unread());
        text += QString("</div>\n");
        text += "</div>\n"; // /headerbox
    
        m_view->renderContent(text);
        return true;
    }

    virtual bool visitTagNode(TagNode* node)
    {
        QString text;
        text = QString("<div class=\"headerbox\" dir=\"%1\">\n").arg(QApplication::isRightToLeft() ? "rtl" : "ltr");
        text += QString("<div class=\"headertitle\" dir=\"%1\">%2").arg(directionOf(Utils::stripTags(node->title()))).arg(node->title());
        if(node->unread() == 0)
            text += i18n(" (no unread articles)");
        else
            text += i18n(" (1 unread article)", " (%n unread articles)", node->unread());
        text += QString("</div>\n");
        text += "</div>\n"; // /headerbox
    
        m_view->renderContent(text);
        return true;
    }

    private:

    ArticleViewer* m_view;
};

ArticleViewer::ArticleViewer(QWidget *parent, const char *name)
    : Viewer(parent, name), m_htmlFooter(), m_currentText(), m_node(0), m_viewMode(NormalView)
{
    m_showSummaryVisitor = new ShowSummaryVisitor(this);
    setXMLFile(locate("data", "akregator/articleviewer.rc"), true);

    generateNormalModeCSS();
    generateCombinedModeCSS();
    new KAction( i18n("&Scroll Up"), QString::null, "Up", this, SLOT(slotScrollUp()), actionCollection(), "articleviewer_scroll_up" );
    new KAction( i18n("&Scroll Down"), QString::null, "Down", this, SLOT(slotScrollDown()), actionCollection(), "articleviewer_scroll_down" );
    
    connect(this, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()));

    connect(kapp, SIGNAL(kdisplayPaletteChanged()), this, SLOT(slotPaletteOrFontChanged()) );
    connect(kapp, SIGNAL(kdisplayFontChanged()), this, SLOT(slotPaletteOrFontChanged()) );
    
    m_imageDir.setPath(KGlobal::dirs()->saveLocation("cache", "akregator/Media/"));
    m_htmlFooter = "</body></html>";
}

ArticleViewer::~ArticleViewer()
{
    delete m_showSummaryVisitor;
}

void ArticleViewer::generateNormalModeCSS()
{
    const QColorGroup & cg = QApplication::palette().active();
    
    // from kmail::headerstyle.cpp
    m_normalModeCSS = QString (
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
    m_normalModeCSS += (
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

    m_normalModeCSS += QString(".headertitle a:link { color: %1  ! important; }\n"
    ".headertitle a:visited { color: %2 ! important; }\n"
    ".headertitle a:hover{ color: %3 ! important; }\n"
            ".headertitle a:active { color: %4 ! important; }\n")
            .arg(cg.highlightedText().name())
            .arg(cg.highlightedText().name())
            .arg(cg.highlightedText().name())
            .arg(cg.highlightedText().name());
    m_normalModeCSS += QString(
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
    
    m_normalModeCSS += QString(
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
}

void ArticleViewer::generateCombinedModeCSS()
{
    const QColorGroup & cg = QApplication::palette().active();
    
    // from kmail::headerstyle.cpp
    m_combinedModeCSS = QString (
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
    m_combinedModeCSS += (
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

    m_combinedModeCSS += QString(".headertitle a:link { color: %1  ! important; }\n"
    ".headertitle a:visited { color: %2 ! important; }\n"
    ".headertitle a:hover{ color: %3 ! important; }\n"
            ".headertitle a:active { color: %4 ! important; }\n")
            .arg(cg.highlightedText().name())
            .arg(cg.highlightedText().name())
            .arg(cg.highlightedText().name())
            .arg(cg.highlightedText().name());
    m_combinedModeCSS += QString(
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
    
    m_combinedModeCSS += QString(
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
}

void ArticleViewer::reload()
{
    beginWriting();
    write(m_currentText);
    endWriting();
}

bool ArticleViewer::openURL(const KURL& url)
{
    if (!m_article.isNull() && m_article.feed()->loadLinkedWebsite())
    {
        return Viewer::openURL(url);
    }
    else
    {
        reload();
        return true;
    }
}

void ArticleViewer::displayAboutPage()
{
    QString location = locate("data", "akregator/about/main.html");
    QString content = KPIM::kFileToByteArray(location);
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
            "<a href=\"%3\">Akregator website</a>. If you do not want to see this page anymore, <a href=\"config:/disable_introduction\">click here</a>.</p>"
            "<p>We hope that you will enjoy Akregator.</p>\n"
            "<p>Thank you,</p>\n"
            "<p style='margin-bottom: 0px'>&nbsp; &nbsp; The Akregator Team</p>\n")
            .arg(AKREGATOR_VERSION) // Akregator version
            .arg("http://akregator.sourceforge.net/"); // Akregator homepage URL

    QString fontSize = QString::number( pointsToPixel( Settings::mediumFontSize() ));
    QString appTitle = i18n("Akregator");
    QString catchPhrase = ""; //not enough space for a catch phrase at default window size i18n("Part of the Kontact Suite");
    QString quickDescription = i18n("An RSS feed reader for the K Desktop Environment.");
    write(content.arg(fontSize).arg(appTitle).arg(catchPhrase).arg(quickDescription).arg(info));
    end();
}

QString ArticleViewer::formatArticleNormalMode(Feed* feed, const Article& article)
{
    QString text;
    text = QString("<div class=\"headerbox\" dir=\"%1\">\n").arg(QApplication::isRightToLeft() ? "rtl" : "ltr");

    if (!article.title().isEmpty())
    {
        text += QString("<div class=\"headertitle\" dir=\"%1\">\n").arg(directionOf(Utils::stripTags(article.title())));
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
        text += QString("<div dir=\"%1\">").arg(directionOf(Utils::stripTags(article.description())) );
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
    
    if (!article.enclosure().isNull())
    {
        //QString url = article.enclosure().url();
        //QString type = article.enclosure().type();
        //int length = article.enclosure().length();
        //QString lengthStr = KIO::convertSize(length);

        //text += QString("<hr><div><a href=\"%1\">%2</a> (%3, %4)</div>").arg(url).arg(url).arg(lengthStr).arg(type);
    }
    //kdDebug() << text << endl;
    return text;

}

QString ArticleViewer::formatArticleCombinedMode(Feed* feed, const Article& article)
{
    QString text;
    text = QString("<div class=\"headerbox\" dir=\"%1\">\n").arg(QApplication::isRightToLeft() ? "rtl" : "ltr");

    if (!article.title().isEmpty())
    {
        text += QString("<div class=\"headertitle\" dir=\"%1\">\n").arg(directionOf(Utils::stripTags(article.title())));
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
        text += QString("<div dir=\"%1\">").arg(directionOf(Utils::stripTags(article.description())) );
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
    //kdDebug() << text << endl;
    return text;

}

void ArticleViewer::renderContent(const QString& text)
{
    closeURL();
    m_currentText = text;
    beginWriting();
    //kdDebug() << text << endl;
    write(text);
    endWriting();
}

void ArticleViewer::beginWriting()
{
    QString head = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n <html><head><title>.</title>");
    
    if (m_viewMode == CombinedView)
        head += m_combinedModeCSS;
    else
        head += m_normalModeCSS; 

    head += "</style></head><body>";
    view()->setContentsPos(0,0);
    begin(m_link);
    write(head);
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
    m_showSummaryVisitor->visit(node);
}


void ArticleViewer::slotShowArticle(const Article& article)
{
    m_viewMode = NormalView;
    disconnectFromNode(m_node);
    m_article = article;
    m_node = 0;
    m_link = article.link();
    if (article.feed()->loadLinkedWebsite())
        openURL(article.link());
    else
        renderContent( formatArticleNormalMode(article.feed(), article) );
}

void ArticleViewer::slotSetFilter(const Akregator::Filters::ArticleMatcher& textFilter, const Akregator::Filters::ArticleMatcher& statusFilter)
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

    QList<Article> articles = m_node->articles();
    QList<Article>::ConstIterator end = articles.end();
    QList<Article>::ConstIterator it = articles.begin();

    QString text;

    int num = 0;
    QTime spent;
    spent.start();
    
    for ( ; it != end; ++it)
    {
        if ( !(*it).isDeleted() && m_textFilter.matches(*it) && m_statusFilter.matches(*it) )
        {
            text += "<p><div class=\"article\">"+formatArticleCombinedMode(0, *it)+"</div><p>";
            ++num;
        }
    }
    kdDebug() << "Combined view rendering: (" << num << " articles):\n" << "generating HTML: " << spent.elapsed() << "ms " << endl;
    renderContent(text);
    kdDebug() << "HTML rendering: " << spent.elapsed() << "ms" << endl;


}

void ArticleViewer::slotArticlesUpdated(TreeNode* /*node*/, const QList<Article>& /*list*/)
{
    if (m_viewMode == CombinedView)
        slotUpdateCombinedView();
}

void ArticleViewer::slotArticlesAdded(TreeNode* /*node*/, const QList<Article>& /*list*/)
{
}

void ArticleViewer::slotArticlesRemoved(TreeNode* /*node*/, const QList<Article>& /*list*/)
{
}

/* testingtesting :)
void ArticleViewer::slotPopupMenu(KXMLGUIClient*, const QPoint& p, const KURL& kurl, const KParts::URLArgs&, KParts::BrowserExtension::PopupFlags, mode_t)
{
    kdDebug() << m_link << endl;
    kdDebug() << kurl.url() << endl;
}*/


void ArticleViewer::slotClear()
{
    disconnectFromNode(m_node);
    m_node = 0;
    m_article = Article();

    renderContent(QString());
}

void ArticleViewer::slotShowNode(TreeNode* node)
{
    m_viewMode = CombinedView;

    if (node != m_node)
        disconnectFromNode(m_node);

    connectToNode(node);
    
    m_article = Article();
    m_node = node;

    if (node && !node->articles().isEmpty())
        m_link = node->articles().first().link();
    else
        m_link = KURL();

    slotUpdateCombinedView();
}

void ArticleViewer::keyPressEvent(QKeyEvent* e)
{
    e->ignore();
}

void ArticleViewer::urlSelected(const QString &url, int button, int state, const QString& _target, KParts::URLArgs args)
{
    if(url == "config:/disable_introduction") {
        if(KMessageBox::questionYesNo( widget(), i18n("Are you sure you want to disable this introduction page?"), i18n("Disable Introduction Page"), i18n("Disable"), i18n("Keep Enabled") ) == KMessageBox::Yes) {
            KConfig *conf = Settings::self()->config();
            conf->setGroup("General");
            conf->writeEntry("Disable Introduction", "true");
        }
    }
    else
        Viewer::urlSelected(url, button, state, _target, args);
}

void ArticleViewer::slotPaletteOrFontChanged()
{
    generateNormalModeCSS();
    generateCombinedModeCSS();
    reload();
}

void ArticleViewer::connectToNode(TreeNode* node)
{
    if (node)
    {
        if (m_viewMode == CombinedView)
        {
//            connect( node, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotUpdateCombinedView() ) );
            connect( node, SIGNAL(signalArticlesAdded(TreeNode*, const QList<Article>&)), this, SLOT(slotArticlesAdded(TreeNode*, const QList<Article>&)));
            connect( node, SIGNAL(signalArticlesRemoved(TreeNode*, const QList<Article>&)), this, SLOT(slotArticlesRemoved(TreeNode*, const QList<Article>&)));
            connect( node, SIGNAL(signalArticlesUpdated(TreeNode*, const QList<Article>&)), this, SLOT(slotArticlesUpdated(TreeNode*, const QList<Article>&)));
        }
        if (m_viewMode == SummaryView)
            connect( node, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotShowSummary(TreeNode*) ) );

        connect( node, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotClear() ) );
    }   
}

void ArticleViewer::disconnectFromNode(TreeNode* node)
{
    if (node)
    {
//        disconnect( node, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotUpdateCombinedView() ) );
        disconnect( node, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotClear() ) );
        disconnect( node, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotShowSummary(TreeNode*) ) );
        disconnect( node, SIGNAL(signalArticlesAdded(TreeNode*, const QList<Article>&)), this, SLOT(slotArticlesAdded(TreeNode*, const QList<Article>&)));
        disconnect( node, SIGNAL(signalArticlesRemoved(TreeNode*, const QList<Article>&)), this, SLOT(slotArticlesRemoved(TreeNode*, const QList<Article>&)));
        disconnect( node, SIGNAL(signalArticlesUpdated(TreeNode*, const QList<Article>&)), this, SLOT(slotArticlesUpdated(TreeNode*, const QList<Article>&)));

    }
}

}
#include "articleviewer.moc"

