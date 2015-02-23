/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <osterfeld@kde.org>

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

#include "utils.h"
#include <QRegExp>
#include <QString>
#include <kdelibs4configmigrator.h>

namespace Akregator
{

QString Utils::stripTags(const QString &str)
{
    return QString(str).remove(QRegExp(QStringLiteral("<[^>]*>")));
}

uint Utils::calcHash(const QString &str)
{
    const QByteArray array = str.toLatin1();
    return qChecksum(array.constData(), array.size());
}

QString Utils::fileNameForUrl(const QString &url_p)
{
    QString url2(url_p);

    url2 = url2.replace(QLatin1Char('/'), QLatin1Char('_')).replace(QLatin1Char(':'), QLatin1Char('_'));

    if (url2.length() > 255) {
        url2 = url2.left(200) + QString::number(Akregator::Utils::calcHash(url2), 16);
    }

    return url2;
}

// from kmail::headerstyle.cpp
QString Utils::directionOf(const QString &str)
{
    return str.isRightToLeft() ? QStringLiteral("rtl") : QStringLiteral("ltr");
}

void Utils::migrateConfig()
{
    Kdelibs4ConfigMigrator migrate(QStringLiteral("akregator"));
    migrate.setConfigFiles(QStringList() << QStringLiteral("akregatorrc") << QStringLiteral("akregator.notifyrc"));
    migrate.setUiFiles(QStringList() << QStringLiteral("akregator_shell.rc") << QStringLiteral("akregator_part.rc") << QStringLiteral("pageviewer.rc")
                       << QStringLiteral("articleviewer.rc") << QStringLiteral("akregator_sharemicroblog_plugin.rc") << QStringLiteral("akregator_onlinesync_plugin.rc"));
    migrate.migrate();
}

}
