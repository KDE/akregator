/*
   SPDX-FileCopyrightText: 2019-2021 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef GRANTLEEUTIL_H
#define GRANTLEEUTIL_H
#include <QString>
namespace Akregator
{
class Feed;
namespace GrantleeUtil
{
Q_REQUIRED_RESULT QString imageFeed(const Feed *feed);
}
}

#endif // GRANTLEEUTIL_H
