/***************************************************************************
 *   Copyright (C) 2004 by Sashmit Bhaduri                                 *
 *   smt@vfemail.net                                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "articleviewer.h"
#include "viewer.h"
#include "feed.h"
#include "feedgroup.h"
#include "myarticle.h"
#include "treenode.h"
#include "akregatorconfig.h"
#include "akregator_run.h"

#include <kapplication.h>
#include <klocale.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <khtmlview.h>
#include <krun.h>
#include <kprocess.h>
#include <kshell.h>

#include <qdatetime.h>
#include <qvaluelist.h>
#include <qscrollview.h>
#include <qevent.h>


using namespace Akregator;

// from kmail::headerstyle.cpp
static inline QString directionOf(const QString &str)
{
    return str.isRightToLeft() ? "rtl" : "ltr" ;
}

int pointsToPixel(const QPaintDeviceMetrics &metrics, int pointSize)
{
    return ( pointSize * metrics.logicalDpiY() + 36 ) / 72 ;
}

ArticleViewer::ArticleViewer(QWidget *parent, const char *name)
    : Viewer(parent, name), m_htmlHead(), m_htmlFooter(), m_metrics(widget()), m_currentText(), m_node(0), m_viewMode(normalView)
{
    generateCSS();
    connect(kapp, SIGNAL(kdisplayPaletteChanged()), this, SLOT(slotPaletteOrFontChanged()) );
    connect(kapp, SIGNAL(kdisplayFontChanged()), this, SLOT(slotPaletteOrFontChanged()) );
    m_imageDir="file:"+KGlobal::dirs()->saveLocation("cache", "akregator/Media/");
    m_htmlFooter = "</body></html>";
}

void ArticleViewer::openDefault()
{
    QString text= QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n"
                        "<html><head><title></title></head><body>\n");

    text += QString("<h1>%1</h1>\n"
                    "<p>%2</p><ul>\n"
                    "<li>%3</li><ul>\n"
                    "<li><b>%4</b>%5</li>\n"
                    "<li><b>%6</b>%7</li>\n"
                    "<li><b>%8</b>%9</li>")
                    .arg(i18n("Welcome to aKregator"))
                    .arg(i18n("Use the tree to manage your feeds."))
                    .arg(i18n("Right click a folder, such as \"All Feeds\", and choose:"))
                    .arg(i18n("Add Feed..."))
                    .arg(i18n(" to add a new feed to your feed list."))
                    .arg(i18n("New Folder..."))
                    .arg(i18n(" to add a new folder to your list."))
                    .arg(i18n("Edit..."))
                    .arg(i18n(" to edit an existing feed or folder."));

    text += QString("<li><b>%1</b>%2</li>\n"
                    "<li><b>%3</b>%4</li></ul></ul>\n"
                    "<p>%5</p></body></html>")
                    .arg(i18n("Delete"))
                    .arg(i18n(" to remove an existing feed or folder."))
                    .arg(i18n("Fetch"))
                    .arg(i18n(" to update a feed or folder."))
                    .arg(i18n("Click \"Fetch All\" to update all feeds."));

    renderContent(text);
}

void ArticleViewer::generateCSS()
{
    const QColorGroup & cg = QApplication::palette().active();
    m_htmlHead=QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n"
                        "<html><head><title></title></head><body>");
    m_htmlHead += QString (
    "<style type=\"text/css\">\n"
    "body {\n"
    "  font-family: \"%1\" ! important;\n"
// from kmail::headerstyle.cpp
    "  font-size: %2 ! important;\n"
    "  color: %3 ! important;\n"
    "  background: %4 ! important;\n"
            "}\n\n").arg(KGlobalSettings::generalFont().family())
            .arg(QString::number( pointsToPixel( m_metrics, KGlobalSettings::generalFont().pointSize()))+"px")
            .arg(cg.text().name())
            .arg(cg.base().name());
    m_htmlHead += QString(
    "a {\n"
    "  color: %1 ! important;\n"
    "  text-decoration: none ! important;\n"
            "}\n\n"
    "#headerbox {\n"
    "  background: %2 ! important;\n"
    "  color: %3 ! important;\n"
    "  border:1px solid #000;\n"
    "  margin-bottom: 10pt;\n"
    "  width: 100%;\n"
            "}\n\n").arg(cg.link().name()).arg(cg.background().name())
            .arg(cg.text().name());
    m_htmlHead += QString("#headertitle a:link { color: %1  ! important; }\n"
    "#headertitle a:visited { color: %2 ! important; }\n"
    "#headertitle a:hover{ color: %3 ! important; }\n"
            "#headertitle a:active { color: %4 ! important; }\n")
            .arg(cg.highlightedText().name())
            .arg(cg.highlightedText().name())
            .arg(cg.highlightedText().name())
            .arg(cg.highlightedText().name());
    m_htmlHead += QString(
    "#headertitle {\n"
    "  background: %1 ! important;\n"
    "  padding:2px;\n"
    "  color: %2 ! important;\n"
    "  font-weight: bold;\n"
    "}\n\n"
    "#header {\n"
    "  font-weight: bold;\n"
    "  padding:2px;\n"
    "  margin-right: 5px;\n"
    "}\n\n"
    "#headertext {\n"
    "}\n\n"
    "#headimage {\n"
    "  float: right;\n"
    "  margin-left: 5px;\n"
            "}\n\n").arg(cg.highlight().name())
            .arg(cg.highlightedText().name());
    
    m_htmlHead += QString(
    "#body {\n"
    "  clear: none;\n"
    "  overflow: auto;\n"
    "}\n\n"
    "#content {\n"
    "  display: block;\n"
    "  margin-bottom: 6px;\n"
            "}\n\n"
    // these rules make sure that there is no leading space between the header and the first of the text
    "#content > P:first-child {\n margin-top: 1px; }"
    "#content > DIV:first-child {\n margin-top: 1px; }"
    "#content > BR:first-child {\n display: none;  }"
    ".contentlink {\n display: block; }"
    "\n\n");
     
   // "  border:1px solid #000;\n"
    m_htmlHead += QString (
    "#article {\n"
    "  overflow: hidden;\n"
    "  background: %1;\n"
    " border:1px solid %2;\n"
    "  padding: 3px;\n"
    "  padding-right: 6px;}\n\n"
    "#titleanchor {\n"
    "  color: %3 !important;}\n\n"
    "</style>\n")
    .arg(cg.background().light(108).name())
            .arg(cg.text().name()).arg(cg.text().name());

}

void ArticleViewer::reload()
{
    beginWriting();
    write(m_currentText);
    endWriting();
}

QString ArticleViewer::formatArticle(Feed* feed, const MyArticle& article)
{
    QString text;
    text = QString("<div id=\"headerbox\" dir=\"%1\">\n").arg(QApplication::reverseLayout() ? "rtl" : "ltr");

    if (!article.title().isEmpty())
    {
        text += QString("<div id=\"headertitle\" dir=\"%1\">\n").arg(directionOf(article.title()));
        if (article.link().isValid())
            text += "<a id=\"titleanchor\" href=\""+article.link().url()+"\">";
        text += article.title().replace("<","&lt;").replace(">", "&gt;"); // TODO: better leave < and > escaped in the parser
        if (article.link().isValid())
            text += "</a>";
        text += "</div>\n";
    }
    if (article.pubDate().isValid())
    {
        text += QString("<span id=\"header\" dir=\"%1\">").arg(directionOf(i18n("Date")));
        text += QString ("%1:").arg(i18n("Date"));
        text += "</span><span id=\"headertext\">";
        text += KGlobal::locale()->formatDateTime(article.pubDate(), false, false)+"</span>\n"; // TODO: might need RTL?
    }
    text += "</div>\n"; // end headerbox

    if (feed && !feed->image().isNull())
    {
        QString url=feed->xmlUrl();
        text += QString("<a href=\""+feed->htmlUrl()+"\"><img id=\"headimage\" src=\""+m_imageDir+url.replace("/", "_").replace(":", "_")+".png\"></a>\n");
    }

    

    if (!article.description().isEmpty())
    {
        text += QString("<div id=\"body\" dir=\"%1\">").arg(directionOf(article.description()) );
        text += "<span id=\"content\">"+article.description()+"</span>";
        text += "</div>";
    }
    
    text += "<div id=\"body\">";
    
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
    return text;

}

void ArticleViewer::renderContent(const QString& text)
{
    m_currentText = text;
    beginWriting();
    write(text);
    endWriting();
}

void ArticleViewer::beginWriting()
{
    view()->setContentsPos(0,0);
    begin();
    write(m_htmlHead);
}

void ArticleViewer::endWriting()
{
    write(m_htmlFooter);
    end();
}

void ArticleViewer::slotShowSummary(TreeNode* node)
{
    if (!node)
    {
        slotClear();
        return;
    }

    if (node->isGroup())
        showSummary(static_cast<FeedGroup*>(node));
    else
        showSummary(static_cast<Feed*>(node));
}

void ArticleViewer::showSummary(FeedGroup* group)
{
    if (!group)
        return;
    QString text;
    text = QString("<div id=\"headerbox\" dir=\"%1\">\n").arg(QApplication::reverseLayout() ? "rtl" : "ltr");
    text += QString("<div id=\"headertitle\" dir=\"%1\">%2").arg(directionOf(group->title())).arg(group->title());
    if(group->unread() == 0)
        text += i18n(" (no unread articles)");
    else
        text += i18n(" (1 unread article)", " (%n unread articles)", group->unread());
    text += QString("</div>\n");
    text += "</div>\n"; // /headerbox
    
    for( TreeNode *it = group; it != 0; it = it->nextSibling() )
        kdDebug() << "title: " << it->title() << endl;

    renderContent(text);
}

void ArticleViewer::showSummary(Feed *f)
{
    if(!f)
        return;
    
    QString text;
    text = QString("<div id=\"headerbox\" dir=\"%1\">\n").arg(QApplication::reverseLayout() ? "rtl" : "ltr");

    text += QString("<div id=\"headertitle\" dir=\"%1\">").arg(directionOf(f->title()));
    text += f->title();
    if(f->unread() == 0)
        text += i18n(" (no unread articles)");
    else
        text += i18n(" (1 unread article)", " (%n unread articles)", f->unread());
    text += "</div>\n"; // headertitle
    text += "</div>\n"; // /headerbox
    
    if (!f->image().isNull()) // image
    {
  //      text += QString("<div id=\"body\" style=\"height:%1px\">").arg(f->image().height()+10);
          text += QString("<div id=\"body\">");
        QString url=f->xmlUrl();
        text += QString("<a href=\""+f->htmlUrl()+"\"><img id=\"headimage\" src=\""+m_imageDir+url.replace("/", "_").replace(":", "_")+".png\"></a>\n");
    }
    else text += "<div id=\"body\">";

    
    if( !f->description().isEmpty() )
    {
        text += QString("<div dir=\"%1\">").arg(directionOf(f->description()));
        text += i18n("<b>Description:</b> %1<br><br>").arg(f->description());
        text += "</div>\n"; // /description
    }

    if ( !f->htmlUrl().isEmpty() )
    {
        text += QString("<div dir=\"%1\">").arg(directionOf(f->htmlUrl()));
        text += i18n("<b>Homepage:</b> <a href=\"%1\">%2</a>").arg(f->htmlUrl()).arg(f->htmlUrl());
        text += "</div>\n"; // / link
    }   
    /*if(f->language() && !f->language().isEmpty()) {
        // language name code from kttsd
        QString langName;
        QString langFile = locate("locale", QString::fromLatin1("%1/entry.desktop").arg(f->language()));
        if (!langFile.isNull() && !langFile.isEmpty()) {
            KSimpleConfig entry(langFile);
            entry.setGroup("KCM Locale");
            langName = entry.readEntry("Name", QString::null);
        }
       // if(name.isEmpty()) langName = f->language();
            
        text += QString("<div dir=\"%1\">").arg(directionOf(f->language()));
        text += i18n("<b>Language:</b> %1").arg((langName));
        text += "</div>\n"; // /language
    }*/

    //text += i18n("<b>Unread articles:</b> %1").arg(f->unread());
    text += "</div>"; // /body
    
    renderContent(text);
}

