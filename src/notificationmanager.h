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

#ifndef AKREGATOR_NOTIFICATIONMANAGER_H
#define AKREGATOR_NOTIFICATIONMANAGER_H

#include <qobject.h>
#include <QList>

#include "article.h"
#include <kcomponentdata.h>
#include "akregator_export.h"

namespace Akregator
{

/** this class collects notification requests (new articles etc.) and processes them using KNotify.  */
class AKREGATOR_EXPORT NotificationManager : public QObject
{
    Q_OBJECT
public:
    /** singleton instance of notification manager */
    static NotificationManager *self();

    ~NotificationManager();

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
    NotificationManager();
    NotificationManager(const NotificationManager &) : QObject() {}

    int m_checkInterval;
    int m_intervalsLapsed;
    int m_maxIntervals;
    int m_maxArticles;
    bool m_running;
    bool m_addedInLastInterval;
    QWidget *m_widget;
    QString m_componantName;

    QList<Article> m_articles;

    static NotificationManager *m_self;
};

} // namespace Akregator

#endif // AKREGATOR_NOTIFICATIONMANAGER_H
