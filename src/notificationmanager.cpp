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

#include <krss/feed.h>
#include <krss/feedlist.h>

#include <klocale.h>
#include <knotification.h>
#include <k3staticdeleter.h>
#include <kurl.h>
#include <kglobal.h>
#include <KDebug>

#include <QTimer>
#include <QList>

using namespace Akregator;
using namespace KRss;
using namespace boost;

NotificationManager::NotificationManager() : QObject()
{
    m_intervalsLapsed = 0;
    m_checkInterval = 2000;
    m_maxIntervals = 10;
    m_running = false;
    m_addedInLastInterval = false;
    m_maxArticles = 20;
    m_widget = 0;
}

NotificationManager::~NotificationManager()
{
    m_self = 0;
}

void NotificationManager::setWidget(QWidget* widget, const KComponentData &inst)
{
    m_widget = widget;
    m_instance = inst.isValid() ? inst : KGlobal::mainComponent();
}

void NotificationManager::slotNotifyArticle(const KRss::Item& item)
{
    m_items.append(item);
    m_addedInLastInterval = true;
    if (m_items.count() >= m_maxArticles)
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
        //KNotifyClient::Instance inst(m_instance);
        KNotification::event("FeedAdded", i18n("Feed added:\n %1", feeds[0]), QPixmap() ,m_widget, KNotification::CloseOnTimeout, m_instance);
    }
    else if (feeds.count() > 1)
    {
        QString message;
        for (QStringList::ConstIterator it = feeds.begin(); it != feeds.end(); ++it)
            message += *it + '\n';
        //KNotifyClient::Instance inst(m_instance);
        KNotification::event("FeedAdded", i18n("Feeds added:\n %1", message), QPixmap() ,m_widget, KNotification::CloseOnTimeout, m_instance);
    }
}

namespace {
    static bool lessThanByFeedId( const Item& lhs, const Item& rhs ) {
        return lhs.sourceFeedId() < rhs.sourceFeedId();
    }
}

void NotificationManager::doNotify()
{
    const shared_ptr<const FeedList> fl = m_feedList.lock();
    if ( fl && !m_items.isEmpty() ) {
        std::sort( m_items.begin(), m_items.end(), lessThanByFeedId );

        QString message = "<html><body>";
        QString currentFeedTitle;

        QHash<Feed::Id, QString> titles;
        Q_FOREACH( const Item& i, m_items )
        {
            const Feed::Id id = i.sourceFeedId();
            if ( !titles.contains( id ) ) {
                const shared_ptr<const Feed> feed = fl->constFeedById( id );
                if ( feed )
                    titles.insert( id, feed->title() );
            }
            const QString title = titles.value( id );
            if ( title != currentFeedTitle ) {
                currentFeedTitle = title;
                if ( title.isEmpty() )
                    message += QString("<p><b>%1:</b></p>").arg( !title.isEmpty() ? title : i18n("Unknown") );
            }
            message += i.title() + "<br>";
        }
        message += "</body></html>";
        KNotification::event("NewArticles", message, QPixmap() ,m_widget, KNotification::CloseOnTimeout, m_instance);
    }
    m_items.clear();
    m_running = false;
    m_intervalsLapsed = 0;
    m_addedInLastInterval = false;
}

void NotificationManager::setFeedList( const weak_ptr<const FeedList>& fl ) {
    m_feedList = fl;
}

void NotificationManager::slotIntervalCheck()
{
    if (!m_running)
        return;
    m_intervalsLapsed++;
    if (!m_addedInLastInterval || m_items.count() >= m_maxArticles || m_intervalsLapsed >= m_maxIntervals)
        doNotify();
    else
    {
        m_addedInLastInterval = false;
        QTimer::singleShot(m_checkInterval, this, SLOT(slotIntervalCheck()));
    }

}

NotificationManager* NotificationManager::m_self;
static K3StaticDeleter<NotificationManager> notificationmanagersd;

NotificationManager* NotificationManager::self()
{
    if (!m_self)
        m_self = notificationmanagersd.setObject(m_self, new NotificationManager);
    return m_self;
}

#include "notificationmanager.moc"
