/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "actions.h"

#include <QIcon>
#include <KLocalizedString>
#include <QUrl>

#include <QAction>

QAction *Akregator::createOpenLinkInNewTabAction(const QUrl &url, QObject *parent)
{
    QAction *action = new QAction(QIcon::fromTheme(QStringLiteral("tab-new")), i18n("Open Link in New &Tab"), parent);
    action->setData(url);
    return action;
}

QAction *Akregator::createOpenLinkInExternalBrowserAction(const QUrl &url, QObject *parent)
{
    QAction *action = new QAction(QIcon::fromTheme(QStringLiteral("window-new")), i18n("Open Link in External &Browser"), parent);
    action->setData(url);
    return action;
}
