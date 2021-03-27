/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include <QObject>
#include <QVector>

#include "akregator_export.h"
#include "article.h"

namespace Akregator
{
/** this class collects notification requests (new articles etc.) and processes them using KNotify.  */
class AKREGATOR_EXPORT NotificationManager : public QObject
{
    Q_OBJECT
public:
    /** singleton instance of notification manager */
    static NotificationManager *self();

    ~NotificationManager() override;

    /** the widget used for notification, normally either the mainwindow or the tray icon */
    void setWidget(QWidget *widget, const QString &componentName = QString());

public Q_SLOTS:

    /** notifies an article. Note that articles are not notified separately, but
    "collected" and notified all together */
    void slotNotifyArticle(const Akregator::Article &article);

    /** notifies the addition of feeds (used when added via DCOP or command line) */
    void slotNotifyFeeds(const QStringList &feeds);

protected:
    void doNotify();

protected Q_SLOTS:

    void slotIntervalCheck();

private:
    explicit NotificationManager(QObject *parent = nullptr);
    NotificationManager(const NotificationManager &)
        : QObject()
    {
    }

    int m_checkInterval;
    int m_intervalsLapsed;
    int m_maxIntervals;
    int m_maxArticles;
    bool m_running = false;
    bool m_addedInLastInterval = false;
    QWidget *m_widget = nullptr;
    QString m_componantName;

    QVector<Article> m_articles;

    static NotificationManager *m_self;
};
} // namespace Akregator

