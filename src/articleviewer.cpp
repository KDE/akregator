/***************************************************************************
 *   Copyright (C) 2004 by Sashmit Bhaduri                                 *
 *   smt@vfemail.net                                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "articleviewer.h"
#include "feed.h"

#include <kapplication.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <khtmlview.h>

#include <qdatetime.h>
#include <qvaluelist.h>
#include <qscrollview.h>

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
    : KHTMLPart(parent, name), m_metrics(widget())
{
    generateCSS();
    // to be on a safe side
    setJScriptEnabled(false);
    setJavaEnabled(false);
    setMetaRefreshEnabled(false);
    setPluginsEnabled(false);
    setDNDEnabled(false);
    setAutoloadImages(true);
    setStatusMessagesEnabled(true);

    connect( browserExtension(), SIGNAL(openURLRequestDelayed(const KURL&, const KParts::URLArgs&)),
                           this, SLOT(slotOpenURLRequest(const KURL&, const KParts::URLArgs& )) );
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
    "  text-decoration: none ! important;"
    "}\n\n"
    "#headerbox {\n"
    "  background: %6 ! important;\n"
    "  color: %7 ! important;\n"
    "  border:1px solid #000;\n"
    "  margin-bottom: 10pt;\n"
    "  width: 100%;\n"
    "}\n\n"
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
    "</style>\n")
    .arg(cg.background().light(108).name());

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
        text += a.title()+"</div>\n";
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

void ArticleViewer::show(Feed *f)
{
    QString art, text;

    begin( KURL( "file:"+KGlobal::dirs()->saveLocation("cache", "akregator/Media/") ) );
    write(m_htmlHead);

    ArticleSequence::iterator it;
    for ( it = f->articles.begin(); it != f->articles.end(); ++it )
    {
        // we set f to 0 to not show feed image
        art="<p><div id=\"article\">"+formatArticle(0, *it)+"</div><p>";
        text += art;
        write(art);
    }

    m_currentText = text + "</body></html>";
    write("</body></html>");
    end();
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

   emit urlClicked(url);
}

#include "articleviewer.moc"
