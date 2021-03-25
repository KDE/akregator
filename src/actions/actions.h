/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "akregator_export.h"
#include <QObject>
class QAction;
class QObject;

class QUrl;

namespace Akregator
{
AKREGATOR_EXPORT QAction *createOpenLinkInNewTabAction(const QUrl &url, QObject *parent = nullptr);
AKREGATOR_EXPORT QAction *createOpenLinkInExternalBrowserAction(const QUrl &url, QObject *parent = nullptr);
}

