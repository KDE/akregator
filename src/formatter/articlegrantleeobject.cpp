/*
   Copyright (C) 2016-2020 Laurent Montel <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "articlegrantleeobject.h"
#include "articleformatter.h"
#include "grantleeutil.h"
#include "utils.h"
#include "feed.h"
#include <QLocale>
#include <QUrl>
#include <QUrlQuery>
#include <QDateTime>

using namespace Akregator;

ArticleGrantleeObject::ArticleGrantleeObject(const Article &article, ArticleFormatter::IconOption iconOption, QObject *parent)
    : QObject(parent)
    , mArticle(article)
    , mArticleFormatOption(iconOption)
{
}

ArticleGrantleeObject::~ArticleGrantleeObject()
{
}

ArticleGrantleeObject::ArticleStatus ArticleGrantleeObject::articleStatus() const
{
    if (mArticle.status() == ArticleStatus::Read) {
        return ArticleGrantleeObject::Read;
    } else if (mArticle.status() == ArticleStatus::Unread) {
        return ArticleGrantleeObject::Unread;
    } else {
        return ArticleGrantleeObject::New;
    }
}

QString ArticleGrantleeObject::strippedTitle() const
{
    return Akregator::Utils::stripTags(mArticle.title());
}

QString ArticleGrantleeObject::author() const
{
    return mArticle.authorAsHtml();
}

QString ArticleGrantleeObject::content() const
{
    return mArticle.content(Article::DescriptionAsFallback);
}

QString ArticleGrantleeObject::articleLinkUrl() const
{
    return mArticle.link().url();
}

QString ArticleGrantleeObject::articlePubDate() const
{
    if (mArticle.pubDate().isValid()) {
        return QLocale().toString(mArticle.pubDate(), QLocale::ShortFormat);
    }
    return {};
}

QString ArticleGrantleeObject::enclosure() const
{
    const QString enc = ArticleFormatter::formatEnclosure(*mArticle.enclosure());
    return enc;
}

QString ArticleGrantleeObject::articleCompleteStoryLink() const
{
    QString link;
    if (mArticle.link().isValid() || (mArticle.guidIsPermaLink() && QUrl(mArticle.guid()).isValid())) {
        // in case link isn't valid, fall back to the guid permaLink.
        if (mArticle.link().isValid()) {
            link = mArticle.link().url();
        } else {
            link = mArticle.guid();
        }
    }
    return link;
}

QString ArticleGrantleeObject::imageFeed() const
{
    QString text;
    if (mArticleFormatOption == ArticleFormatter::ShowIcon && mArticle.feed() && !mArticle.feed()->logoInfo().imageUrl.isEmpty()) {
        const Feed *feed = mArticle.feed();
        text += GrantleeUtil::imageFeed(feed);
    }
    return text;
}

QString ArticleGrantleeObject::actionToken() const
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("id"), mArticle.guid());
    query.addQueryItem(QStringLiteral("feed"), mArticle.feed()->xmlUrl());
    return QLatin1Char('?') + query.toString(QUrl::FullyEncoded);
}

bool ArticleGrantleeObject::important() const
{
    return mArticle.keep();
}
