/***************************************************************************
 *   Copyright (C) 2004 by Sashmit Bhaduri                                 *
 *   smt@vfemail.net                                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "pageviewer.h"
#include "viewer.h"
#include "akregator_run.h"

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
    : Viewer(parent, name)
{
}

bool PageViewer::slotOpenURLRequest(const KURL& url, const KParts::URLArgs& args)
{
    aKregatorRun *run= new aKregatorRun(this, (QWidget*)parent(), this, url, args, true);
    return true;
}

void PageViewer::openPage(const KURL&url, const KParts::URLArgs& args, const QString &mimetype)
{
    kdDebug() << "PageViewer: Open url request: " << url << endl;
    if(Viewer::slotOpenURLRequest(url, args)) return;
    openURL(url);
    return;
}

#include "pageviewer.moc"
