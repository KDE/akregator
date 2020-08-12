/*
   SPDX-FileCopyrightText: 2019-2020 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "grantleeutil.h"
#include "feed.h"
QString Akregator::GrantleeUtil::imageFeed(const Feed *feed)
{
    if (feed->logoInfo().height != -1 && feed->logoInfo().width != -1) {
        return QStringLiteral("<a href=\"%1\"><img class=\"headimage\" src=\"%2\" height=\"%3\" width=\"%4\"></a>\n").arg(feed->htmlUrl(),
                                                                                                                          feed->logoInfo().imageUrl)
               .arg(feed->logoInfo().height)
               .arg(feed->logoInfo().width);
    }
    return QStringLiteral("<a href=\"%1\"><img class=\"headimage\" src=\"%2\"></a>\n").arg(feed->htmlUrl(), feed->logoInfo().imageUrl);
}
