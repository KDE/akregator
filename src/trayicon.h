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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef AKREGATORTRAYICON_H
#define AKREGATORTRAYICON_H

#include <ksystemtray.h>

#include <qimage.h>
#include <qpixmap.h>

namespace Akregator
{
    class TrayIcon : public KSystemTray
    {
        Q_OBJECT
        public:
            TrayIcon(QWidget *parent = 0, const char *name = 0);
            ~TrayIcon();
            
            QPixmap takeScreenshot() const;
            virtual void mousePressEvent(QMouseEvent *);
        public slots:
            void settingsChanged();
            void slotSetUnread(int unread);
            void viewButtonClicked();
        
        signals:
            void showPart();

        private:
            QPixmap m_defaultIcon;
            QImage m_lightIconImage;
            int m_unread;
    };
}

#endif
