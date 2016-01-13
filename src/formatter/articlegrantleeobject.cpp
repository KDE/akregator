/*
  Copyright (c) 2016 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "articlegrantleeobject.h"
#include "articleformatter.h"
#include "utils.h"
#include "feed.h"
#include <KLocale>
#include <KGlobal>
#include <QDateTime>
using namespace Akregator;

ArticleGrantleeObject::ArticleGrantleeObject(const QUrl &imageDir, const Article &article, ArticleFormatter::IconOption iconOption, QObject *parent)
    : QObject(parent),
      mArticle(article),
      mArticleFormatOption(iconOption),
      mImageDir(imageDir)
{

}

ArticleGrantleeObject::~ArticleGrantleeObject()
{

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
        return KLocale::global()->formatDateTime(mArticle.pubDate(), KLocale::FancyLongDate);
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
    if (mArticleFormatOption == ArticleFormatter::ShowIcon && mArticle.feed() && !mArticle.feed()->image().isNull()) {
        const Feed *feed = mArticle.feed();
        QString file = Utils::fileNameForUrl(feed->xmlUrl());
        QUrl u(mImageDir);
        u = u.adjusted(QUrl::RemoveFilename);
        u.setPath(u.path() + file);
        text += QStringLiteral("<a href=\"%1\"><img class=\"headimage\" src=\"%2.png\"></a>\n").arg(feed->htmlUrl(), u.url());
    }
    return text;
}

QString ArticleGrantleeObject::deleteAction() const
{
    //TODO fix image
    const QString text = QStringLiteral("<a href=\"akregatoraction:delete#%1\"><img class=\"headimage\" src=\"%2.png\"></a>\n").arg(mArticle.guid(), QStringLiteral("image"));
    return text;
}

QString ArticleGrantleeObject::markAsAction() const
{
    //TODO fix image
    const QString text = QStringLiteral("<a href=\"akregatoraction:markAsRead#%1\"><img class=\"headimage\" src=\"%2.png\"></a>\n").arg(mArticle.guid(), QStringLiteral("image"));
    return text;
}

QString ArticleGrantleeObject::markAsUnreadAction() const
{
    //TODO fix image
    const QString text = QStringLiteral("<a href=\"akregatoraction:markAsUnRead#%1\"><img class=\"headimage\" src=\"%2.png\"></a>\n").arg(mArticle.guid(), QStringLiteral("image"));
    return text;
}

QString ArticleGrantleeObject::markAsImportantAction() const
{
    //TODO fix image
    const QString text = QStringLiteral("<a href=\"akregatoraction:markAsImportant#%1\"><img class=\"headimage\" src=\"%2.png\"></a>\n").arg(mArticle.guid(), QStringLiteral("image"));
    return text;
}

QString ArticleGrantleeObject::sendUrlAction() const
{
    //TODO fix image
    const QString text = QStringLiteral("<a href=\"akregatoraction:sendUrlArticle#%1\"><img class=\"headimage\" src=\"%2.png\"></a>\n").arg(mArticle.guid(), QStringLiteral("image"));
    return text;
}

QString ArticleGrantleeObject::openInExternalBrowser() const
{
    //Fix image url;
    const QString text = QStringLiteral("<a href=\"akregatoraction:openInExternalBrowser\"><img class=\"headimage\" src=\"%2.png\"></a>\n").arg(QStringLiteral("image"));
    return text;
}

QString ArticleGrantleeObject::share() const
{
    //Fix image url;
    const QString text = QStringLiteral("<a href=\"akregatoraction:share\"><img class=\"headimage\" src=\"%2.png\"></a>\n").arg(QStringLiteral("image"));
    return text;
}
