/*
   SPDX-FileCopyrightText: 2016-2020 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "grantleeviewformatter.h"
#include "articlegrantleeobject.h"
#include "utils.h"
#include "akregatorconfig.h"
#include "grantleeutil.h"
#include <KLocalizedString>

#include <grantlee/engine.h>

#include <QVariantHash>
#include <QApplication>
#include <QVariantList>
#include <folder.h>
#include <feed.h>

using namespace Akregator;

GrantleeViewFormatter::GrantleeViewFormatter(const QString &htmlFileName, const QString &themePath, int deviceDpiY)
    : GrantleeTheme::GenericFormatter(htmlFileName, themePath)
    , mHtmlArticleFileName(htmlFileName)
    , mDirectionString(QApplication::isRightToLeft() ? QStringLiteral("rtl") : QStringLiteral("ltr"))
    , mGrantleeThemePath(QStringLiteral("file://") + themePath + QLatin1Char('/'))
    , mDeviceDpiY(deviceDpiY)
{
    setApplicationDomain("akregator");
}

GrantleeViewFormatter::~GrantleeViewFormatter()
{
}

int GrantleeViewFormatter::pointsToPixel(int pointSize) const
{
    return (pointSize * mDeviceDpiY + 36) / 72;
}

void GrantleeViewFormatter::addStandardObject(QVariantHash &grantleeObject)
{
    grantleeObject.insert(QStringLiteral("absoluteThemePath"), mGrantleeThemePath);
    grantleeObject.insert(QStringLiteral("applicationDir"), mDirectionString);
    grantleeObject.insert(QStringLiteral("standardFamilyFont"), Settings::standardFont());
    grantleeObject.insert(QStringLiteral("mediumFontSize"), pointsToPixel(Settings::mediumFontSize()));
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
    if (!feed->logoInfo().imageUrl.isEmpty()) { // image
        feedImage = GrantleeUtil::imageFeed(feed);
    } else {
        feedImage = QStringLiteral("<div class=\"body\">");
    }
    feedObject.insert(QStringLiteral("feedImage"), feedImage);

    if (!feed->description().isEmpty()) {
        QString feedDescription = QStringLiteral("<div dir=\"%1\">").arg(mDirectionString);
        feedDescription += i18n("<b>Description:</b> %1<br />", feed->description());
        feedDescription += QStringLiteral("</div>"); // /description
        feedObject.insert(QStringLiteral("feedDescription"), feedDescription);
    }

    if (!feed->htmlUrl().isEmpty()) {
        QString feedHomePage = QStringLiteral("<div dir=\"%1\">").arg(mDirectionString);
        feedHomePage += i18n("<b>Homepage:</b> <a href=\"%1\">%1</a>", feed->htmlUrl());
        feedHomePage += QStringLiteral("</div>"); // / link
        feedObject.insert(QStringLiteral("feedHomePage"), feedHomePage);
    }

    if (!feed->copyright().isEmpty()) {
        QString feedCopyright = QStringLiteral("<div dir=\"%1\">").arg(mDirectionString);
        feedCopyright += i18n("<b>Copyright:</b> %1</a>", feed->copyright());
        feedCopyright += QStringLiteral("</div>");
        feedObject.insert(QStringLiteral("feedCopyright"), feedCopyright);
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
    QList<ArticleGrantleeObject *> lstObj;
    lstObj.reserve(nbArticles);
    for (int i = 0; i < nbArticles; ++i) {
        ArticleGrantleeObject *articleObj = new ArticleGrantleeObject(article.at(i), icon);
        articlesList << QVariant::fromValue(static_cast<QObject *>(articleObj));
        lstObj.append(articleObj);
    }
    articleObject.insert(QStringLiteral("articles"), articlesList);

    addStandardObject(articleObject);

    articleObject.insert(QStringLiteral("dateI18n"), i18n("Date"));
    articleObject.insert(QStringLiteral("commentI18n"), i18n("Comment"));
    articleObject.insert(QStringLiteral("completeStoryI18n"), i18n("Complete Story"));
    articleObject.insert(QStringLiteral("authorI18n"), i18n("Author"));
    articleObject.insert(QStringLiteral("enclosureI18n"), i18n("Enclosure"));

    const QString str = render(articleObject);
    qDeleteAll(lstObj);
    return str;
}
