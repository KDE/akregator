/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

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
            void fetchAllFeeds();

        signals:
            void showPart();

        private:
            QPixmap m_defaultIcon;
            QImage m_lightIconImage;
            int m_unread;
    };
}

#endif
