/*
   SPDX-FileCopyrightText: 2015-2021 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

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

