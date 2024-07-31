/*
   SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "activitiesmanager.h"
#include "akregator_plasma_activities_debug.h"
// FIXME #include "akregatorconfig.h"
#include <PlasmaActivities/Consumer>
ActivitiesManager::ActivitiesManager(QObject *parent)
    : QObject{parent}
    , mActivitiesConsumer(new KActivities::Consumer(this))
{
    // FIXME mEnabled = Settings::self()->plasmaActivities();
    connect(mActivitiesConsumer, &KActivities::Consumer::currentActivityChanged, this, [this](const QString &activityId) {
        qCDebug(AKREGATOR_PLASMA_ACTIVITIES_LOG) << " switch to activity " << activityId;
        Q_EMIT activitiesChanged();
    });
    connect(mActivitiesConsumer, &KActivities::Consumer::activityRemoved, this, [this](const QString &activityId) {
        qCDebug(AKREGATOR_PLASMA_ACTIVITIES_LOG) << " Activity removed " << activityId;
        Q_EMIT activitiesChanged();
    });
    connect(mActivitiesConsumer, &KActivities::Consumer::serviceStatusChanged, this, &ActivitiesManager::activitiesChanged);
    if (mActivitiesConsumer->serviceStatus() != KActivities::Consumer::ServiceStatus::Running) {
        qCWarning(AKREGATOR_PLASMA_ACTIVITIES_LOG) << "Plasma activities is not running: " << mActivitiesConsumer->serviceStatus();
    }
}

ActivitiesManager::~ActivitiesManager() = default;

bool ActivitiesManager::enabled() const
{
    return mEnabled;
}

void ActivitiesManager::setEnabled(bool newEnabled)
{
    mEnabled = newEnabled;
}

bool ActivitiesManager::isInCurrentActivity(const QStringList &lst) const
{
    if (mActivitiesConsumer->serviceStatus() == KActivities::Consumer::ServiceStatus::Running) {
        if (lst.contains(mActivitiesConsumer->currentActivity())) {
            return true;
        } else {
            const QStringList activities = mActivitiesConsumer->activities();
            auto index = std::find_if(activities.constBegin(), activities.constEnd(), [lst](const QString &str) {
                return lst.contains(str);
            });
            // Account doesn't contains valid activities => show it.
            if (index == activities.constEnd()) {
                return true;
            }
            return false;
        }
    } else {
        return true;
    }
}

QString ActivitiesManager::currentActivity() const
{
    return mActivitiesConsumer->currentActivity();
}

#include "moc_activitiesmanager.cpp"
