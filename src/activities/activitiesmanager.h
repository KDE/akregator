/*
   SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QObject>
namespace KActivities
{
class Consumer;
}
namespace Akregator
{
class ActivitiesManager : public QObject
{
    Q_OBJECT
public:
    explicit ActivitiesManager(QObject *parent = nullptr);
    ~ActivitiesManager() override;
    [[nodiscard]] bool enabled() const;
    void setEnabled(bool newEnabled);

    [[nodiscard]] bool isInCurrentActivity(const QStringList &lst) const;

    [[nodiscard]] QString currentActivity() const;
Q_SIGNALS:
    void activitiesChanged();

private:
    KActivities::Consumer *const mActivitiesConsumer;
    bool mEnabled = false;
};
}
