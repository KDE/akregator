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

#include <klocale.h>
#include <knotifyclient.h>
#include <kstaticdeleter.h>
#include <kurl.h>

#include <qlabel.h>
#include <qtimer.h>

#include "feed.h"
#include "notificationmanager.h"

using namespace Akregator;

NotificationManager::NotificationManager() : QObject()
{
    m_intervalsLapsed = 0;
    m_checkInterval = 2000;
    m_maxIntervals = 10;
    m_running = false;
    m_addedInLastInterval = false;
    m_maxArticles = 20;
    m_widget = NULL;
}

NotificationManager::~NotificationManager()
{
    m_self = 0;
}

void NotificationManager::setWidget(QWidget* widget)
{
    m_widget = widget;
}

void NotificationManager::slotNotifyArticle(const MyArticle& article)
{
    m_articles.append(article);
    m_addedInLastInterval = true;
    if (m_articles.count() >= m_maxArticles)
        doNotify();
    else if (!m_running)
    {
        m_running = true;
        QTimer::singleShot(m_checkInterval, this, SLOT(slotIntervalCheck()));
    }
}

void NotificationManager::slotNotifyFeeds(const QStringList& feeds)
{
    if (feeds.count() == 1)
    {
        KNotifyClient::event(m_widget->winId(), "feed_added", i18n("Feed added:\n %1").arg(feeds[0]));
    }
    else if (feeds.count() > 1)
    {
        QString message;
        for (QStringList::ConstIterator it = feeds.begin(); it != feeds.end(); ++it)
            message += *it + "\n";
        KNotifyClient::event(m_widget->winId(), "feed_added", i18n("Feeds added:\n %1").arg(message));
    }
}

void NotificationManager::doNotify()
{
    QString message = "<html><body>";
    QString feedTitle;
    QValueList<MyArticle>::ConstIterator it = m_articles.begin();
    QValueList<MyArticle>::ConstIterator en = m_articles.end();
    for (; it != en; ++it)
    {
        if (feedTitle != (*it).feed()->title())
        {
            feedTitle = (*it).feed()->title();
            message += QString("<p><b>%1:</b></p>").arg(feedTitle);
        }
        message += (*it).title() + "<br>";
    }
    message += "</body></html>";
    KNotifyClient::event(m_widget->winId(), "new_articles", message);

    m_articles.clear();
    m_running = false;
    m_intervalsLapsed = 0;
    m_addedInLastInterval = false;
}

void NotificationManager::slotIntervalCheck()
{
    if (!m_running)
        return;
    m_intervalsLapsed++;
    if (!m_addedInLastInterval || m_articles.count() >= m_maxArticles || m_intervalsLapsed >= m_maxIntervals)
        doNotify();
    else
    {
        m_addedInLastInterval = false;
        QTimer::singleShot(m_checkInterval, this, SLOT(slotIntervalCheck()));
    }
    
}

NotificationManager* NotificationManager::m_self;
static KStaticDeleter<NotificationManager> notificationmanagersd;

NotificationManager* NotificationManager::self()
{
    if (!m_self)
        m_self = notificationmanagersd.setObject(m_self, new NotificationManager);
    return m_self;
}

#include "notificationmanager.moc"
