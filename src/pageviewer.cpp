/***************************************************************************
 *   Copyright (C) 2004 by Sashmit Bhaduri                                 *
 *   smt@vfemail.net                                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "pageviewer.h"

#include <kapplication.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <khtmlview.h>

#include <qdatetime.h>
#include <qvaluelist.h>
#include <qscrollview.h>

using namespace Akregator;

PageViewer::PageViewer(QWidget *parent, const char *name)
    : KHTMLPart(parent, name)
{
    // to be on a safe side
    setJScriptEnabled(false);
    setJavaEnabled(true);
    setMetaRefreshEnabled(true);
    setPluginsEnabled(true);
    setDNDEnabled(false);
    setAutoloadImages(true);
    setStatusMessagesEnabled(true);

    connect( browserExtension(), SIGNAL(openURLRequestDelayed(const KURL&, const KParts::URLArgs&)),
                           this, SLOT(slotOpenURLRequest(const KURL&, const KParts::URLArgs& )) );
}

void PageViewer::slotOpenURLRequest(const KURL& url, const KParts::URLArgs& args)
{
   kdDebug() << "PageViewer: Open url request: " << url << endl;
   openURL(url);
   //emit urlClicked(url);
}

#include "pageviewer.moc"
