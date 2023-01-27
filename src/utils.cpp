/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "utils.h"
#include <QRegularExpression>
#include <QTextDocument>

using namespace Akregator;
QString Utils::convertHtmlTags(const QString &title)
{
    QTextDocument newText;
    newText.setHtml(title);
    return newText.toPlainText();
}

QString Utils::stripTags(QString str)
{
    return str.remove(QRegularExpression(QStringLiteral("<[^>]*>")));
}

uint Utils::calcHash(const QString &str)
{
    const QByteArray array = str.toLatin1();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return qChecksum(array.constData(), array.size());
#else
    return qChecksum(QByteArray(array.constData(), array.size()));
#endif
}
