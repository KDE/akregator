/*
   SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "akregator_export.h"
#include <QObject>
namespace KActivities
{
class Consumer;
}
namespace Akregator
{
class AKREGATOR_EXPORT ActivitiesManager : public QObject
{
    Q_OBJECT
public:
    struct ActivitySettings {
        QStringList activities;
        bool enabled = false;
        [[nodiscard]] bool contains(const QString &str) const
        {
            return activities.contains(str);
        }

        void changeActivities(bool added, const QString &currentActivity)
        {
            if (added) {
                if (!activities.contains(currentActivity)) {
                    activities.append(currentActivity);
                }
            } else {
                if (activities.contains(currentActivity)) {
                    activities.removeAll(currentActivity);
                }
            }
            enabled = true;
        }
    };

    explicit ActivitiesManager(QObject *parent = nullptr);
    ~ActivitiesManager() override;
    [[nodiscard]] bool enabled() const;

    [[nodiscard]] bool isInCurrentActivity(const QStringList &lst) const;

    [[nodiscard]] QString currentActivity() const;
Q_SIGNALS:
    void activitiesChanged();

private:
    KActivities::Consumer *const mActivitiesConsumer;
};
}
