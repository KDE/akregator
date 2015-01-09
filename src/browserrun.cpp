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

#include <qdebug.h>

namespace Akregator
{

BrowserRun::BrowserRun(const OpenUrlRequest &request, QWidget *parent)
    : KParts::BrowserRun(request.url(), request.args(), request.browserArgs(), Q_NULLPTR, parent, /*removeReferrer=*/false, /*trustedSource=*/false, /*hideErrorDialog=*/true), m_request(request)
{
    setEnableExternalBrowser(false);
    m_window = parent; // remove member and use KRun::window() instead once we can depend on kdelibs >= 4.10
}

BrowserRun::~BrowserRun()
{}

void BrowserRun::foundMimeType(const QString &type)
{
    KParts::OpenUrlArguments args = m_request.args();
    args.setMimeType(type);
    m_request.setArgs(args);
    m_request.setWasHandled(false);

    emit signalFoundMimeType(m_request);

    if (m_request.wasHandled()) {
        setFinished(true);
        return;
    }

    KService::Ptr selectedService;
    if (handleNonEmbeddable(type, &selectedService) == KParts::BrowserRun::NotHandled) {
        if (selectedService) {
            KRun::setPreferredService(selectedService->desktopEntryName());
            KRun::foundMimeType(type);
        } else {
            KRun::displayOpenWithDialog(QList<QUrl>() << url(), m_window, false, suggestedFileName());
            setFinished(true);
        }
    }
}

} // namespace Akregator

