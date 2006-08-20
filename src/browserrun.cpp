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

#include "browserrun.h"
#include "openurlrequest.h"
#include "frame.h"

#include <kdebug.h>

namespace Akregator {

BrowserRun::BrowserRun(const OpenURLRequest& request, QWidget* parent)
    : KParts::BrowserRun(request.url(), request.args(), 0L, parent, false, true), m_request(request)
{
    setEnableExternalBrowser(false);
}

BrowserRun::~BrowserRun()
{}

void BrowserRun::foundMimeType(const QString& type)
{
    KParts::URLArgs args = m_request.args();
    args.serviceType = type;
    m_request.setArgs(args);
    
    emit signalFoundMimeType(m_request);
/*
    if (type=="text/html" ||type=="text/xml" || type=="application/xhtml+xml")
        m_viewer->openPage(url());
    else
        if ( handleNonEmbeddable(type) == KParts::BrowserRun::NotHandled )
            KRun::foundMimeType( type );
*/
}

} // namespace Akregator

#include "browserrun.moc"
