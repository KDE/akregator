/*
   SPDX-FileCopyrightText: 2018-2020 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef UNITYSERVICEMANAGER_H
#define UNITYSERVICEMANAGER_H

#include <QObject>
#include "akregator_export.h"
class QDBusServiceWatcher;

class AKREGATOR_EXPORT UnityServiceManager : public QObject
{
    Q_OBJECT
public:
    explicit UnityServiceManager(QObject *parent = nullptr);
    ~UnityServiceManager();

    static UnityServiceManager *instance();

public Q_SLOTS:
    void slotSetUnread(int unread);

private:
    void updateCount();
    void initUnity();
    QDBusServiceWatcher *mUnityServiceWatcher = nullptr;
    static UnityServiceManager *mInstance;
    int m_unread = 0;
    bool mUnityServiceAvailable = false;
};

#endif // UNITYSERVICEMANAGER_H
