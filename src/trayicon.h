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

class Balloon;
    
namespace Akregator
{
    class TrayIcon : public KSystemTray
    {
        Q_OBJECT
        public:
            TrayIcon(QWidget *parent = 0, const char *name = 0);
            ~TrayIcon();
            void updateUnread(int unread);
            void newArticle(const QString&, const QPixmap&, const QString&);


        private:
            QPixmap m_defaultIcon;
            QImage m_lightIconImage;
            int m_unread;
            Balloon *m_balloon;

        public slots:
            void settingsChanged();       
			void viewButtonClicked();

    };
}

#endif
