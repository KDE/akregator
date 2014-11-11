/*
    This file is part of Akregator.

    Copyright (C) 2006 Frank Osterfeld <osterfeld@kde.org>

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

#include "openurlrequest.h"
#include "akregatorconfig.h"

namespace Akregator
{

OpenUrlRequest::OpenUrlRequest(const KUrl &url) : m_frameId(-1), m_url(url),
    m_options(None), m_part(), m_inBackground(false), m_wasHandled(false)
{
}

int OpenUrlRequest::frameId() const
{
    return m_frameId;
}

void OpenUrlRequest::setFrameId(int frameId)
{
    m_frameId = frameId;
}

KUrl OpenUrlRequest::url() const
{
    return m_url;
}

void OpenUrlRequest::setUrl(const KUrl &url)
{
    m_url = url;
}
bool OpenUrlRequest::openInBackground() const
{
    return m_inBackground;
}

void OpenUrlRequest::setOpenInBackground(bool background)
{
    m_inBackground = background;
}

KParts::OpenUrlArguments OpenUrlRequest::args() const
{
    return m_args;
}

void OpenUrlRequest::setArgs(const KParts::OpenUrlArguments &args)
{
    //m_hasArgs = true;
    m_args = args;
}

KParts::BrowserArguments OpenUrlRequest::browserArgs() const
{
    return m_browserArgs;
}

void OpenUrlRequest::setBrowserArgs(const KParts::BrowserArguments &args)
{
    m_browserArgs = args;
}

OpenUrlRequest::Options OpenUrlRequest::options() const
{
    if (m_options == None && m_browserArgs.frameName == QLatin1String("_blank")) {
        if (Settings::newWindowInTab()) {
            return (NewTab);
        } else {
            return (ExternalBrowser);
        }
    }

    return m_options;
}

void OpenUrlRequest::setOptions(OpenUrlRequest::Options options)
{
    m_options = options;
}

void OpenUrlRequest::setPart(KParts::ReadOnlyPart *part)
{
    m_part = part;
}

KParts::ReadOnlyPart *OpenUrlRequest::part() const
{
    return m_part;
}

QString OpenUrlRequest::debugInfo() const
{
    return QString::fromLatin1("url=%1 mimeType=%2 newTab=%3 forcesNewWindow=%4 options=%5")
           .arg(m_url.url())
           .arg(m_args.mimeType())
           .arg(m_browserArgs.newTab())
           .arg(m_browserArgs.forcesNewWindow())
           .arg(m_options);
}

bool OpenUrlRequest::wasHandled() const
{
    return m_wasHandled;
}

void OpenUrlRequest::setWasHandled(bool handled)
{
    m_wasHandled = handled;
}

} // namespace Akregator
