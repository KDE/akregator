/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

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

#ifndef AKREGATORNOTIFICATIONMANAGER_H
#define AKREGATORNOTIFICATIONMANAGER_H

#include <qobject.h>

#include "myarticle.h"

namespace Akregator 
{

    /** this class collects notification requests (new articles etc.) and processes them using KNotify.  */
class NotificationManager : public QObject
{
    Q_OBJECT
    public:
        /** singleton instance of notification manager */
        static NotificationManager* self();
        
        ~NotificationManager();

        /** the widget used for notification, normally either the mainwindow or the tray icon */
        void setWidget(QWidget* widget);
        
    public slots:

        /** notifies an article. Note that articles are not notified separately, but
        "collected" and notified all together */
        void slotNotifyArticle(const MyArticle& article);

        /** notifies the addition of feeds (used when added via DCOP or command line) */
        void slotNotifyFeeds(const QStringList& feeds);
        
    protected:
        
        void doNotify();
        
    protected slots:

        void slotIntervalCheck();
        
    private:
        NotificationManager();
        NotificationManager(const NotificationManager&) : QObject(){}
        
        uint m_checkInterval;
        uint m_intervalsLapsed;
        uint m_maxIntervals;
        uint m_maxArticles;
        bool m_running;
        bool m_addedInLastInterval;
        QWidget* m_widget;
        
        QValueList<MyArticle> m_articles;

        static NotificationManager* m_self;
};

} // namespace

#endif
