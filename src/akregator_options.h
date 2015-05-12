/*
    This file is part of Akregator.

    Copyright (C) 2004 Frank Osterfeld <osterfeld@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef AKREGATOR_OPTIONS_H
#define AKREGATOR_OPTIONS_H

#include <QCommandLineParser>
#include <KLocalizedString>

namespace Akregator
{

static void akregator_options(QCommandLineParser *parser)
{
    QList<QCommandLineOption> options;
    options << QCommandLineOption(
          QStringList() << QLatin1String("a") << QLatin1String("addfeed"),
          i18n("Add a feed with the given URL"),
          QLatin1String("url"))
    << QCommandLineOption(
          QStringList() << QLatin1String("g") << QLatin1String("group"),
          i18n("When adding feeds, place them in this group"),
          QLatin1String("groupname")) //     "Imported"
    << QCommandLineOption(
          QLatin1String("hide-mainwindow"),
          i18n("Hide main window on startup"));

    parser->addOptions(options);
    parser->addPositionalArgument(
          QLatin1String("url"),
          i18n("Add a feed with the given URL"),
          QLatin1String("[url...]"));
}

}

#endif
