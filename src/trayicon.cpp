/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "trayicon.h"
#include <qpixmap.h>

using namespace Akregator;

TrayIcon::TrayIcon(QWidget *parent, const char *name)
        : KSystemTray(parent, name)
{
    setPixmap( KSystemTray::loadIcon("akregator") );
}

TrayIcon::~TrayIcon()
{}

#include "trayicon.moc"
