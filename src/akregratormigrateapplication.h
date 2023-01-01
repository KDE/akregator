/*
   SPDX-FileCopyrightText: 2015-2023 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include "akregator_export.h"
#include <PimCommon/MigrateApplicationFiles>

class AKREGATOR_EXPORT AkregratorMigrateApplication
{
public:
    AkregratorMigrateApplication();
    void migrate();

private:
    void initializeMigrator();
    PimCommon::MigrateApplicationFiles mMigrator;
};
#endif
