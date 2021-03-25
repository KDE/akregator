/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include <KLocalizedString>
#include <QCommandLineParser>

namespace Akregator
{
static void akregator_options(QCommandLineParser *parser)
{
    QList<QCommandLineOption> options;
    options << QCommandLineOption(QStringList() << QStringLiteral("a") << QStringLiteral("addfeed"),
                                  i18n("Add a feed with the given URL"),
                                  QStringLiteral("url"))
            << QCommandLineOption(QStringList() << QStringLiteral("g") << QStringLiteral("group"),
                                  i18n("When adding feeds, place them in this group"),
                                  QStringLiteral("groupname")) //     "Imported"
            << QCommandLineOption(QStringLiteral("hide-mainwindow"), i18n("Hide main window on startup"));

    parser->addOptions(options);
    parser->addPositionalArgument(QStringLiteral("url"), i18n("Add a feed with the given URL"), QStringLiteral("[url...]"));
#ifdef WITH_KUSERFEEDBACK
    parser->addOption(QCommandLineOption(QStringLiteral("feedback"), i18n("Lists the available options for user feedback")));
#endif
}
}

