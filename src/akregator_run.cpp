/***************************************************************************
 *   Copyright (C) 2004 by Sashmit Bhaduri                                 *
 *   smt@vfemail.net                                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include <kdebug.h>

#include "viewer.h"
#include "akregator_run.h"

using namespace Akregator;


BrowserRun::BrowserRun(Viewer *viewer, QWidget *parent, KParts::ReadOnlyPart *part, const KURL & url, const KParts::URLArgs &args, bool shouldEmbed)
    : KParts::BrowserRun(url, args, part, parent, false, true)
{
    m_viewer=viewer;
    m_shouldEmbed=shouldEmbed;
}

BrowserRun::~BrowserRun()
{
}

void BrowserRun::foundMimeType( const QString & type )
{
    if (type=="text/html" ||type=="text/xml" || type=="application/xhtml+xml"  )
    {
        if (m_shouldEmbed)
        {
            m_viewer->open(url());//Page(url(), m_args, type);
            return;
        }
    }
    KParts::BrowserRun::foundMimeType( type );
}

#include "akregator_run.moc"

// vim: set et ts=4 sts=4 sw=4:
