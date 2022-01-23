/*
   SPDX-FileCopyrightText: 2015-2022 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "akregratormigrateapplication.h"
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Kdelibs4ConfigMigrator>

AkregratorMigrateApplication::AkregratorMigrateApplication()
{
    initializeMigrator();
}

void AkregratorMigrateApplication::migrate()
{
    // Migrate to xdg.
    Kdelibs4ConfigMigrator migrate(QStringLiteral("akregator"));
    migrate.setConfigFiles(QStringList() << QStringLiteral("akregatorrc") << QStringLiteral("akregator.notifyrc"));
    migrate.setUiFiles(QStringList() << QStringLiteral("akregator_shell.rc") << QStringLiteral("akregator_part.rc")
                                     << QStringLiteral("akregator_sharemicroblog_plugin.rc") << QStringLiteral("akregator_onlinesync_plugin.rc"));
    migrate.migrate();

    // Migrate folders and files.
    if (mMigrator.checkIfNecessary()) {
        mMigrator.start();
    }
}

void AkregratorMigrateApplication::initializeMigrator()
{
    const int currentVersion = 2;

    mMigrator.setApplicationName(QStringLiteral("akregator"));
    mMigrator.setConfigFileName(QStringLiteral("akregatorrc"));
    mMigrator.setCurrentConfigVersion(currentVersion);

    // To migrate we need a version > currentVersion
    const int initialVersion = currentVersion + 1;

    // Database
    PimCommon::MigrateFileInfo migrateInfoArchive;
    migrateInfoArchive.setFolder(true);
    migrateInfoArchive.setType(QStringLiteral("data"));
    migrateInfoArchive.setPath(QStringLiteral("akregator/Archive/"));
    migrateInfoArchive.setVersion(initialVersion);
    mMigrator.insertMigrateInfo(migrateInfoArchive);

    PimCommon::MigrateFileInfo migrateInfoData;
    migrateInfoData.setFolder(true);
    migrateInfoData.setType(QStringLiteral("data"));
    migrateInfoData.setPath(QStringLiteral("akregator/data/"));
    migrateInfoData.setVersion(initialVersion);
    mMigrator.insertMigrateInfo(migrateInfoData);
    // TODO
}
#endif
