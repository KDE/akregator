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

#include <kdebug.h>

#include "viewer.h"
#include "akregator_run.h"

using namespace Akregator;


BrowserRun::BrowserRun(Viewer *viewer, QWidget *parent, KParts::ReadOnlyPart *part, const KURL & url, const KParts::URLArgs &args)
    : KParts::BrowserRun(url, args, part, parent, false, true)
{
    m_viewer=viewer;
    connect(m_viewer, SIGNAL(destroyed()), this, SLOT(killMyself()));
}

BrowserRun::~BrowserRun()
{
    kdDebug() << "BrowserRun::~BrowserRun()" << endl;
}

void BrowserRun::foundMimeType( const QString & type )
{
    if (type=="text/html" ||type=="text/xml" || type=="application/xhtml+xml"  )
        m_viewer->openPage(url());
    else
        handleNonEmbeddable(type);
}

void BrowserRun::killMyself()
{
    kdDebug() << "BrowserRun::killMyself()" << endl;
    delete this;
}

#include "akregator_run.moc"

// vim: set et ts=4 sts=4 sw=4:
