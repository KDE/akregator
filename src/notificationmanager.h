/***************************************************************************
 *   Copyright (C) 2005 by Frank Osterfeld                                 *
 *   frank.osterfeld AT kdemail.net                                        *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

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
