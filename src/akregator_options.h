/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#pragma once

#include "config-akregator.h"
#include <KLocalizedString>
#include <QCommandLineParser>

using namespace Qt::Literals::StringLiterals;

namespace Akregator
{
static void akregator_options(QCommandLineParser *parser)
{
    QList<QCommandLineOption> options;
    options << QCommandLineOption(QStringList() << u"a"_s << u"addfeed"_s, i18nc("@info:shell", "Add a feed with the given URL"), u"url"_s)
            << QCommandLineOption(QStringList() << u"g"_s << u"group"_s,
                                  i18nc("@info:shell", "When adding feeds, place them in this group"),
                                  i18n("Group Name")) //     "Imported"
            << QCommandLineOption(u"hide-mainwindow"_s, i18nc("@info:shell", "Hide main window on startup"));

    parser->addOptions(options);
    parser->addPositionalArgument(u"url"_s, i18nc("@info:shell", "Add a feed with the given URL"), u"[url…]"_s);
#if AKREGATOR_WITH_KUSERFEEDBACK
    parser->addOption(QCommandLineOption(u"feedback"_s, i18nc("@info:shell", "Lists the available options for user feedback")));
#endif
}
}
