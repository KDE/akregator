/*
   Copyright (C) 2015-2016 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "akregratormigrateapplication.h"

#include <kdelibs4configmigrator.h>

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
    //TODO
}
