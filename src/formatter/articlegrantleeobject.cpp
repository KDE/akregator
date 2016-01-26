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
#include <MessageViewer/IconNameCache>
#include "utils.h"
#include "feed.h"
#include <KLocale>
#include <KGlobal>
#include <KIconLoader>
#include <QDateTime>
#include <QUrl>
#include <QUrlQuery>

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

QString ArticleGrantleeObject::commentLink() const
{
    return mArticle.commentsLink().url();
}

QString ArticleGrantleeObject::commentNumber() const
{
    if (mArticle.comments()) {
        return QString::number(mArticle.comments());
    } else {
        return {};
    }
}
QString ArticleGrantleeObject::createActionUrl(const QString &actionName) const
{
    QUrl url;
    url.setScheme(QStringLiteral("akregatoraction"));
    url.setPath(actionName);
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("id"), mArticle.guid());
    query.addQueryItem(QStringLiteral("feed"), mArticle.feed()->xmlUrl());
    url.setQuery(query);
    return url.toDisplayString();
}

QString ArticleGrantleeObject::deleteAction() const
{
    const QString iconPath = MessageViewer::IconNameCache::instance()->iconPath(QStringLiteral("edit-delete"), KIconLoader::Small);
    const QString text = QStringLiteral("<a href=\"%1\"><img class=\"headimage\" title=\"%3\" src=\"file:///%2\"></a>\n")
            .arg(createActionUrl(QStringLiteral("delete")), iconPath, i18n("Delete Article"));
    return text;
}

QString ArticleGrantleeObject::markAsAction() const
{
    const QString iconPath = MessageViewer::IconNameCache::instance()->iconPath(QStringLiteral("mail-mark-read"), KIconLoader::Small);
    const QString text = QStringLiteral("<a href=\"%1\"><img class=\"headimage\" title=\"%3\" src=\"file:///%2\"></a>\n")
            .arg(createActionUrl(QStringLiteral("markAsRead")), iconPath, i18n("Mark as Read"));
    return text;
}

QString ArticleGrantleeObject::markAsUnreadAction() const
{
    const QString iconPath = MessageViewer::IconNameCache::instance()->iconPath(QStringLiteral("mail-mark-unread"), KIconLoader::Small);
    const QString text = QStringLiteral("<a href=\"%1\"><img class=\"headimage\" title=\"%3\" src=\"file:///%2\"></a>\n")
            .arg(createActionUrl(QStringLiteral("markAsUnRead")), iconPath, i18n("Mark as Unread"));
    return text;
}

QString ArticleGrantleeObject::markAsImportantAction() const
{
    const QString iconPath = MessageViewer::IconNameCache::instance()->iconPath(QStringLiteral("mail-mark-important"), KIconLoader::Small);
    const QString text = QStringLiteral("<a href=\"%1\"><img class=\"headimage\" title=\"%3\" src=\"file:///%2\"></a>\n")
            .arg(createActionUrl(QStringLiteral("markAsImportant")), iconPath, i18n("Mark as Important"));
    return text;
}

QString ArticleGrantleeObject::sendUrlAction() const
{
    const QString iconPath = MessageViewer::IconNameCache::instance()->iconPath(QStringLiteral("mail-message-new"), KIconLoader::Small);
    const QString text = QStringLiteral("<a href=\"%1\"><img class=\"headimage\" title=\"%3\" src=\"file:///%2\"></a>\n")
            .arg(createActionUrl(QStringLiteral("sendUrlArticle")), iconPath, i18n("Mark as New"));
    return text;
}

QString ArticleGrantleeObject::openInExternalBrowser() const
{
    const QString iconPath = MessageViewer::IconNameCache::instance()->iconPath(QStringLiteral("window-new"), KIconLoader::Small);
    const QString text = QStringLiteral("<a href=\"%1\"><img class=\"headimage\" title=\"%3\" src=\"file:///%2\"></a>\n")
            .arg(createActionUrl(QStringLiteral("openInExternalBrowser")), iconPath, i18n("Open Link in External Browser"));
    return text;
}

QString ArticleGrantleeObject::share() const
{
    const QString iconPath = MessageViewer::IconNameCache::instance()->iconPath(QStringLiteral("document-share"), KIconLoader::Small);
    const QString text = QStringLiteral("<a href=\"%1\"><img class=\"headimage\" title=\"%3\" src=\"file:///%2\"></a>\n")
            .arg(createActionUrl(QStringLiteral("share")), iconPath, i18n("Share"));
    return text;
}

QString ArticleGrantleeObject::openInBackgroundTab() const
{
    const QString iconPath = MessageViewer::IconNameCache::instance()->iconPath(QStringLiteral("tab-new"), KIconLoader::Small);
    const QString text = QStringLiteral("<a href=\"%1\"><img class=\"headimage\" title=\"%3\" src=\"file:///%2\"></a>\n")
            .arg(createActionUrl(QStringLiteral("openInBackgroundTab")), iconPath, i18n("Open In Background Tab"));
    return text;
}
