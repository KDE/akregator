/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef AKREGATORTRAYICON_H
#define AKREGATORTRAYICON_H

#include <ksystemtray.h>

namespace Akregator
{
    class TrayIcon : public KSystemTray
    {
        Q_OBJECT
        public:
            TrayIcon(QWidget *parent = 0, const char *name = 0);
            ~TrayIcon();
    };
}

#endif
