/*
   SPDX-FileCopyrightText: 2024-2025 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "activitiesmanager.h"
#include "akregatorconfig.h"
using namespace Akregator;
ActivitiesManager::ActivitiesManager(QObject *parent)
    : PimCommonActivities::ActivitiesBaseManager{parent}
{
}

ActivitiesManager::~ActivitiesManager() = default;

#include "moc_activitiesmanager.cpp"
