/*
   SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "akregator_export.h"
#include <PimCommonActivities/ActivitiesBaseManager>
#include <QObject>
namespace Akregator
{
class AKREGATOR_EXPORT ActivitiesManager : public PimCommonActivities::ActivitiesBaseManager
{
    Q_OBJECT
public:
    explicit ActivitiesManager(QObject *parent = nullptr);
    ~ActivitiesManager() override;
    [[nodiscard]] bool enabled() const override;
};
}
