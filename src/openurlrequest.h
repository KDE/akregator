/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

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

    [[nodiscard]] QUrl url() const;
    void setUrl(const QUrl &url);

    [[nodiscard]] Options options() const;
    void setOptions(Options options);

    [[nodiscard]] bool openInBackground() const;
    void setOpenInBackground(bool background);

    [[nodiscard]] QString debugInfo() const;

    [[nodiscard]] bool wasHandled() const;
    void setWasHandled(bool handled);

private:
    int m_frameId = -1;
    QUrl m_url;
    Options m_options;
    bool m_inBackground = false;
    bool m_wasHandled = false;
};
} // namespace Akregator
