/*
   SPDX-FileCopyrightText: 2018-2021 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "akregator_export.h"
#include <QObject>
class QDBusServiceWatcher;

class AKREGATOR_EXPORT UnityServiceManager : public QObject
{
    Q_OBJECT
public:
    explicit UnityServiceManager(QObject *parent = nullptr);
    ~UnityServiceManager() override;

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

