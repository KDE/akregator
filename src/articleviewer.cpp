/***************************************************************************
 *   Copyright (C) 2004 by Sashmit Bhaduri                                 *
 *   smt@vfemail.net                                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "articleviewer.h"
#include <kapplication.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <qdatetime.h>

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
    m_bodyFont = KGlobalSettings::generalFont();
    // to be on a safe side
    setJScriptEnabled(false);
    setJavaEnabled(false);
    setMetaRefreshEnabled(false);
    setPluginsEnabled(false);
}

void ArticleViewer::openDefault()
{
    openURL( ::locate( "data", "akregatorpart/welcome.html" ) );
}

QString ArticleViewer::htmlHead() const
{
    return
        "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n"
        "<html><head><title></title></head><body>";
}

QString ArticleViewer::cssDefinitions() const
{
    const QColorGroup & cg = QApplication::palette().active();
    return QString (
    "<style type=\"text/css\">"
    "body {\n"
    "  font-family: \"%1\" ! important;\n"
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
    "</style>\n")
    .arg(m_bodyFont.family()).
    arg(QString::number( pointsToPixel( m_metrics, m_bodyFont.pointSize()))+"px").
    arg(cg.text().name()).
    arg(cg.base().name()).
    arg(cg.link().name()).
    arg(cg.background().name()).
    arg(cg.text().name()).
    arg(cg.highlight().name()).
    arg(cg.highlightedText().name());
}


void ArticleViewer::show(Article a)
{
    QString dir = ( QApplication::reverseLayout() ? "rtl" : "ltr" );
    QString headerBoxStr = QString("<div id=\"headerbox\" dir=\"%1\">\n").arg(dir);

    begin( KURL( "file:/tmp/something.html" ) );

    QString text;
    text += htmlHead()+cssDefinitions();
    text += QString("<div id=\"headerbox\" dir=\"%1\">\n").arg(dir);
    text += QString("<div id=\"headertitle\" dir=\"%1\">\n").arg(directionOf(a.title()));
    text += a.title()+"</div>\n";
    if (a.pubDate().isValid())
    {
        text += QString("<span id=\"header\" dir=\"%1\">").arg(directionOf(i18n("Date")));
        text += QString ("%1:").arg(i18n("Date"));
        text += "</span><span id=\"headertext\">";
        text += KGlobal::locale()->formatDateTime(a.pubDate(), false, false)+"</span>\n"; // TODO: might need RTL?
    }
    text += "</div>\n"; // end headerbox

    text += a.description();
    if (a.link().isValid())
    {
        text += "<p><a href=\"";
        text += a.link().url();
        text += "\">Full Story</a></p>";
    }
    text += "</body></html>";
    write(text);
    end();
}

#include "articleviewer.moc"
