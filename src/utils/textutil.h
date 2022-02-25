/*
   SPDX-FileCopyrightText: 2022 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once
#include <QString>

namespace TextUtil
{
Q_REQUIRED_RESULT QString normalize(QStringView str);
Q_REQUIRED_RESULT QChar normalize(QChar c);
};
