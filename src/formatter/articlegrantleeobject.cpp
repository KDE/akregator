/*
   SPDX-FileCopyrightText: 2016-2026 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "articlegrantleeobject.h"
#include "feed.h"
#include "grantleeutil.h"
#include "types.h"
#include "utils.h"
#include <QDateTime>
#include <QLocale>
#include <QUrl>
#include <QUrlQuery>

using namespace Qt::Literals::StringLiterals;

using namespace Akregator;

ArticleGrantleeObject::ArticleGrantleeObject(const Article &article, ArticleFormatter::IconOption iconOption, QObject *parent)
    : QObject(parent)
    , mArticle(article)
    , mArticleFormatOption(iconOption)
{
}

ArticleGrantleeObject::~ArticleGrantleeObject() = default;

ArticleGrantleeObject::ArticleStatus ArticleGrantleeObject::articleStatus() const
{
    const int status = mArticle.status();
    if (status == Akregator::Read) {
        return ArticleGrantleeObject::Read;
    } else if (status == Akregator::Unread) {
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
    if (const QDateTime pubDate = mArticle.pubDate(); pubDate.isValid()) {
        return QLocale().toString(pubDate, QLocale::ShortFormat);
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
    const QUrl link = mArticle.link();
    if (link.isValid()) {
        return link.url();
    }
    // in case link isn't valid, fall back to the guid permaLink.
    if (mArticle.guidIsPermaLink()) {
        if (const QString guid = mArticle.guid(); QUrl(guid).isValid()) {
            return guid;
        }
    }
    return {};
}

QString ArticleGrantleeObject::imageFeed() const
{
    QString text;
    if (mArticleFormatOption == ArticleFormatter::ShowIcon) {
        if (const Feed *feed = mArticle.feed(); feed && !feed->logoInfo().imageUrl.isEmpty()) {
            text += GrantleeUtil::imageFeed(feed);
        }
    }
    return text;
}

QString ArticleGrantleeObject::actionToken() const
{
    QUrlQuery query;
    query.addQueryItem(u"id"_s, mArticle.guid());
    if (mArticle.feed()) {
        query.addQueryItem(u"feed"_s, mArticle.feed()->xmlUrl());
    }
    return u'?' + query.toString(QUrl::FullyEncoded);
}

bool ArticleGrantleeObject::important() const
{
    return mArticle.keep();
}

#include "moc_articlegrantleeobject.cpp"
