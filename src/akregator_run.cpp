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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <kdebug.h>

#include "viewer.h"
#include "akregator_run.h"

namespace Akregator {


BrowserRun::BrowserRun(QWidget* mainWindow, Viewer* currentViewer, const KURL& url, const KParts::URLArgs& args, OpeningMode mode)
    : KParts::BrowserRun(url, args, 0L, mainWindow, false, false, true)
{
    m_currentViewer = currentViewer;
    m_openingMode = mode;

    if (mode == CURRENT_TAB)
    {
        connect(m_currentViewer, SIGNAL(destroyed()), this, SLOT(slotViewerDeleted()));
    }
    setEnableExternalBrowser(false);
}

BrowserRun::~BrowserRun()
{
    kdDebug() << "BrowserRun::~BrowserRun()" << endl;
}

void BrowserRun::foundMimeType( const QString & type )
{
    if (type=="text/html" ||type=="text/xml" || type=="application/xhtml+xml")
        emit signalOpenInViewer(url(), m_currentViewer, m_openingMode);
    else
        if ( handleNonEmbeddable(type) == KParts::BrowserRun::NotHandled )
            KRun::foundMimeType( type );
}

void BrowserRun::slotViewerDeleted()
{

    // HACK: if the mode is to open the page in the current viewer, we set it to new tab (foreground) if the part gets deleted meanwhile
    m_currentViewer = 0;
    m_openingMode = NEW_TAB_FOREGROUND;
}

} // namespace Akregator

#include "akregator_run.moc"

// vim: set et ts=4 sts=4 sw=4:
