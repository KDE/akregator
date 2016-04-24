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

#include "grantleeviewformatter.h"
#include "articlegrantleeobject.h"
#include "utils.h"
#include "akregatorconfig.h"
#include <KLocalizedString>

#include <grantlee/engine.h>

#include <QVariantHash>
#include <QApplication>
#include <QDateTime>
#include <QVariantList>
#include <QDebug>
#include <folder.h>
#include <feed.h>

using namespace Akregator;

GrantleeViewFormatter::GrantleeViewFormatter(const QString &htmlFileName, const QString &themePath, const QUrl &imageDir, QObject *parent)
    : PimCommon::GenericGrantleeFormatter(htmlFileName, themePath, parent),
      mImageDir(imageDir),
      mHtmlArticleFileName(htmlFileName),
      mGrantleeThemePath(QStringLiteral("file://") + themePath + QLatin1Char('/'))
{
    mDirectionString = QApplication::isRightToLeft() ? QStringLiteral("rtl") : QStringLiteral("ltr");
}

GrantleeViewFormatter::~GrantleeViewFormatter()
{

}

void GrantleeViewFormatter::addStandardObject(QVariantHash &grantleeObject)
{
    grantleeObject.insert(QStringLiteral("absoluteThemePath"), mGrantleeThemePath);
    grantleeObject.insert(QStringLiteral("applicationDir"), mDirectionString);
    grantleeObject.insert(QStringLiteral("standardFamilyFont"), Settings::standardFont());
    //TODO grantleeObject.insert(QStringLiteral("mediumFontSize"), Settings::standardFont());
}

QString GrantleeViewFormatter::formatFeed(Akregator::Feed *feed)
{
    setDefaultHtmlMainFile(QStringLiteral("defaultnormalvisitfeed.html"));
    if (!errorMessage().isEmpty()) {
        return errorMessage();
    }
    QVariantHash feedObject;
    addStandardObject(feedObject);
    feedObject.insert(QStringLiteral("strippedTitle"), Utils::stripTags(feed->title()));
    QString numberOfArticle;
    if (feed->unread() == 0) {
        numberOfArticle = i18n(" (no unread articles)");
    } else {
        numberOfArticle = i18np(" (1 unread article)", " (%1 unread articles)", feed->unread());
    }

    feedObject.insert(QStringLiteral("feedCount"), numberOfArticle);

    QString feedImage;
    if (!feed->image().isNull()) { // image
        feedImage = QLatin1String("<div class=\"body\">");
        QString file = Utils::fileNameForUrl(feed->xmlUrl());
        QUrl u(mImageDir);
        u = u.adjusted(QUrl::RemoveFilename);
        u.setPath(u.path() + file);
        feedImage = QStringLiteral("<a href=\"%1\"><img class=\"headimage\" src=\"%2.png\"></a>\n").arg(feed->htmlUrl(), u.url());
    } else {
        feedImage = QStringLiteral("<div class=\"body\">");
    }
    feedObject.insert(QStringLiteral("feedImage"), feedImage);

    if (!feed->description().isEmpty()) {
        QString feedDescription;
        feedDescription = QStringLiteral("<div dir=\"%1\">").arg(mDirectionString);
        feedDescription += i18n("<b>Description:</b> %1<br />", feed->description());
        feedDescription += QStringLiteral("</div>"); // /description
        feedObject.insert(QStringLiteral("feedDescription"), feedDescription);
    }

    if (!feed->htmlUrl().isEmpty()) {
        QString feedHomePage;
        feedHomePage = QStringLiteral("<div dir=\"%1\">").arg(mDirectionString);
        feedHomePage += i18n("<b>Homepage:</b> <a href=\"%1\">%2</a>", feed->htmlUrl(), feed->htmlUrl());
        feedHomePage += QStringLiteral("</div>"); // / link
        feedObject.insert(QStringLiteral("feedHomePage"), feedHomePage);
    }

    return render(feedObject);
}

QString GrantleeViewFormatter::formatFolder(Akregator::Folder *node)
{
    setDefaultHtmlMainFile(QStringLiteral("defaultnormalvisitfolder.html"));
    if (!errorMessage().isEmpty()) {
        return errorMessage();
    }
    QVariantHash folderObject;
    addStandardObject(folderObject);

    folderObject.insert(QStringLiteral("nodeTitle"), node->title());
    QString numberOfArticle;
    if (node->unread() == 0) {
        numberOfArticle = i18n(" (no unread articles)");
    } else {
        numberOfArticle = i18np(" (1 unread article)", " (%1 unread articles)", node->unread());
    }

    folderObject.insert(QStringLiteral("nodeCount"), numberOfArticle);
    return render(folderObject);
}

QString GrantleeViewFormatter::formatArticles(const QVector<Article> &article, ArticleFormatter::IconOption icon)
{
    setDefaultHtmlMainFile(mHtmlArticleFileName);
    if (!errorMessage().isEmpty()) {
        return errorMessage();
    }

    QVariantHash articleObject;

    QVariantList articlesList;
    const int nbArticles(article.count());
    articlesList.reserve(nbArticles);
    for (int i = 0; i < nbArticles; ++i) {
        ArticleGrantleeObject *articleObj = new ArticleGrantleeObject(mImageDir, article.at(i), icon);
        articlesList << QVariant::fromValue(static_cast<QObject *>(articleObj));
    }
    articleObject.insert(QStringLiteral("articles"), articlesList);

    addStandardObject(articleObject);

    articleObject.insert(QStringLiteral("dateI18n"), i18n("Date"));
    articleObject.insert(QStringLiteral("commentI18n"), i18n("Comment"));
    articleObject.insert(QStringLiteral("completeStoryI18n"), i18n("Complete Story"));
    articleObject.insert(QStringLiteral("authorI18n"), i18n("Author"));
    articleObject.insert(QStringLiteral("enclosureI18n"), i18n("Enclosure"));

    return render(articleObject);
}

