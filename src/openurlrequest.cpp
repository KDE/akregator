/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "openurlrequest.h"

using namespace Akregator;

OpenUrlRequest::OpenUrlRequest(const QUrl &url)
    : m_url(url)
    , m_options(OpenUrlRequest::Options::None)
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

OpenUrlRequest::Options OpenUrlRequest::options() const
{
    return m_options;
}

void OpenUrlRequest::setOptions(OpenUrlRequest::Options options)
{
    m_options = options;
}

QString OpenUrlRequest::debugInfo() const
{
    return QStringLiteral("url=%1 options=%2").arg(m_url.url(), QString::number(static_cast<int>(m_options)));
}

bool OpenUrlRequest::wasHandled() const
{
    return m_wasHandled;
}

void OpenUrlRequest::setWasHandled(bool handled)
{
    m_wasHandled = handled;
}
