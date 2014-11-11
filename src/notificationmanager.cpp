/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <osterfeld@kde.org>

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

#include "notificationmanager.h"
#include "feed.h"

#include <KLocalizedString>
#include <knotification.h>
#include <kurl.h>
#include <kglobal.h>
#include <KComponentData>

#include <QTimer>

namespace Akregator
{

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

void NotificationManager::setWidget(QWidget *widget, const KComponentData &inst)
{
    m_widget = widget;
    m_instance = inst.isValid() ? inst : KComponentData::mainComponent();
}

void NotificationManager::slotNotifyArticle(const Article &article)
{
    m_articles.append(article);
    m_addedInLastInterval = true;
    if (m_articles.count() >= m_maxArticles) {
        doNotify();
    } else if (!m_running) {
        m_running = true;
        QTimer::singleShot(m_checkInterval, this, SLOT(slotIntervalCheck()));
    }
}

void NotificationManager::slotNotifyFeeds(const QStringList &feeds)
{
    if (feeds.count() == 1) {
        //KNotifyClient::Instance inst(m_instance);
        //QT5 KNotification::event(QLatin1String("FeedAdded"), i18n("Feed added:\n %1", feeds[0]), QPixmap() ,m_widget, KNotification::CloseOnTimeout, m_instance);
    } else if (feeds.count() > 1) {
        QString message;
        for (QStringList::ConstIterator it = feeds.constBegin(); it != feeds.constEnd(); ++it) {
            message += *it + QLatin1Char('\n');
        }
        //KNotifyClient::Instance inst(m_instance);
        //QT5 KNotification::event(QLatin1String("FeedAdded"), i18n("Feeds added:\n %1", message), QPixmap() ,m_widget, KNotification::CloseOnTimeout, m_instance);
    }
}

void NotificationManager::doNotify()
{
    QString message = QLatin1String("<html><body>");
    QString feedTitle;

    Q_FOREACH (const Article &i, m_articles) {
        if (feedTitle != i.feed()->title()) {
            feedTitle = i.feed()->title();
            message += QString::fromLatin1("<p><b>%1:</b></p>").arg(feedTitle);
        }
        message += i.title() + QLatin1String("<br>");
    }
    message += QLatin1String("</body></html>");
    //QT5 KNotification::event(QLatin1String("NewArticles"), message, QPixmap() ,m_widget, KNotification::CloseOnTimeout, m_instance);

    m_articles.clear();
    m_running = false;
    m_intervalsLapsed = 0;
    m_addedInLastInterval = false;
}

void NotificationManager::slotIntervalCheck()
{
    if (!m_running) {
        return;
    }
    m_intervalsLapsed++;
    if (!m_addedInLastInterval || m_articles.count() >= m_maxArticles || m_intervalsLapsed >= m_maxIntervals) {
        doNotify();
    } else {
        m_addedInLastInterval = false;
        QTimer::singleShot(m_checkInterval, this, SLOT(slotIntervalCheck()));
    }

}

NotificationManager *NotificationManager::m_self = 0;

NotificationManager *NotificationManager::self()
{
    static NotificationManager self;
    if (!m_self) {
        m_self = &self;
    }
    return m_self;
}

} // namespace Akregator

