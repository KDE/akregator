/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include <klocale.h>

#include "trayicon.h"
#include <qpixmap.h>
#include <qtooltip.h>

using namespace Akregator;

TrayIcon::TrayIcon(QWidget *parent, const char *name)
        : KSystemTray(parent, name)
{
    setPixmap( KSystemTray::loadIcon("akregator") );
    QToolTip::add(this, i18n("aKregator"));
}

TrayIcon::~TrayIcon()
{}

#include "trayicon.moc"
