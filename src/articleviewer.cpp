/***************************************************************************
 *   Copyright (C) 2004 by Sashmit Bhaduri                                 *
 *   smt@vfemail.net                                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "articleviewer.h"
#include "viewer.h"
#include "feed.h"

#include <kapplication.h>
#include <klocale.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <khtmlview.h>
#include <krun.h>

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
    : Viewer(parent, name), m_metrics(widget())
{
    generateCSS();
    // to be on a safe side
    /*setJScriptEnabled(false);
    setJavaEnabled(false);
    setMetaRefreshEnabled(false);
    setPluginsEnabled(false);
    setDNDEnabled(false);
    setAutoloadImages(true);
    setStatusMessagesEnabled(true);
*/
}

void ArticleViewer::openDefault()
{
    openURL( ::locate( "data", "akregatorpart/welcome.html" ) );
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
    "}\n\n"
    "a {\n"
    "  color: %5 ! important;\n"
    "  text-decoration: none ! important;\n"
    "}\n\n"
    "#headerbox {\n"
    "  background: %6 ! important;\n"
    "  color: %7 ! important;\n"
    "  border:1px solid #000;\n"
    "  margin-bottom: 10pt;\n"
    "  width: 100%;\n"
    "}\n\n"
    "#headertitle a:link { color: %9  ! important; }\n"
    "#headertitle a:visited { color: %9 ! important; }\n"
    "#headertitle a:hover{ color: %9 ! important; }\n"
    "#headertitle a:active { color: %9 ! important; }\n"
    "#headertitle {\n"
    "  background: %8 ! important;\n"
    "  padding:2px;\n"
    "  color: %9 ! important;\n"
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
    "}\n\n"
    "#content {\n"
    "  clear: none;\n"
    "  overflow: auto;\n"
    "}\n\n")
    .arg(KGlobalSettings::generalFont().family())
    .arg(QString::number( pointsToPixel( m_metrics, KGlobalSettings::generalFont().pointSize()))+"px")
    .arg(cg.text().name())
    .arg(cg.base().name())
    .arg(cg.link().name())
    .arg(cg.background().name())
    .arg(cg.text().name())
    .arg(cg.highlight().name())
    .arg(cg.highlightedText().name());

    m_htmlHead += QString (
    "#article {\n"
    "  overflow: hidden;\n"
    "  border:1px solid #000;\n"
    "  background: %1;\n"
    "  padding: 3px;\n"
    "  padding-right: 6px;}\n\n"
    "#titleanchor {\n"
    "  color: %2 !important;}\n\n"
    "</style>\n")
    .arg(cg.background().light(108).name())
    .arg(cg.text().name());

}

void ArticleViewer::reload()
{
    generateCSS();
    begin( KURL( "file:"+KGlobal::dirs()->saveLocation("cache", "akregator/Media/") ) );
    write(m_htmlHead + m_currentText);
    end();
}

QString ArticleViewer::formatArticle(Feed *f, MyArticle a)
{
    QString text;
    text = QString("<div id=\"headerbox\" dir=\"%1\">\n").arg(QApplication::reverseLayout() ? "rtl" : "ltr");

    if (!a.title().isEmpty())
    {
        text += QString("<div id=\"headertitle\" dir=\"%1\">\n").arg(directionOf(a.title()));
        if (a.link().isValid())
            text += "<a id=\"titleanchor\" href=\""+a.link().url()+"\">";
        text += a.title();
        if (a.link().isValid())
            text += "</a>";
        text += "</div>\n";
    }
    if (a.pubDate().isValid())
    {
        text += QString("<span id=\"header\" dir=\"%1\">").arg(directionOf(i18n("Date")));
        text += QString ("%1:").arg(i18n("Date"));
        text += "</span><span id=\"headertext\">";
        text += KGlobal::locale()->formatDateTime(a.pubDate(), false, false)+"</span>\n"; // TODO: might need RTL?
    }
    text += "</div>\n"; // end headerbox

    if (f && !f->image.isNull())
    {
        QString url=f->xmlUrl;
        text += QString("<a href=\""+f->htmlUrl+"\"><img id=\"headimage\" src=\""+url.replace("/", "_").replace(":", "_")+".png\"></a>\n");
    }

    text += "<div id=\"content\">"+a.description();
    if (a.link().isValid())
    {
        if (!a.description().isNull())
            text += "<p>\n";
        text += "<a href=\"";
        text += a.link().url();
        text += "\">" + i18n( "Full Story" ) + "</a>";
        if (!a.description().isNull())
            text += "<p>\n";
    }
    text += "</div>";
    return text;

}

void ArticleViewer::beginWriting()
{
    begin( KURL( "file:"+KGlobal::dirs()->saveLocation("cache", "akregator/Media/") ) );
    write(m_htmlHead);
}

void ArticleViewer::endWriting()
{
    m_currentText = m_currentText + "</body></html>";
    write("</body></html>");
    end();
}

void ArticleViewer::show(Feed *f, bool writeHeaders)
{
    QString art, text;

    if (writeHeaders)
    {
        begin( KURL( "file:"+KGlobal::dirs()->saveLocation("cache", "akregator/Media/") ) );
        write(m_htmlHead);
    }

    ArticleSequence::iterator it;
    for ( it = f->articles.begin(); it != f->articles.end(); ++it )
    {
        // we set f to 0 to not show feed image
        art="<p><div id=\"article\">"+formatArticle(0, *it)+"</div><p>";
        text += art;
        write(art);
    }

    if (writeHeaders)
    {
        m_currentText = text + "</body></html>";
        write("</body></html>");
        end();
    }
    else
    {
        m_currentText = m_currentText+text;
    }
}

void ArticleViewer::show(Feed *f, MyArticle a)
{
    begin( KURL( "file:"+KGlobal::dirs()->saveLocation("cache", "akregator/Media/") ) );

    QString text=formatArticle(f, a) +"</body></html>";
    m_currentText=text;

    write(m_htmlHead + text);
    end();
}

void ArticleViewer::slotOpenURLRequest(const KURL& url, const KParts::URLArgs& args)
{
   kdDebug() << "ArticleViewer: Open url request: " << url << endl;
   if(args.frameName == "_blank")
      KRun::runURL(url, "text/html", false, false);
   else
      emit urlClicked(url);
}



#include "articleviewer.moc"
