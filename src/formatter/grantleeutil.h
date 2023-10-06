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
[[nodiscard]] QString imageFeed(const Feed *feed);
}
}
