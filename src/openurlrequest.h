/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef AKREGATOR_OPENURLREQUEST_H
#define AKREGATOR_OPENURLREQUEST_H

#include <kparts/browserextension.h>

#include <QUrl>

#include "akregator_export.h"

namespace Akregator
{
class AKREGATOR_EXPORT OpenUrlRequest
{
public:
    /**
     * Akregator-specific options specifying how a link should be handled.
     * TODO: check what can be done by overriding KURLArgs flags.
     */
    enum Options {
        None = 0, /**< no explicit options, use default */
        NewTab, /**< open in new tab */
        ExternalBrowser /**< open in external browser */
    };

    explicit OpenUrlRequest(const QUrl &url = QUrl());

    /**
     * the Id of the frame that sent the request */
    int frameId() const;
    void setFrameId(int frameId);

    Q_REQUIRED_RESULT QUrl url() const;
    void setUrl(const QUrl &url);

    Q_REQUIRED_RESULT KParts::OpenUrlArguments args() const;
    void setArgs(const KParts::OpenUrlArguments &args);

    Q_REQUIRED_RESULT KParts::BrowserArguments browserArgs() const;
    void setBrowserArgs(const KParts::BrowserArguments &args);

    Q_REQUIRED_RESULT Options options() const;
    void setOptions(Options options);

    Q_REQUIRED_RESULT bool openInBackground() const;
    void setOpenInBackground(bool background);

    Q_REQUIRED_RESULT QString debugInfo() const;

    Q_REQUIRED_RESULT bool wasHandled() const;
    void setWasHandled(bool handled);

private:
    int m_frameId = -1;
    QUrl m_url;
    KParts::OpenUrlArguments m_args;
    KParts::BrowserArguments m_browserArgs;
    Options m_options;
    bool m_inBackground = false;
    bool m_wasHandled = false;
};
} // namespace Akregator

#endif // AKREGATOR_OPENURLREQUEST_H
