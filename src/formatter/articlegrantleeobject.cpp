/*
   SPDX-FileCopyrightText: 2016-2021 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "articlegrantleeobject.h"
#include "feed.h"
#include "grantleeutil.h"
#include "utils.h"
#include <QDateTime>
#include <QLocale>
#include <QUrl>
#include <QUrlQuery>

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
