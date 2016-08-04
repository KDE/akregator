/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

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

#include "trayicon.h"
#include "akregatorconfig.h"

#include <kwindowsystem.h>
#include "akregator_debug.h"
#include <KLocalizedString>
#include <QMenu>
#include <kiconloader.h>
#include <KColorScheme>

#include <QPainter>
#include <QFontDatabase>

using namespace Akregator;

TrayIcon *TrayIcon::m_instance = 0;

TrayIcon *TrayIcon::getInstance()
{
    return m_instance;
}

void TrayIcon::setInstance(TrayIcon *trayIcon)
{
    m_instance = trayIcon;
}

TrayIcon::TrayIcon(QObject *parent)
    : KStatusNotifierItem(parent),
      m_unread(0)
{
    setToolTipTitle(i18n("Akregator"));
    setToolTipIconByName(i18n("Akregator"));
    setIconByName(QStringLiteral("akregator"));
}

TrayIcon::~TrayIcon()
{
}

void TrayIcon::slotSetUnread(int unread)
{
    m_unread = unread;

    this->setToolTip(QStringLiteral("akregator"), i18n("Akregator"), unread == 0 ? i18n("There are no unread articles")  : i18np("1 unread article", "%1 unread articles", unread));
    setStatus(unread > 0 ? KStatusNotifierItem::Active : KStatusNotifierItem::Passive);

}

void TrayIcon::settingsChanged()
{
    slotSetUnread(m_unread);
}

