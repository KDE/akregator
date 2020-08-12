/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "openurlrequest.h"
#include "akregatorconfig.h"

using namespace Akregator;

OpenUrlRequest::OpenUrlRequest(const QUrl &url)
    : m_url(url)
    , m_options(None)
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

QUrl OpenUrlRequest::url() const
{
    return m_url;
}

void OpenUrlRequest::setUrl(const QUrl &url)
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
            return NewTab;
        } else {
            return ExternalBrowser;
        }
    }

    return m_options;
}

void OpenUrlRequest::setOptions(OpenUrlRequest::Options options)
{
    m_options = options;
}

QString OpenUrlRequest::debugInfo() const
{
    return QStringLiteral("url=%1 mimeType=%2 newTab=%3 forcesNewWindow=%4 options=%5")
           .arg(m_url.url(), m_args.mimeType()).arg(m_browserArgs.newTab()).arg(m_browserArgs.forcesNewWindow()).arg(m_options);
}

bool OpenUrlRequest::wasHandled() const
{
    return m_wasHandled;
}

void OpenUrlRequest::setWasHandled(bool handled)
{
    m_wasHandled = handled;
}
