/*
   SPDX-FileCopyrightText: 2019-2023 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once
#include <QString>
namespace Akregator
{
class Feed;
namespace GrantleeUtil
{
Q_REQUIRED_RESULT QString imageFeed(const Feed *feed);
}
}