void ArticleViewer::slotShowArticle(const MyArticle& article)
{
    m_viewMode = normalView;

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
    if (m_viewMode != combinedView)
        return;

    if (!m_node)
        return slotClear();

    ArticleSequence articles = m_node->articles();
    ArticleSequence::ConstIterator end = articles.end();
    ArticleSequence::ConstIterator it = articles.begin();

    QString text;

    for ( ; it != end; ++it)
        if ( m_textFilter.matches(*it) && m_statusFilter.matches(*it) )
            text += "<p><div id=\"article\">"+formatArticle(0, *it)+"</div><p>";

    renderContent(text);
}

void ArticleViewer::slotClear()
{
    if (m_node)
    {
        disconnect( m_node, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotUpdateCombinedView() ) );
        disconnect( m_node, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotClear() ) );
    }
    m_node = 0;

    renderContent(QString());
}

void ArticleViewer::slotShowNode(TreeNode* node)
{
    m_viewMode = combinedView;

    if (m_node)
    {
        disconnect( m_node, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotUpdateCombinedView() ) );
        disconnect( m_node, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotClear() ) );
    }

    m_node = node;

    if (node)
    {
        connect( node, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotUpdateCombinedView()) );
        connect( node, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotClear()) );
    }
    slotUpdateCombinedView();
}

bool ArticleViewer::slotOpenURLRequest(const KURL& url, const KParts::URLArgs& args)
{
    openPage(url, args, QString::null);
    return true;
}


void ArticleViewer::openPage(const KURL&url, const KParts::URLArgs& args, const QString &)
{
   kdDebug() << "ArticleViewer: Open url request: " << url << endl;
   if( !(Viewer::slotOpenURLRequest(url, args)) )
       emit urlClicked(url);
}

void ArticleViewer::slotOpenLinkInternal()
{
    if(!m_url.isEmpty())
    	emit urlClicked(m_url);
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
#include "articleviewer.moc"
// vim: set et ts=4 sts=4 sw=4:
