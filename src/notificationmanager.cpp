/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "notificationmanager.h"
#include "feed.h"

#include <KAboutData>
#include <KLocalizedString>
#include <KNotification>
#include <QTimer>

using namespace Akregator;
NotificationManager::NotificationManager(QObject *parent)
    : QObject(parent)
{
    m_intervalsLapsed = 0;
    m_checkInterval = 2000;
    m_maxIntervals = 10;
    m_running = false;
    m_addedInLastInterval = false;
    m_maxArticles = 20;
    m_widget = nullptr;
}

NotificationManager::~NotificationManager()
{
    m_self = nullptr;
}

void NotificationManager::setWidget(QWidget *widget, const QString &componentName)
{
    m_widget = widget;
    m_componantName = componentName.isEmpty() ? KAboutData::applicationData().componentName() : componentName;
}

void NotificationManager::slotNotifyArticle(const Article &article)
{
    m_articles.append(article);
    m_addedInLastInterval = true;
    if (!m_running) {
        m_running = true;
        QTimer::singleShot(m_checkInterval, this, &NotificationManager::slotIntervalCheck);
    }
}

void NotificationManager::slotNotifyFeeds(const QStringList &feeds)
{
    const int feedsCount(feeds.count());
    if (feedsCount == 1) {
        KNotification::event(QStringLiteral("FeedAdded"),
                             i18n("Feed added:\n %1", feeds[0]),
                             QPixmap(),
                             m_widget,
                             KNotification::CloseOnTimeout,
                             m_componantName);
    } else if (feedsCount > 1) {
        QString message;
        QStringList::ConstIterator end = feeds.constEnd();
        for (QStringList::ConstIterator it = feeds.constBegin(); it != end; ++it) {
            message += *it + QLatin1Char('\n');
        }
        KNotification::event(QStringLiteral("FeedAdded"),
                             i18n("Feeds added:\n %1", message),
                             QPixmap(),
                             m_widget,
                             KNotification::CloseOnTimeout,
                             m_componantName);
    }
}

void NotificationManager::doNotify()
{
    QString message = QStringLiteral("<html><body>");
    QString feedTitle;
    int entriesCount = 1;
    const int maxNewArticlesShown = 2;

    // adding information about how many new articles
    auto feedClosure = [&entriesCount, &message, maxNewArticlesShown]() {
        if ((entriesCount - maxNewArticlesShown) > 1) {
            message += i18np("<i>and 1 other</i>", "<i>and %1 others</i>", entriesCount - maxNewArticlesShown - 1) + QLatin1String("<br>");
        }
    };

    for (const Article &i : qAsConst(m_articles)) {
        const QString currentFeedTitle(i.feed()->title());
        if (feedTitle != currentFeedTitle) {
            // closing previous feed, if any, and resetting the counter
            feedClosure();
            entriesCount = 1;

            // starting a new feed
            feedTitle = currentFeedTitle;
            message += QStringLiteral("<p><b>%1:</b></p>").arg(feedTitle);
        }
        // check not exceeding maxNewArticlesShown per feed
        if (entriesCount <= maxNewArticlesShown) {
            message += i.title() + QLatin1String("<br>");
        }
        entriesCount++;
    }
    feedClosure();
    message += QLatin1String("</body></html>");
    KNotification::event(QStringLiteral("NewArticles"), message, QPixmap(), m_widget, KNotification::CloseOnTimeout, m_componantName);

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
        QTimer::singleShot(m_checkInterval, this, &NotificationManager::slotIntervalCheck);
    }
}

NotificationManager *NotificationManager::m_self = nullptr;

NotificationManager *NotificationManager::self()
{
    static NotificationManager self;
    if (!m_self) {
        m_self = &self;
    }
    return m_self;
}
