/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "trayicon.h"

#include <KLocalizedString>

using namespace Akregator;

TrayIcon *TrayIcon::m_instance = nullptr;

TrayIcon *TrayIcon::getInstance()
{
    return m_instance;
}

void TrayIcon::setInstance(TrayIcon *trayIcon)
{
    m_instance = trayIcon;
}

bool TrayIcon::isEnabled() const
{
    return mEnabled;
}

void TrayIcon::setEnabled(bool enabled)
{
    mEnabled = enabled;
}

TrayIcon::TrayIcon(QObject *parent)
    : KStatusNotifierItem(parent)
{
    setToolTipTitle(i18n("Akregator"));
    setToolTipIconByName(i18n("Akregator"));
    setIconByName(QStringLiteral("akregator-symbolic"));
}

TrayIcon::~TrayIcon() = default;

void TrayIcon::slotSetUnread(int unread)
{
    if (mEnabled && m_unread != unread) {
        m_unread = unread;

        setToolTip(QStringLiteral("akregator"),
                   i18n("Akregator"),
                   unread == 0 ? i18n("There are no unread articles") : i18np("1 unread article", "%1 unread articles", unread));
        setStatus(unread > 0 ? KStatusNotifierItem::Active : KStatusNotifierItem::Passive);
    }
}

void TrayIcon::settingsChanged()
{
    slotSetUnread(m_unread);
}

#include "moc_trayicon.cpp"
