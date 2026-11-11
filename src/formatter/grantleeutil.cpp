/*
   SPDX-FileCopyrightText: 2019-2026 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "grantleeutil.h"
#include "feed.h"

using namespace Qt::Literals::StringLiterals;
QString Akregator::GrantleeUtil::imageFeed(const Feed *feed)
{
    const Feed::ImageInfo info = feed->logoInfo();
    if (info.height != -1 && info.width != -1) {
        return u"<a href=\"%1\"><img class=\"headimage\" src=\"%2\" height=\"%3\" width=\"%4\"></a>\n"_s.arg(feed->htmlUrl(),
                                                                                                             info.imageUrl,
                                                                                                             QString::number(info.height),
                                                                                                             QString::number(info.width));
    }
    return u"<a href=\"%1\"><img class=\"headimage\" src=\"%2\"></a>\n"_s.arg(feed->htmlUrl(), info.imageUrl);
}
